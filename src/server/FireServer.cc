#include "FireServer.h"

using namespace omnetpp;
using namespace forestfiresim;

Define_Module(FireServer);

//
// FireServer.cc
// The sink of the simulation. All packets end up here.
// Records end-to-end delay and packet counts for PDR calculation.
//
// End-to-end delay = simTime() - packet.timestamp
// This captures: sensor processing + LoRa transmission + gateway queue + INET
//

void FireServer::initialize()
{
    numNodes           = par("numNodes");
    packetsReceived    = 0;
    alarmsReceived     = 0;
    falseAlarmsReceived = 0;
    telemetryReceived  = 0;

    // Register output signals
    e2eDelaySignal            = registerSignal("e2eDelay");
    telemetryDelaySignal      = registerSignal("telemetryDelay");
    packetsReceivedSignal     = registerSignal("packetsReceived");
    alarmsReceivedSignal      = registerSignal("alarmsReceived");
    falseAlarmsReceivedSignal = registerSignal("falseAlarmsReceived");

    EV << "[FireServer] initialized. Waiting for packets..." << endl;
}

void FireServer::handleMessage(cMessage *msg)
{
    double now = simTime().dbl();

    AlarmMsg *alarmMsg = dynamic_cast<AlarmMsg*>(msg);
    if (alarmMsg != nullptr) {
        double delay = now - alarmMsg->getTimestamp();
        emit(e2eDelaySignal, delay);
        emit(alarmsReceivedSignal, 1L);
        alarmsReceived++;
        packetsReceived++;
        if (alarmMsg->isFalseAlarm())  {
            falseAlarmsReceived++;
            emit(falseAlarmsReceivedSignal, 1L);
            EV << "[FireServer] FALSE ALARM from sensor " << alarmMsg->getSensorId()
               << " delay=" << delay << "s" << endl;
        } else {
            EV << "[FireServer] ALARM from sensor " << alarmMsg->getSensorId()
               << " zone=" << alarmMsg->getZoneId()
               << " delay=" << delay << "s" << endl;
        }
        delete alarmMsg;
        return;
    }

    // Check if it is a TelemetryMsg
    TelemetryMsg *telem = dynamic_cast<TelemetryMsg*>(msg);
    if (telem != nullptr) {
        double delay = now - telem->getTimestamp();
        emit(telemetryDelaySignal, delay);
        emit(packetsReceivedSignal, 1L);

        telemetryReceived++;
        packetsReceived++;

        EV << "[FireServer] Telemetry from sensor " << telem->getSensorId()
           << " temp=" << telem->getTemperature()
           << " delay=" << delay << "s" << endl;

        delete telem;
        return;
    }

    // Unknown packet type
    EV << "[FireServer] Unknown packet type: " << msg->getName() << endl;
    delete msg;
}

void FireServer::finish()
{
    // Record final scalars
    recordScalar("totalPacketsReceived",    packetsReceived);
    recordScalar("totalAlarmsReceived",     alarmsReceived);
    recordScalar("totalFalseAlarms",        falseAlarmsReceived);
    recordScalar("totalTelemetryReceived",  telemetryReceived);

    // PDR is computed in post-processing (Python):
    // PDR = totalPacketsReceived / totalPacketsSent (from SensorApp scalars)

    EV << "[FireServer] Simulation ended." << endl;
    EV << "  Total packets received : " << packetsReceived    << endl;
    EV << "  Alarms received        : " << alarmsReceived     << endl;
    EV << "  False alarms           : " << falseAlarmsReceived << endl;
    EV << "  Telemetry received     : " << telemetryReceived  << endl;
}
