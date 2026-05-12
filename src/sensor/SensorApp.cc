#include "SensorApp.h"
#include "../utils/RngUtils.h"
#include <cmath>

using namespace forestfiresim;

Define_Module(SensorApp);

//
// SensorApp event flow:
//   initialize() -> scheduleTelemetry()
//   telemetryTimer -> sendTelemetry() -> checkFalseAlarm() -> scheduleTelemetry()
//   "FireInZone" from FireGen -> handleFireNotification() -> Bernoulli trial -> AlarmMsg
//
// RNG stream allocation (each node uses its own streams for statistical independence):
//   nodeId*5+0 : Exponential telemetry intervals
//   nodeId*5+1 : Normal temperature readings
//   nodeId*5+2 : Normal humidity readings
//   nodeId*5+3 : Smoke/wind readings
//   nodeId*5+4 : Bernoulli detection and false-alarm trials
//

void SensorApp::initialize()
{
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

    telemetrySent        = 0;
    alarmsSent           = 0;
    trueAlarmsGenerated  = 0;
    falseAlarmsGenerated = 0;

    alarmsSentSignal    = registerSignal("alarmsSent");
    telemetrySentSignal = registerSignal("telemetrySent");

    telemetryTimer = new cMessage("TelemetryTimer");
    scheduleTelemetry();

    EV << "[SensorApp " << nodeId << "] init zone=" << zoneId << endl;
}

void SensorApp::handleMessage(cMessage *msg)
{
    if (msg == telemetryTimer) {
        sendTelemetry();
        checkFalseAlarm();
        scheduleTelemetry();
    } else if (strcmp(msg->getName(), "FireInZone") == 0) {
        handleFireNotification(msg);
        delete msg;
    } else {
        // Incoming packet from lower layer (e.g. downlink ACK) - not used
        delete msg;
    }
}

// Exponential inter-arrival: t = -mean * ln(U)   (inverse CDF, from RngUtils)
void SensorApp::scheduleTelemetry()
{
    double interval = RngUtils::exponentialRV(1.0 / meanTelemetryInterval, 0);
    scheduleAt(simTime() + interval, telemetryTimer);
}

void SensorApp::sendTelemetry()
{
    TelemetryMsg *pkt = new TelemetryMsg("Telemetry");
    pkt->setSensorId(nodeId);
    pkt->setTimestamp(simTime().dbl());
    pkt->setTemperature(readTemperature());
    pkt->setHumidity(readHumidity());
    pkt->setSmokeLevel(readSmokeLevel(false));
    pkt->setWindSpeed(RngUtils::normalRV(3.0, 1.0, 3));
    pkt->setNearThreshold(pkt->getTemperature() > tempThreshold * 0.8);

    send(pkt, "socketOut");   // IApp gate name

    telemetrySent++;
    emit(telemetrySentSignal, 1);
    EV << "[SensorApp " << nodeId << "] Telemetry t=" << simTime() << endl;
}

void SensorApp::handleFireNotification(cMessage *msg)
{
    int fireZone          = (int)msg->par("zoneId").longValue();
    double fireEventTime  = msg->par("timestamp").doubleValue();

    if (fireZone != zoneId) return;

    EV << "[SensorApp " << nodeId << "] Fire in zone " << zoneId
       << " at t=" << simTime() << endl;

    // Bernoulli(pd) detection trial -- inverse transform for discrete distribution
    int detected = RngUtils::bernoulliRV(fireDetectionProb, 4);

    if (detected) {
        AlarmMsg *alarm = new AlarmMsg("Alarm");
        alarm->setSensorId(nodeId);
        alarm->setTimestamp(simTime().dbl());
        alarm->setFireEventTimestamp(fireEventTime);  // D_alarm origin
        alarm->setSensorReading(readSmokeLevel(true));
        alarm->setIsFire(true);
        alarm->setIsFalseAlarm(false);
        alarm->setZoneId(zoneId);
        alarm->setKind(1);   // high-priority kind flag

        send(alarm, "socketOut");

        alarmsSent++;
        trueAlarmsGenerated++;
        emit(alarmsSentSignal, 1);
        EV << "[SensorApp " << nodeId << "] ALARM sent (true fire)" << endl;
    } else {
        EV << "[SensorApp " << nodeId << "] Fire missed (Bernoulli trial failed)" << endl;
    }
}

// False alarm: Bernoulli(pf) each telemetry cycle
void SensorApp::checkFalseAlarm()
{
    int falseAlarm = RngUtils::bernoulliRV(falseAlarmProb, 4);

    if (falseAlarm) {
        AlarmMsg *alarm = new AlarmMsg("FalseAlarm");
        alarm->setSensorId(nodeId);
        alarm->setTimestamp(simTime().dbl());
        alarm->setFireEventTimestamp(simTime().dbl());  // no real fire; use now
        alarm->setSensorReading(readSmokeLevel(false) + 0.3);
        alarm->setIsFire(false);
        alarm->setIsFalseAlarm(true);
        alarm->setZoneId(zoneId);
        alarm->setKind(1);

        send(alarm, "socketOut");

        alarmsSent++;
        falseAlarmsGenerated++;
        emit(alarmsSentSignal, 1);
        EV << "[SensorApp " << nodeId << "] FALSE ALARM at t=" << simTime() << endl;
    }
}

double SensorApp::readTemperature()
{
    return RngUtils::normalRV(baseTempMu, baseTempSigma, 1);
}

double SensorApp::readHumidity()
{
    return RngUtils::normalRV(baseHumMu, baseHumSigma, 2);
}

double SensorApp::readSmokeLevel(bool fireNearby)
{
    double base = RngUtils::normalRV(0.1, 0.05, 3);
    if (fireNearby) base += 0.6 + RngUtils::uniformRV(0, 0.3, 3);
    if (base < 0) base = 0;
    if (base > 1) base = 1;
    return base;
}

void SensorApp::finish()
{
    cancelAndDelete(telemetryTimer);
    telemetryTimer = nullptr;

    recordScalar("telemetrySent",        telemetrySent);
    recordScalar("alarmsSent",           alarmsSent);
    recordScalar("trueAlarmsGenerated",  trueAlarmsGenerated);
    recordScalar("falseAlarmsGenerated", falseAlarmsGenerated);

    EV << "[SensorApp " << nodeId << "] finish(): telemetry="
       << telemetrySent << " alarms=" << alarmsSent << endl;
}
