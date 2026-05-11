#ifndef FIREGEN_H
#define FIREGEN_H

#include <omnetpp.h>

using namespace omnetpp;

//
// FireGen.h
// Global Poisson fire event generator.
// Schedules FireIgnitionEvents and notifies affected sensor nodes.
//

class FireGen : public cSimpleModule
{
private:
    // parameters
    double fireRate;
    double influenceRadius;
    int    numNodes;
    int    numZones;

    // internal self-message used to schedule next fire event
    cMessage *fireEvent;

    // statistics
    int totalFireEvents;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

private:
    // Schedule the next fire ignition using Exp(fireRate)
    void scheduleNextFire();

    // Notify all sensor nodes within influenceRadius of (x,y)
    void notifyNearbySensors(int zoneId);
};

#endif
