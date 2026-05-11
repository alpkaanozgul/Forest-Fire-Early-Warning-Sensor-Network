#ifndef GATEWAYQUEUE_H
#define GATEWAYQUEUE_H

#include <omnetpp.h>
#include <queue>

using namespace omnetpp;

//
// GatewayQueue.h
// Single-server FIFO queue with finite buffer capacity K.
// Models the M/M/1/K queue for analytical comparison.
//
// State variables maintained:
//   queue        : the actual packet buffer (std::queue)
//   serverBusy   : whether the server is currently processing a packet
//   queueLength  : current number of packets waiting (not being served)
//   dropCount    : total packets dropped due to full buffer
//
// Events:
//   Packet arrival  -> if server idle: start service immediately
//                      if server busy and queue not full: enqueue
//                      if queue full: drop packet
//   Service end     -> send packet out, start next if queue not empty
//

class GatewayQueue : public cSimpleModule
{
private:
    // Parameters
    int    capacity;
    double meanServiceTime;

    // State
    std::queue<cMessage*> buffer;   // waiting packets
    bool   serverBusy;
    int    dropCount;

    // Self-message used to signal end of service
    cMessage *serviceEndEvent;

    // Output signals for @statistic
    simsignal_t queueLengthSignal;
    simsignal_t waitingTimeSignal;
    simsignal_t utilisationSignal;
    simsignal_t dropCountSignal;
    simsignal_t serviceTimeSignal;

    // Timestamps for waiting time calculation
    // We store arrival time in the packet's timestamp parameter
    simtime_t serviceStartTime;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    void startService(cMessage *pkt);
    void endService();
};

#endif
