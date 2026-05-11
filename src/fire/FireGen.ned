package forestfiresim.fire;

//
// FireGen.ned
// Declares the global fire ignition generator module.
// This module runs a Poisson process of fire events and signals
// sensor nodes within a configurable influence radius.
//
simple FireGen
{
    parameters:
        double fireRate = default(0.005);      // lambda_fire: events per second
        double influenceRadius = default(200); // metres: sensors within this radius are affected
        int    numNodes = default(40);         // must match network numNodes
        int    numZones = default(4);          // how many forest zones

        @display("i=block/source;p=50,50");
}
