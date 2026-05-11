#include "FireGen.h"
#include "../utils/RngUtils.h"

Define_Module(FireGen);

//
// FireGen.cc
// Implements a Poisson process of fire ignition events.
//
// Flow:
//   initialize() -> scheduleNextFire()
//   handleMessage() fires -> notifyNearbySensors() -> scheduleNextFire()
//
// RNG stream index 3 is reserved for FireGen (see omnetpp.ini comments).
//

void FireGen::initialize()
{
    fireRate        = par("fireRate");
    influenceRadius = par("influenceRadius");
    numNodes        = par("numNodes");
    numZones        = par("numZones");
    totalFireEvents = 0;

    // self-message token — we reuse this same object every time
    fireEvent = new cMessage("FireIgnitionEvent");

    // schedule the first fire
    scheduleNextFire();

    EV << "[FireGen] initialized. fireRate=" << fireRate
       << " numNodes=" << numNodes << endl;
}

void FireGen::handleMessage(cMessage *msg)
{
    // The only message this module ever receives is its own fireEvent
    ASSERT(msg == fireEvent);

    totalFireEvents++;

    // Pick a random zone where the fire starts
    // Discrete Uniform {0, ..., numZones-1}
    int zoneId = RngUtils::discreteUniformRV(numZones, 3) - 1;

    EV << "[FireGen] Fire ignition event #" << totalFireEvents
       << " at t=" << simTime() << " in zone " << zoneId << endl;

    // Tell all sensors in this zone that a fire occurred
    notifyNearbySensors(zoneId);

    // Schedule the next fire event
    scheduleNextFire();
}

void FireGen::scheduleNextFire()
{
    // Inter-arrival time ~ Exp(fireRate), generated via inverse transform
    // RNG stream 3 is dedicated to FireGen
    double interArrival = RngUtils::exponentialRV(fireRate, 3);
    scheduleAt(simTime() + interArrival, fireEvent);
}

void FireGen::notifyNearbySensors(int zoneId)
{
    // We signal sensor nodes by sending them a cMessage("FireInZone").
    // Each SensorApp checks its own zoneId and responds with a Bernoulli trial.
    //
    // getModuleByPath searches the simulation for sibling modules.
    // Naming convention: the network has node[0], node[1], ... node[N-1]

    cModule *network = getParentModule();

    for (int i = 0; i < numNodes; i++) {
        // Build path like "node[5].app"
        std::string path = std::string("node[") + std::to_string(i) + "].app";
        cModule *appMod = network->getModuleByPath(path.c_str());

        if (appMod != nullptr) {
            // Create a lightweight notification message
            cMessage *notify = new cMessage("FireInZone");
            notify->addPar("zoneId") = zoneId;
            notify->addPar("timestamp") = simTime().dbl();

            // Send directly into the module's message queue
            sendDirect(notify, appMod, "directIn");
        }
    }
}

void FireGen::finish()
{
    recordScalar("totalFireEvents", totalFireEvents);
    EV << "[FireGen] Simulation ended. Total fire events: "
       << totalFireEvents << endl;
}
