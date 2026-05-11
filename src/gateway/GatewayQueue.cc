#include "GatewayQueue.h"
#include "../utils/RngUtils.h"

Define_Module(GatewayQueue);

//
// GatewayQueue.cc
// Implements a single-server FIFO queue with finite buffer (M/M/1/K).
//
// This is the queueing analysis component of the project.
// Arrivals: packets from the LoRa radio (Poisson process in steady state)
// Service:  exponential service times with rate mu = 1/meanServiceTime
//
// Analytical M/M/1 baseline (for comparison in report):
//   rho  = lambda / mu
//   Lq   = rho^2 / (1 - rho)     (mean queue length)
//   Wq   = Lq / lambda            (mean waiting time)
//   Note: with finite K, use M/M/1/K formulas instead
//

void GatewayQueue::initialize()
{
    capacity        = par("capacity");
    meanServiceTime = par("meanServiceTime").doubleValue();
    serverBusy      = false;
    dropCount       = 0;

    serviceEndEvent = new cMessage("ServiceEnd");

    // Register signals
    queueLengthSignal = registerSignal("queueLength");
    waitingTimeSignal = registerSignal("waitingTime");
    utilisationSignal = registerSignal("utilisation");
    dropCountSignal   = registerSignal("dropCount");
    serviceTimeSignal = registerSignal("serviceTime");

    // Emit initial state
    emit(queueLengthSignal, 0);
    emit(utilisationSignal, 0.0);

    EV << "[GatewayQueue] initialized. capacity=" << capacity
       << " meanServiceTime=" << meanServiceTime << endl;
}

void GatewayQueue::handleMessage(cMessage *msg)
{
    if (msg == serviceEndEvent) {
        // ---- Service completion event ----
        endService();
    } else {
        // ---- Packet arrival event ----
        // Tag the packet with its arrival time so we can compute waiting time
        msg->addPar("arrivalTime") = simTime().dbl();

        if (!serverBusy) {
            // Server is idle: start service immediately (no waiting)
            startService(msg);
        } else if ((int)buffer.size() < capacity) {
            // Server busy, buffer has space: enqueue
            buffer.push(msg);
            emit(queueLengthSignal, (long)buffer.size());
            EV << "[GatewayQueue] Packet queued. Queue size=" << buffer.size() << endl;
        } else {
            // Buffer full: DROP the packet
            dropCount++;
            emit(dropCountSignal, 1L);
            EV << "[GatewayQueue] Packet DROPPED (buffer full). Total drops=" << dropCount << endl;
            delete msg;
        }
    }
}

// ---------------------------------------------------------------
// Start serving a packet.
// Draw service time from Exp(1/meanServiceTime) -- inverse transform.
// ---------------------------------------------------------------
void GatewayQueue::startService(cMessage *pkt)
{
    serverBusy = true;
    serviceStartTime = simTime();

    // Record waiting time = now - arrival time
    double arrivalTime = pkt->par("arrivalTime").doubleValue();
    double waitTime    = simTime().dbl() - arrivalTime;
    emit(waitingTimeSignal, waitTime);

    // Exponential service time: RNG stream 10 reserved for all gateway queues
    double svcTime = RngUtils::exponentialRV(1.0 / meanServiceTime, 10);
    emit(serviceTimeSignal, svcTime);
    emit(utilisationSignal, 1.0);   // server is busy

    // Store packet in serviceEndEvent so we can forward it when done
    serviceEndEvent->addPar("pkt") = (long)(intptr_t)pkt;

    scheduleAt(simTime() + svcTime, serviceEndEvent);

    EV << "[GatewayQueue] Service started for packet. svcTime=" << svcTime
       << " waitTime=" << waitTime << endl;
}

// ---------------------------------------------------------------
// End service: forward the packet out, start next if queue not empty
// ---------------------------------------------------------------
void GatewayQueue::endService()
{
    // Retrieve the packet we were serving
    cMessage *pkt = (cMessage*)(intptr_t)serviceEndEvent->par("pkt").longValue();
    serviceEndEvent->removeObject("pkt");

    // Forward packet to the next stage (INET backend)
    send(pkt, "out");

    EV << "[GatewayQueue] Service completed. Forwarding packet." << endl;

    // Check if another packet is waiting
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
    recordScalar("totalDrops",   dropCount);
    recordScalar("finalQueueLength", (long)buffer.size());

    EV << "[GatewayQueue] finish(): drops=" << dropCount
       << " remainingInQueue=" << buffer.size() << endl;

    // Clean up any remaining packets in the buffer
    while (!buffer.empty()) {
        delete buffer.front();
        buffer.pop();
    }
}
