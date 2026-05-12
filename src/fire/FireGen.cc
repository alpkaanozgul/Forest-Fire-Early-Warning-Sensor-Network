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
    fireRate        = par("fireRate");
    influenceRadius = par("influenceRadius");
    numNodes        = par("numNodes");
    numZones        = par("numZones");
    totalFireEvents = 0;

    fireEvent = new cMessage("FireIgnitionEvent");
    scheduleNextFire();

    EV << "[FireGen] init. λ_fire=" << fireRate
       << "/s  numZones=" << numZones
       << "  numNodes=" << numNodes << endl;
}

void FireGen::handleMessage(cMessage *msg)
{
    ASSERT(msg == fireEvent);

    totalFireEvents++;

    // Uniform zone selection: Discrete Uniform {0 .. numZones-1}
    // (Uses inverse transform for discrete distributions via discreteUniformRV)
    int zoneId = RngUtils::discreteUniformRV(numZones, 0) - 1;

    EV << "[FireGen] Ignition #" << totalFireEvents
       << "  zone=" << zoneId
       << "  t=" << simTime() << endl;

    notifyNearbySensors(zoneId);
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
    // Sensor modules are named "node[0]", "node[1]", ... in ForestFire.ned.
    // Each node is a SensorApp simple module with a @directIn "directIn" gate.
    cModule *network = getParentModule();

    for (int i = 0; i < numNodes; i++) {
        std::string path = std::string("node[") + std::to_string(i) + "]";
        cModule *sensorMod = network->getModuleByPath(path.c_str());

        if (sensorMod) {
            cMessage *notify = new cMessage("FireInZone");
            notify->addPar("zoneId")    = zoneId;
            notify->addPar("timestamp") = simTime().dbl();  // for D_alarm
            sendDirect(notify, sensorMod, "directIn");
        }
    }
}

void FireGen::finish()
{
    recordScalar("totalFireEvents", totalFireEvents);
    EV << "[FireGen] Total ignitions: " << totalFireEvents << endl;
}
