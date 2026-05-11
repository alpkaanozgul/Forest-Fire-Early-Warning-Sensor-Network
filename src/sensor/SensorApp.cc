#include "SensorApp.h"
#include "../utils/RngUtils.h"
#include <cmath>

using namespace forestfiresim;


Define_Module(SensorApp);

//
// SensorApp.cc
// The heart of the simulation. Each sensor node runs this module.
//
// Event flow:
//   initialize()
//     -> scheduleTelemetry()  [schedules first telemetryTimer]
//
//   handleMessage(telemetryTimer)
//     -> sendTelemetry()      [builds TelemetryMsg, sends down to LoRa]
//     -> checkFalseAlarm()    [Bernoulli trial for false alarm]
//     -> scheduleTelemetry()  [reschedule next telemetry]
//
//   handleMessage("FireInZone" from FireGen)
//     -> handleFireNotification()
//        -> Bernoulli(pd) trial
//        -> if detected: build AlarmMsg, send down to LoRa
//
// RNG stream allocation (per node, offset by nodeId to ensure independence):
//   stream 0 + nodeId*5 + 0 : telemetry inter-arrival times (Exponential)
//   stream 0 + nodeId*5 + 1 : temperature readings (Normal)
//   stream 0 + nodeId*5 + 2 : humidity readings (Normal)
//   stream 0 + nodeId*5 + 3 : smoke level (Uniform / elevated)
//   stream 0 + nodeId*5 + 4 : Bernoulli trials (detection + false alarm)
//

void SensorApp::initialize()
{
    // Read parameters from NED / INI
    nodeId                = par("nodeId");
    zoneId                = par("zoneId");
    meanTelemetryInterval = par("meanTelemetryInterval").doubleValue();
    fireDetectionProb     = par("fireDetectionProb");
    falseAlarmProb        = par("falseAlarmProb");
    baseTempMu            = par("baseTempMu");
    baseTempSigma         = par("baseTempSigma");
    baseHumMu             = par("baseHumMu");
    baseHumSigma          = par("baseHumSigma");
    tempThreshold         = par("tempThreshold");
    smokeThreshold        = par("smokeThreshold");

    // Counters
    telemetrySent         = 0;
    alarmsSent            = 0;
    trueAlarmsGenerated   = 0;
    falseAlarmsGenerated  = 0;

    // Register output signals for @statistic
    alarmsSentSignal     = registerSignal("alarmsSent");
    telemetrySentSignal  = registerSignal("telemetrySent");

    // Create self-message token for the telemetry timer
    telemetryTimer = new cMessage("TelemetryTimer");

    // Schedule the first telemetry event
    scheduleTelemetry();

    EV << "[SensorApp " << nodeId << "] initialized in zone " << zoneId << endl;
}

void SensorApp::handleMessage(cMessage *msg)
{
    if (msg == telemetryTimer) {
        // ---- Periodic telemetry cycle ----
        sendTelemetry();
        checkFalseAlarm();
        scheduleTelemetry();   // schedule next

    } else if (strcmp(msg->getName(), "FireInZone") == 0) {
        // ---- Fire notification from FireGen ----
        handleFireNotification(msg);
        delete msg;

    } else {
        // ---- Packet coming UP from LoRa layer (e.g. downlink ACK) ----
        // For this project we don't process downlink; just delete
        EV << "[SensorApp " << nodeId << "] received unknown message: "
           << msg->getName() << endl;
        delete msg;
    }
}

// ---------------------------------------------------------------
// Schedule next telemetry using Exp(1/meanTelemetryInterval)
// This implements the inverse transform: t = -mean * ln(U)
// ---------------------------------------------------------------
void SensorApp::scheduleTelemetry()
{
    int rngStream = nodeId * 5 + 0;
    double interval = RngUtils::exponentialRV(1.0 / meanTelemetryInterval, rngStream);
    scheduleAt(simTime() + interval, telemetryTimer);
}

// ---------------------------------------------------------------
// Build a TelemetryMsg with Normal-distributed readings and send
// it down to the LoRa layer
// ---------------------------------------------------------------
void SensorApp::sendTelemetry()
{
    TelemetryMsg *pkt = new TelemetryMsg("Telemetry");
    pkt->setSensorId(nodeId);
    pkt->setTimestamp(simTime().dbl());
    pkt->setTemperature(readTemperature());
    pkt->setHumidity(readHumidity());
    pkt->setSmokeLevel(readSmokeLevel(false));  // normal conditions
    pkt->setWindSpeed(RngUtils::normalRV(3.0, 1.0, nodeId * 5 + 3));
    pkt->setNearThreshold(pkt->getTemperature() > tempThreshold * 0.8);

    send(pkt, "lowerLayerOut");

    telemetrySent++;
    emit(telemetrySentSignal, 1);

    EV << "[SensorApp " << nodeId << "] Telemetry sent at t=" << simTime() << endl;
}

// ---------------------------------------------------------------
// Handle fire notification from FireGen.
// Only react if the fire is in our zone.
// Bernoulli(pd) trial decides detection.
// ---------------------------------------------------------------
void SensorApp::handleFireNotification(cMessage *msg)
{
    int fireZone = (int)msg->par("zoneId").longValue();

    // Only sensors in the affected zone respond
    if (fireZone != zoneId) {
        EV << "[SensorApp " << nodeId << "] Fire in zone " << fireZone
           << ", I am in zone " << zoneId << " -- ignoring." << endl;
        return;
    }

    EV << "[SensorApp " << nodeId << "] Fire in my zone " << zoneId
       << " at t=" << simTime() << endl;

    // Bernoulli(pd) detection trial
    int rngStream = nodeId * 5 + 4;
    int detected  = RngUtils::bernoulliRV(fireDetectionProb, rngStream);

    if (detected) {
        // Build and send alarm packet
        AlarmMsg *alarm = new AlarmMsg("Alarm");
        alarm->setSensorId(nodeId);
        alarm->setTimestamp(simTime().dbl());
        alarm->setSensorReading(readSmokeLevel(true));  // elevated reading
        alarm->setIsFire(true);
        alarm->setIsFalseAlarm(false);
        alarm->setZoneId(zoneId);

        // Give alarm higher priority by setting kind=1
        // (GatewayQueue can use this for priority scheduling)
        alarm->setKind(1);

        send(alarm, "lowerLayerOut");

        alarmsSent++;
        trueAlarmsGenerated++;
        emit(alarmsSentSignal, 1);

        EV << "[SensorApp " << nodeId << "] ALARM sent (true fire detected)" << endl;
    } else {
        EV << "[SensorApp " << nodeId << "] Fire in zone but NOT detected (missed)" << endl;
    }
}

// ---------------------------------------------------------------
// False alarm check: each telemetry cycle, Bernoulli(pf) trial
// If triggered, send a false alarm packet
// ---------------------------------------------------------------
void SensorApp::checkFalseAlarm()
{
    int rngStream  = nodeId * 5 + 4;
    int falseAlarm = RngUtils::bernoulliRV(falseAlarmProb, rngStream);

    if (falseAlarm) {
        AlarmMsg *alarm = new AlarmMsg("FalseAlarm");
        alarm->setSensorId(nodeId);
        alarm->setTimestamp(simTime().dbl());
        alarm->setSensorReading(readSmokeLevel(false) + 0.3);  // spurious spike
        alarm->setIsFire(false);
        alarm->setIsFalseAlarm(true);
        alarm->setZoneId(zoneId);
        alarm->setKind(1);

        send(alarm, "lowerLayerOut");

        alarmsSent++;
        falseAlarmsGenerated++;
        emit(alarmsSentSignal, 1);

        EV << "[SensorApp " << nodeId << "] FALSE ALARM sent at t=" << simTime() << endl;
    }
}

// ---------------------------------------------------------------
// Sensor reading generators using Normal distribution (Box-Muller)
// ---------------------------------------------------------------
double SensorApp::readTemperature()
{
    return RngUtils::normalRV(baseTempMu, baseTempSigma, nodeId * 5 + 1);
}

double SensorApp::readHumidity()
{
    return RngUtils::normalRV(baseHumMu, baseHumSigma, nodeId * 5 + 2);
}

double SensorApp::readSmokeLevel(bool fireNearby)
{
    // Normal baseline smoke level, clipped to [0,1]
    double base = RngUtils::normalRV(0.1, 0.05, nodeId * 5 + 3);
    if (fireNearby) base += 0.6 + RngUtils::uniformRV(0, 0.3, nodeId * 5 + 3);
    if (base < 0) base = 0;
    if (base > 1) base = 1;
    return base;
}

void SensorApp::finish()
{
    // Record final scalars to .sca file for output analysis
    recordScalar("telemetrySent",        telemetrySent);
    recordScalar("alarmsSent",           alarmsSent);
    recordScalar("trueAlarmsGenerated",  trueAlarmsGenerated);
    recordScalar("falseAlarmsGenerated", falseAlarmsGenerated);

    EV << "[SensorApp " << nodeId << "] finish(): telemetry="
       << telemetrySent << " alarms=" << alarmsSent << endl;
}
