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
        // Packet arrival
        totalArrivals++;
        // tag arrival time for waiting-time computation
        msg->addPar("arrivalTime") = simTime().dbl();

        if (!serverBusy) {
            startService(msg);
        } else if ((int)buffer.size() < capacity) {
            buffer.push(msg);
            emit(queueLengthSignal, (long)buffer.size());
            EV << "[GatewayQueue] Queued. depth=" << buffer.size() << endl;
        } else {
            // Buffer full: M/M/1/K drop
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

// End service: forward packet, start next or go idle.
void GatewayQueue::endService()
{
    ASSERT(pktInService != nullptr);
    cMessage *pkt = pktInService;
    pktInService  = nullptr;

    send(pkt, "out");
    totalServed++;

    EV << "[GatewayQueue] Service done. Forwarding to server." << endl;

    if (!buffer.empty()) {
        cMessage *next = buffer.front();
        buffer.pop();
        emit(queueLengthSignal, (long)buffer.size());
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
    recordScalar("finalQueueLength",  (long)buffer.size());

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
    while (!buffer.empty()) {
        delete buffer.front();
        buffer.pop();
    }
}
