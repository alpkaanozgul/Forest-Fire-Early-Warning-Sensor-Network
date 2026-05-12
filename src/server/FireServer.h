#ifndef FIRESERVER_H
#define FIRESERVER_H

#include <omnetpp.h>
#include "../../msg/TelemetryMsg_m.h"
#include "../../msg/AlarmMsg_m.h"

using namespace omnetpp;
using namespace forestfiresim;

//
// FireServer: sink module. Receives all packets from gateway queues.
// Computes D_alarm (fire-event → server), E2E delay (packet creation → server),
// and PDR scalars.
//
class FireServer : public cSimpleModule
{
private:
    int numNodes;

    int packetsReceived;
    int alarmsReceived;
    int falseAlarmsReceived;
    int telemetryReceived;

    simsignal_t dAlarmSignal;          // D_alarm: fire event → server reception
    simsignal_t e2eDelaySignal;        // packet creation → server reception
    simsignal_t telemetryDelaySignal;
    simsignal_t packetsReceivedSignal;
    simsignal_t alarmsReceivedSignal;
    simsignal_t falseAlarmsReceivedSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif
