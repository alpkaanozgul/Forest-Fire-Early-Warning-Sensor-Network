#include "FireGen.h"
#include "../utils/RngUtils.h"

Define_Module(FireGen);

//
// FireGen: global Poisson fire ignition generator.
//
// Inter-arrival time ~ Exp(λ_fire) via inverse transform:
//   t = -(1/λ) * ln(U),  U ~ Uniform(0,1)
//
// RNG stream 3 is reserved for FireGen.
//
// On each ignition event the generator:
//   1. Picks a random zone (Discrete Uniform)
//   2. Sends a "FireInZone" notification to every sensor via sendDirect
//      (sensors whose zoneId matches respond with a Bernoulli detection trial)
//   3. Schedules the next fire event
//
// The notification carries:
//   zoneId    - which zone ignited
//   timestamp - simTime() at ignition (stored in AlarmMsg.fireEventTimestamp
//               for D_alarm computation at FireServer)
//

void FireGen::initialize()
{
    fireRate          = par("fireRate");
    influenceRadius   = par("influenceRadius");
    numNodes          = par("numNodes");
    numZones          = par("numZones");
    numZonesToAffect  = par("numZonesToAffect");
    if (numZonesToAffect < 1) numZonesToAffect = 1;
    if (numZonesToAffect > numZones) numZonesToAffect = numZones;
    totalFireEvents   = 0;

    fireEvent = new cMessage("FireIgnitionEvent");
    scheduleNextFire();

    EV << "[FireGen] init. λ_fire=" << fireRate
       << "/s  numZones=" << numZones
       << "  numZonesToAffect=" << numZonesToAffect
       << "  numNodes=" << numNodes << endl;
}

void FireGen::handleMessage(cMessage *msg)
{
    ASSERT(msg == fireEvent);

    totalFireEvents++;

    // Pick a random starting zone, then take numZonesToAffect consecutive zones
    // (wrapping around). This guarantees the fire always spreads to exactly
    // numZonesToAffect zones, producing burst arrivals at the gateway.
    int startZone = RngUtils::discreteUniformRV(numZones, 0) - 1;

    EV << "[FireGen] Ignition #" << totalFireEvents
       << "  startZone=" << startZone
       << "  spread=" << numZonesToAffect
       << "  t=" << simTime() << endl;

    for (int k = 0; k < numZonesToAffect; k++) {
        int zoneId = (startZone + k) % numZones;
        notifyNearbySensors(zoneId);
    }

    scheduleNextFire();
}

// Exponential inter-arrival: X = -(1/λ) * ln(U)  (inverse CDF, RNG stream 3)
void FireGen::scheduleNextFire()
{
    double ia = RngUtils::exponentialRV(fireRate, 1);
    scheduleAt(simTime() + ia, fireEvent);
}

void FireGen::notifyNearbySensors(int zoneId)
{
    // Only notify nodes that belong to the affected zone.
    // Zone z contains nodes [z*nodesPerZone .. (z+1)*nodesPerZone - 1].
    cModule *network = getParentModule();
    int nodesPerZone = numNodes / numZones;
    int first = zoneId * nodesPerZone;
    int last  = first + nodesPerZone;   // exclusive

    for (int i = first; i < last; i++) {
        std::string path = std::string("node[") + std::to_string(i) + "]";
        cModule *sensorMod = network->getModuleByPath(path.c_str());

        if (sensorMod) {
            cMessage *notify = new cMessage("FireInZone");
            notify->addPar("zoneId")    = zoneId;
            notify->addPar("timestamp") = simTime().dbl();
            sendDirect(notify, sensorMod, "directIn");
        }
    }
}

void FireGen::finish()
{
    recordScalar("totalFireEvents", totalFireEvents);
    EV << "[FireGen] Total ignitions: " << totalFireEvents << endl;
}
