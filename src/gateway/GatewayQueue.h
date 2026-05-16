#ifndef GATEWAYQUEUE_H
#define GATEWAYQUEUE_H

#include <omnetpp.h>
#include <queue>

using namespace omnetpp;

//
// GatewayQueue: single-server priority queue with finite buffer capacity K.
// Alarm packets (kind=1) are served before telemetry packets (kind=0).
// Models a two-class priority M/M/1/K queue.
//
// Priority:
//   alarmBuffer     - high priority (AlarmMsg, kind=1): fire alarms and false alarms
//   telemetryBuffer - low priority (TelemetryMsg, kind=0): routine sensor readings
//
// Events:
//   Arrival    -> if idle: start service; if busy & space: enqueue by priority; else: drop
//   ServiceEnd -> forward packet, dequeue alarm first, then telemetry
//
class GatewayQueue : public cSimpleModule
{
private:
    int    capacity;
    double meanServiceTime;

    std::queue<cMessage*> alarmBuffer;      // high-priority: alarms (kind=1)
    std::queue<cMessage*> telemetryBuffer;  // low-priority:  telemetry (kind=0)
    bool      serverBusy;
    cMessage *pktInService;
    int       dropCount;
    int       totalArrivals;
    int       totalServed;

    cMessage *serviceEndEvent;

    simsignal_t queueLengthSignal;
    simsignal_t waitingTimeSignal;
    simsignal_t utilisationSignal;
    simsignal_t dropCountSignal;
    simsignal_t serviceTimeSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    void startService(cMessage *pkt);
    void endService();
};

#endif
