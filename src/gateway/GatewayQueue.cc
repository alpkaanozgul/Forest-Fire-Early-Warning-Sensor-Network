#include "GatewayQueue.h"
#include "../utils/RngUtils.h"

Define_Module(GatewayQueue);

//
// GatewayQueue: M/M/1/K single-server FIFO queue.
//
// Arrivals: packets from sensors (Poisson in steady state).
// Service:  Exp(1/meanServiceTime) via inverse transform (RNG stream 10).
//
// Analytical M/M/1/K baseline (for report comparison):
//   rho = lambda/mu
//   P0  = 1 / sum_{n=0}^{K} rho^n
//   Lq  = rho^2*(1-(K*rho^(K-1) - (K-1)*rho^K)) / ((1-rho)^2 * sum_{n=0}^{K} rho^n)
//

void GatewayQueue::initialize()
{
    capacity        = par("capacity");
    meanServiceTime = par("meanServiceTime").doubleValue();
    serverBusy      = false;
    pktInService    = nullptr;
    dropCount       = 0;
    totalArrivals   = 0;
    totalServed     = 0;

    serviceEndEvent = new cMessage("ServiceEnd");

    queueLengthSignal = registerSignal("queueLength");
    waitingTimeSignal = registerSignal("waitingTime");
    utilisationSignal = registerSignal("utilisation");
    dropCountSignal   = registerSignal("dropCount");
    serviceTimeSignal = registerSignal("serviceTime");

    emit(queueLengthSignal, 0L);
    emit(utilisationSignal, 0.0);

    EV << "[GatewayQueue] initialized. capacity=" << capacity
       << " meanServiceTime=" << meanServiceTime << "s" << endl;
}

void GatewayQueue::handleMessage(cMessage *msg)
{
    if (msg == serviceEndEvent) {
        endService();
    } else {
        totalArrivals++;
        msg->addPar("arrivalTime") = simTime().dbl();

        bool isAlarm = (msg->getKind() == 1);
        int totalQueued = (int)(alarmBuffer.size() + telemetryBuffer.size());

        if (!serverBusy) {
            startService(msg);
        } else if (totalQueued < capacity) {
            if (isAlarm)
                alarmBuffer.push(msg);
            else
                telemetryBuffer.push(msg);
            long depth = (long)(alarmBuffer.size() + telemetryBuffer.size());
            emit(queueLengthSignal, depth);
            EV << "[GatewayQueue] Queued (" << (isAlarm ? "ALARM" : "telemetry")
               << "). alarms=" << alarmBuffer.size()
               << " telemetry=" << telemetryBuffer.size() << endl;
        } else {
            dropCount++;
            emit(dropCountSignal, 1L);
            EV << "[GatewayQueue] DROP (buffer full, K=" << capacity
               << "). Total drops=" << dropCount << endl;
            delete msg;
        }
    }
}

// Start serving pkt; draw Exp(mu) service time via inverse transform.
void GatewayQueue::startService(cMessage *pkt)
{
    serverBusy   = true;
    pktInService = pkt;   // safe C++ member variable, no cPar trickery

    double arrivalTime = pkt->par("arrivalTime").doubleValue();
    double waitTime    = simTime().dbl() - arrivalTime;
    emit(waitingTimeSignal, waitTime);
    emit(utilisationSignal, 1.0);

    // Exponential service time: inverse transform, RNG stream 10
    double svcTime = RngUtils::exponentialRV(1.0 / meanServiceTime, 0);
    emit(serviceTimeSignal, svcTime);

    scheduleAt(simTime() + svcTime, serviceEndEvent);

    EV << "[GatewayQueue] Service started. svcTime=" << svcTime
       << "s waitTime=" << waitTime << "s" << endl;
}

// End service: forward packet, dequeue alarm first (priority), then telemetry.
void GatewayQueue::endService()
{
    ASSERT(pktInService != nullptr);
    cMessage *pkt = pktInService;
    pktInService  = nullptr;

    send(pkt, "out");
    totalServed++;

    EV << "[GatewayQueue] Service done. Forwarding to server." << endl;

    cMessage *next = nullptr;
    if (!alarmBuffer.empty()) {
        next = alarmBuffer.front();
        alarmBuffer.pop();
        EV << "[GatewayQueue] Dequeuing ALARM (priority)." << endl;
    } else if (!telemetryBuffer.empty()) {
        next = telemetryBuffer.front();
        telemetryBuffer.pop();
    }

    if (next) {
        emit(queueLengthSignal, (long)(alarmBuffer.size() + telemetryBuffer.size()));
        startService(next);
    } else {
        serverBusy = false;
        emit(utilisationSignal, 0.0);
        emit(queueLengthSignal, 0L);
    }
}

void GatewayQueue::finish()
{
    recordScalar("totalArrivals",     totalArrivals);
    recordScalar("totalServed",       totalServed);
    recordScalar("totalDrops",        dropCount);
    recordScalar("finalQueueLength",  (long)(alarmBuffer.size() + telemetryBuffer.size()));

    double dropRate = (totalArrivals > 0) ? (double)dropCount / totalArrivals : 0.0;
    recordScalar("dropRate", dropRate);

    EV << "[GatewayQueue] finish(): arrivals=" << totalArrivals
       << " served=" << totalServed
       << " drops=" << dropCount
       << " dropRate=" << dropRate << endl;

    cancelAndDelete(serviceEndEvent);
    serviceEndEvent = nullptr;

    if (pktInService) {
        delete pktInService;
        pktInService = nullptr;
    }
    while (!alarmBuffer.empty()) {
        delete alarmBuffer.front();
        alarmBuffer.pop();
    }
    while (!telemetryBuffer.empty()) {
        delete telemetryBuffer.front();
        telemetryBuffer.pop();
    }
}
