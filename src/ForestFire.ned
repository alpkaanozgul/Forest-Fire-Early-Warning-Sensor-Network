// ForestFire.ned -- Forest Fire Early Warning Sensor Network (DES topology)
//
// Three-tier IoT architecture (CNG 476 System Simulation, Spring 2025-2026):
//
//   Tier 1 - Sensor Nodes  : N SensorApp modules (LoRa sensor application layer)
//   Tier 2 - Gateway Tier  : G GatewayQueue modules (M/M/1/K finite-buffer FIFO)
//   Tier 3 - Server Tier   : FireServer (central fire monitoring server)
//
// Tools used:
//   OMNeT++  : discrete-event simulation engine; all events, scheduling, stats
//   FLoRa    : LoRa physical-layer model; LoRaMedium referenced for LoRa channel
//              parameters (path loss, spreading factor, bandwidth, coding rate).
//              LoRa collision and propagation delay parameters inform the service
//              time distribution used in GatewayQueue (meanServiceTime).
//   INET     : Ipv4NetworkConfigurator provides IP address management.
//              The gateway-to-server link (100 Mbps / 5 ms delay) models the
//              IP/Ethernet backhaul as specified by INET link parameters.
//
// Discrete-event packet flow:
//   FireGen --sendDirect--> SensorApp.directIn   (Poisson fire events)
//   SensorApp.socketOut  --> GatewayQueue.in[n]  (round-robin, n = i % numGateways)
//   GatewayQueue.out     --> FireServer.in[g]    (100 Mbps INET link, 5 ms delay)
//
// Key statistics collected:
//   D_alarm = fire event time --> server reception (FireServer.dAlarm)
//   E2E     = packet creation  --> server reception (FireServer.e2eDelay)
//   PDR     = received / sent  (computed in post-processing from scalars)
//   Lq, Wq, rho, drop rate    (GatewayQueue statistics)
//
// Monte Carlo: 30 independent replications per configuration (omnetpp.ini).

package forestfiresim;

import forestfiresim.sensor.SensorApp;
import forestfiresim.gateway.GatewayQueue;
import forestfiresim.server.FireServer;
import forestfiresim.fire.FireGen;

network ForestFire
{
    parameters:
        int numNodes    = default(8);    // N: number of sensor nodes (4 per zone)
        int numGateways = default(2);    // G: number of gateways (1 per zone)
        int numZones    = default(2);    // forest zones (Zone A and Zone B)

        @display("bgb=1200,900");

    submodules:

        // Sensor tier: each node runs SensorApp
        // Telemetry: Exp(1/T) inter-transmission times (inverse transform)
        // Alarms: Bernoulli(pd) detection + Bernoulli(pf) false alarms
        // Readings: Normal(mu,sigma) temperature, humidity, smoke (Box-Muller)
        node[numNodes]: SensorApp {
            parameters:
                nodeId = index;
                zoneId = int(index / (parent.numNodes / parent.numGateways));
                @display("p=150,700,row,55");
        }

        // Gateway tier: M/M/1/K FIFO queue per gateway
        // Arrivals: Poisson (from N/G sensors each)
        // Service:  Exp(1/meanServiceTime) via inverse transform
        // Models finite-buffer queuing; drops when buffer capacity K is full
        gatewayQueue[numGateways]: GatewayQueue {
            parameters:
                @display("p=600,450,row,250");
        }

        // Server tier: all packets arrive here
        // Computes D_alarm, E2E delay, PDR
        server: FireServer {
            parameters:
                numNodes = parent.numNodes;
                @display("p=1000,300");
        }

        // Global Poisson fire ignition process (lambda_fire events/s)
        // Uses inverse transform: inter-arrival = -1/lambda * ln(U)
        fireGen: FireGen {
            parameters:
                numNodes = parent.numNodes;
                numZones = parent.numZones;
                @display("p=100,100");
        }

    connections allowunconnected:

        // Sensor --> Gateway queue: zone-based assignment
        // Nodes 0..(N/G-1) --> gateway[0] (Zone A), nodes N/G..(N-1) --> gateway[1] (Zone B)
        for i=0..numNodes-1 {
            node[i].socketOut --> gatewayQueue[int(i / (numNodes / numGateways))].in++;
        }

        // Gateway queue --> Server: 100 Mbps INET link with 5 ms propagation delay
        // Models IP/Ethernet backhaul between LoRaWAN gateway and cloud server
        for j=0..numGateways-1 {
            gatewayQueue[j].out --> { datarate=100Mbps; delay=5ms; } --> server.in++;
        }
}
