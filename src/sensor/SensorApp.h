#ifndef SENSORAPP_H
#define SENSORAPP_H

#include <omnetpp.h>
#include "../../msg/TelemetryMsg_m.h"
#include "../../msg/AlarmMsg_m.h"

using namespace omnetpp;
using namespace forestfiresim;

//
// SensorApp.h
// Application module running on each sensor node.
// This is the most important file you write — it drives the simulation
// from the sensor side.
//

class SensorApp : public cSimpleModule
{
private:
    // Parameters
    int    nodeId;
    int    zoneId;
    double meanTelemetryInterval;
    double fireDetectionProb;
    double falseAlarmProb;
    double baseTempMu, baseTempSigma;
    double baseHumMu,  baseHumSigma;
    double tempThreshold, smokeThreshold;

    // Self-message used to drive the periodic telemetry loop
    cMessage *telemetryTimer;

    // Counters
    int telemetrySent;
    int alarmsSent;
    int trueAlarmsGenerated;
    int falseAlarmsGenerated;

    // OMNeT++ output signals (for @statistic recording)
    simsignal_t alarmsSentSignal;
    simsignal_t telemetrySentSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    // Schedule the next telemetry event using Exp(1/meanInterval)
    void scheduleTelemetry();

    // Build and send a telemetry packet with Normal-distributed sensor readings
    void sendTelemetry();

    // Handle a fire notification from FireGen:
    // Perform Bernoulli detection trial and send AlarmMsg if detected
    void handleFireNotification(cMessage *msg);

    // Check for false alarms each telemetry cycle (Bernoulli trial)
    void checkFalseAlarm();

    // Generate a Normal-distributed sensor reading
    double readTemperature();
    double readHumidity();
    double readSmokeLevel(bool fireNearby);
};

#endif
