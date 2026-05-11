#ifndef FIRESERVER_H
#define FIRESERVER_H

#include <omnetpp.h>
#include "../../msg/TelemetryMsg_m.h"
#include "../../msg/AlarmMsg_m.h"

using namespace omnetpp;
using namespace forestfiresim;

//
// FireServer.h
// Sink module. Receives all packets, records statistics.
//

class FireServer : public cSimpleModule
{
private:
    int numNodes;

    // Counters
    int packetsReceived;
    int alarmsReceived;
    int falseAlarmsReceived;
    int telemetryReceived;

    // For PDR calculation
    // We track how many packets were transmitted vs received
    // PDR = packetsReceived / totalTransmitted (obtained from scalars)

    // Output signals
    simsignal_t e2eDelaySignal;
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
