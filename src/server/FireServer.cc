#include "FireServer.h"

using namespace omnetpp;
using namespace forestfiresim;

Define_Module(FireServer);

//
// FireServer: collects all statistics at the simulation sink.
//
// D_alarm = simTime() - alarm->getFireEventTimestamp()
//   -> measures the true end-to-end alarm delay from fire occurrence.
//
// E2E delay = simTime() - msg->getTimestamp()
//   -> measures transmission latency from when the sensor created the packet.
//

void FireServer::initialize()
{
    numNodes            = par("numNodes");
    packetsReceived     = 0;
    alarmsReceived      = 0;
    falseAlarmsReceived = 0;
    telemetryReceived   = 0;

    dAlarmSignal              = registerSignal("dAlarm");
    e2eDelaySignal            = registerSignal("e2eDelay");
    telemetryDelaySignal      = registerSignal("telemetryDelay");
    packetsReceivedSignal     = registerSignal("packetsReceived");
    alarmsReceivedSignal      = registerSignal("alarmsReceived");
    falseAlarmsReceivedSignal = registerSignal("falseAlarmsReceived");

    EV << "[FireServer] init. Waiting for packets on " << gateSize("in") << " gate(s)." << endl;
}

void FireServer::handleMessage(cMessage *msg)
{
    double now = simTime().dbl();

    AlarmMsg *alarmMsg = dynamic_cast<AlarmMsg*>(msg);
    if (alarmMsg) {
        double e2e    = now - alarmMsg->getTimestamp();
        double dAlarm = now - alarmMsg->getFireEventTimestamp();

        emit(e2eDelaySignal,  e2e);
        emit(dAlarmSignal,    dAlarm);
        emit(alarmsReceivedSignal, 1L);

        alarmsReceived++;
        packetsReceived++;

        if (alarmMsg->isFalseAlarm()) {
            falseAlarmsReceived++;
            emit(falseAlarmsReceivedSignal, 1L);
            EV << "[FireServer] FALSE ALARM sensor=" << alarmMsg->getSensorId()
               << " e2e=" << e2e << "s" << endl;
        } else {
            EV << "[FireServer] ALARM sensor=" << alarmMsg->getSensorId()
               << " zone=" << alarmMsg->getZoneId()
               << " D_alarm=" << dAlarm << "s e2e=" << e2e << "s" << endl;
        }
        delete alarmMsg;
        return;
    }

    TelemetryMsg *telem = dynamic_cast<TelemetryMsg*>(msg);
    if (telem) {
        double e2e = now - telem->getTimestamp();
        emit(telemetryDelaySignal, e2e);
        emit(packetsReceivedSignal, 1L);

        telemetryReceived++;
        packetsReceived++;

        EV << "[FireServer] Telemetry sensor=" << telem->getSensorId()
           << " temp=" << telem->getTemperature()
           << " e2e=" << e2e << "s" << endl;

        delete telem;
        return;
    }

    EV << "[FireServer] Unknown packet: " << msg->getName() << endl;
    delete msg;
}

void FireServer::finish()
{
    recordScalar("totalPacketsReceived",   packetsReceived);
    recordScalar("totalAlarmsReceived",    alarmsReceived);
    recordScalar("totalFalseAlarms",       falseAlarmsReceived);
    recordScalar("totalTelemetryReceived", telemetryReceived);

    EV << "[FireServer] finish():"
       << " packets=" << packetsReceived
       << " alarms="  << alarmsReceived
       << " false="   << falseAlarmsReceived
       << " telem="   << telemetryReceived << endl;
}
