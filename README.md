# Forest Fire Early Warning Sensor Network
## CNG 476 - System Simulation | Spring 2025-2026
### METU Northern Cyprus Campus

**Group Members:**
- Alp Kaan Özgül (2638096)
- Yasin Bora Bugdacı (2584753)

---

## Project Overview

A discrete-event simulation of a Forest Fire Early Warning Sensor Network using OMNeT++, INET 4.4, and FLoRa 1.0.

The system models smoke, heat, humidity, and wind-speed sensors communicating via LoRa/LoRaWAN to gateways, which forward alarms to a central monitoring server over an IP backend.

---

## Requirements

- OMNeT++ 6.x
- INET 4.4
- FLoRa 1.0

---

## Setup Instructions

1. Clone this repository into your OMNeT++ workspace:
```bash
cd ~/omnetpp-6.x/samples
git clone <repo-url> ForestFireSim
```

2. Open OMNeT++ IDE, go to **File → Import → Existing Projects into Workspace**

3. Import `ForestFireSim`. Also make sure `inet4.4` and `flora` are imported.

4. Right-click `ForestFireSim` → **Properties → Project References** → check `inet4.4` and `flora`

5. Build in order: INET → FLoRa → ForestFireSim

---

## Running the Simulation

1. Right-click `simulations/omnetpp.ini` → **Run As → OMNeT++ Simulation**

2. Select a configuration (e.g. `Baseline`) and click Run

3. Results appear in `simulations/results/`

---

## File Structure

```
src/
  network/    ForestFire.ned          Top-level network topology
  sensor/     SensorApp.{ned,h,cc}    Sensor application logic
  fire/       FireGen.{ned,h,cc}      Poisson fire event generator
  gateway/    GatewayQueue.{ned,h,cc} M/M/1/K queue at each gateway
  server/     FireServer.{ned,h,cc}   Monitoring server, records stats
  utils/      RngUtils.{h,cc}         Inverse-transform RNG functions
msg/
  TelemetryMsg.msg                    Telemetry packet definition
  AlarmMsg.msg                        Alarm packet definition
simulations/
  omnetpp.ini                         All experiment configurations
```

---

## Experiment Configurations

| Config | Description |
|---|---|
| Baseline | 40 nodes, 2 gateways, fireRate=0.005 |
| DenseNodes | Sweep numNodes ∈ {20,40,60,80,100} |
| MultiGateway | Sweep numGateways ∈ {1,2,3} |
| HighFireRate | Sweep fireRate ∈ {0.001,0.005,0.01,0.02} |
| FixedInterval | Fixed 60s telemetry interval |
| AdaptiveInterval | Shorter 30s telemetry interval |

Each configuration runs 30 independent replications with different random seeds.
