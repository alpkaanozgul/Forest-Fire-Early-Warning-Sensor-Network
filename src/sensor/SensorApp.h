#ifndef SENSORAPP_H
#define SENSORAPP_H

#include <omnetpp.h>
#include "../../msg/TelemetryMsg_m.h"
#include "../../msg/AlarmMsg_m.h"

using namespace omnetpp;
using namespace forestfiresim;

//
// SensorApp: LoRa sensor node application.
// Runs inside FLoRa's LoRaNode as an IApp implementation.
//
class SensorApp : public cSimpleModule
{
private:
    int    nodeId;
    int    zoneId;
    double meanTelemetryInterval;
    double fireDetectionProb;
    double falseAlarmProb;
    double baseTempMu, baseTempSigma;
    double baseHumMu,  baseHumSigma;
    double tempThreshold, smokeThreshold;

    cMessage *telemetryTimer;

    int telemetrySent;
    int alarmsSent;
    int trueAlarmsGenerated;
    int falseAlarmsGenerated;

    simsignal_t alarmsSentSignal;
    simsignal_t telemetrySentSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    void scheduleTelemetry();
    void sendTelemetry();
    void handleFireNotification(cMessage *msg);
    void checkFalseAlarm();
    double readTemperature();
    double readHumidity();
    double readSmokeLevel(bool fireNearby);
};

#endif
