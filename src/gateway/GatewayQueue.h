#ifndef GATEWAYQUEUE_H
#define GATEWAYQUEUE_H

#include <omnetpp.h>
#include <queue>

using namespace omnetpp;

//
// GatewayQueue: single-server FIFO queue with finite buffer capacity K.
// Models the M/M/1/K queue for analytical comparison.
//
// State variables:
//   buffer       - FIFO queue of waiting packets
//   serverBusy   - true when the server is processing a packet
//   pktInService - pointer to the packet currently being served
//   dropCount    - total packets dropped due to full buffer
//
// Events:
//   Arrival  -> if idle: start service; if busy & space: enqueue; else: drop
//   ServiceEnd -> forward packet out, start next if queue not empty
//
class GatewayQueue : public cSimpleModule
{
private:
    int    capacity;
    double meanServiceTime;

    std::queue<cMessage*> buffer;
    bool      serverBusy;
    cMessage *pktInService;   // packet currently being served (safe pointer storage)
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
