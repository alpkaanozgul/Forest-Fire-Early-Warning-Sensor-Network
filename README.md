# ForestFireSim — Forest Fire Early Warning Sensor Network

**Course:** CNG 476 – System Simulation, Spring 2025-2026  
**Institution:** METU Northern Cyprus Campus  
**Authors:** Alp Kaan Özgül (2638096) · Yasin Bora Buğdacı (2584753)

> This project implements the simulation described in `2638096_2584753.pdf` and was
> developed following the requirements in `Final Project Report Submission Guidelines.pdf`.

---

## Table of Contents

1. [Project Description](#project-description)
2. [System Architecture](#system-architecture)
3. [Stochastic Models](#stochastic-models)
4. [Prerequisites](#prerequisites)
5. [Build Instructions](#build-instructions)
6. [Running the Simulation](#running-the-simulation)
7. [Experiment Configurations](#experiment-configurations)
8. [Analysing Results](#analysing-results)
9. [Key Parameters](#key-parameters)
10. [Performance Metrics](#performance-metrics)
11. [Project Structure](#project-structure)
12. [Implementation Highlights](#implementation-highlights)

---

## Project Description

ForestFireSim is a discrete-event simulation (DES) built with **OMNeT++ 6.2** that models a
LoRa/LoRaWAN-based forest fire early warning sensor network.

The system is motivated by the proposal in `2638096_2584753.pdf`: deploying low-power wireless
sensors across a forested area to detect fire events (elevated temperature, smoke, humidity drop)
and relay alarms to a central monitoring server through LoRa gateways. The simulation lets us
measure key quality-of-service metrics — packet delivery ratio, end-to-end alarm delay, gateway
utilisation and queue drop rate — under varying loads and topologies.

The gateway queue is modelled as a **two-class priority M/M/1/K queue** (finite buffer K=10,
exponential service times). Analytical M/M/1/K steady-state values are computed alongside
simulation results to validate the model, as required by the submission guidelines (§6, §8).

---

## System Architecture

<img width="723" height="346" alt="image" src="https://github.com/user-attachments/assets/bd885e7a-c314-4343-bb7b-842afc87c0a5" />


## Stochastic Models

| Variable | Distribution | Parameters | RNG stream |
|---|---|---|---|
| Telemetry interval | Exponential | mean = T = 30 s | stream 0 |
| Fire event inter-arrival | Exponential | rate λ_fire = 0.005 /s | stream 1 |
| Fire detection | Bernoulli | p_d = 0.90 | stream 2 |
| False alarm | Bernoulli | p_f = 0.01 per cycle | stream 2 |
| Temperature reading | Normal (Box-Muller) | μ=25 °C, σ=3 °C | stream 3 |
| Gateway service time | Exponential | mean = 5 s (μ=0.2/s) | stream 0 |

All RNG streams use **Mersenne Twister** (`cMersenneTwister`), seeded per replication via
`seed-set = ${repetition}` to guarantee independent Monte Carlo runs.

---

## Prerequisites

| Software | Version | Notes |
|---|---|---|
| OMNeT++ | 6.2 | Must be compiled and on `PATH` |
| INET Framework | 4.4+ | Required for IP/Ethernet backhaul |
| FLoRa | 1.0 | LoRa physical layer |
| Python | 3.8+ | For `simulations/analyse.py` |
| GNU Make | any | Ships with OMNeT++ on Linux |

---

## Build Instructions

```bash
# 1. Enter the project directory
cd ~/omnetpp-6.2.0/samples/ForestFireSim

# 2. Source the OMNeT++ environment if not already done
source ~/omnetpp-6.2.0/setenv

# 3. Build (release mode, optimised)
make MODE=release -j$(nproc)

# 4. Or debug build (detailed EV logging)
make MODE=debug -j$(nproc)
```

The build produces binaries `ForestFireSim` (release) and `ForestFireSim_dbg` (debug).

> **IDE alternative:** Import the project in the OMNeT++ IDE, ensure `inet4.4` and `flora`
> are listed under *Project References*, then right-click → *Build Project*.

---

## Running the Simulation

### Graphical (Qtenv) — for inspection

```bash
./ForestFireSim simulations/omnetpp.ini -c Baseline -r 0
```

Or from the IDE: right-click `simulations/omnetpp.ini` → *Run As → OMNeT++ Simulation*,
choose a Config and click Run.

### Batch (Cmdenv) — for all 30 replications of all configs

```bash
# Run every config defined in omnetpp.ini (≈ 6 configs × 30 reps each)
opp_runall -j$(nproc) ./ForestFireSim simulations/omnetpp.ini
```

Results (`.sca` scalar files) are written to `simulations/results/`.  
Vector files (`.vec`, `.vci`) are excluded from git due to size.

---

## Experiment Configurations

| Config | Swept parameter | Values | Fixed |
|---|---|---|---|
| `Baseline` | — | — | N=8, G=2, λ=0.005 |
| `DenseNodes` | numNodes N | 4, 8, 12, 16, 20 | G=2, λ=0.005 |
| `MultiGateway` | numGateways G | 1, 2, 3 | N=8, λ=0.005 |
| `HighFireRate` | λ_fire | 0.001, 0.005, 0.01, 0.02 | N=8, G=2 |
| `FixedInterval` | T_telem | 30 s | N=8, G=2, λ=0.005 |
| `AdaptiveInterval` | T_telem | 15 s | N=8, G=2, λ=0.005 |

Each config runs **30 independent replications** (warm-up 300 s, total sim-time 3600 s).

---

## Analysing Results

```bash
# From the project root
python3 simulations/analyse.py
```

The script:

1. Parses all `.sca` files in `simulations/results/`
2. Groups runs by `(configname, iterationvars)`
3. Computes mean, 95 % confidence interval (t_{0.025,29} = 2.045), and std. dev. per KPI
4. Computes exact **M/M/1/K** theoretical values for comparison
5. Prints **Table 13** (report-ready) as required by the submission guidelines (§8)

Sample output sections:

```
===========================================================================
  ForestFireSim — TABLE 13: Simulation Results Summary
  CNG 476 System Simulation · Spring 2025-2026 · METU NCC
===========================================================================

===========================================================================
  SIMULATION vs M/M/1/K THEORY  (per-gateway metrics)
===========================================================================

===========================================================================
  M/M/1/K THEORETICAL QUICK REFERENCE  (μ=0.2/s, K=10)
===========================================================================
```

---

## Key Parameters

| Parameter | Symbol | Value | Rationale |
|---|---|---|---|
| Sensor nodes | N | 8 (baseline) | 4 per zone, matches architecture diagram |
| Gateways | G | 2 | One per zone |
| Zones | — | 2 | Zone A: nodes 0-3 · Zone B: nodes 4-7 |
| Telemetry interval | T | 30 s | Realistic LoRa duty-cycle budget |
| Service rate | μ | 0.2 pkt/s | LoRa SF12 airtime ≈ 5 s |
| Buffer capacity | K | 10 | Finite queue; ρ≈0.72 at baseline → low loss |
| Fire rate | λ_fire | 0.005 /s | ≈1 fire event per 200 s across forest |
| Detection prob. | p_d | 0.90 | Sensor reliability |
| False-alarm prob. | p_f | 0.01 | Per telemetry cycle |
| Replications | — | 30 | Required by guidelines §5 |
| Warm-up | — | 300 s | Transient removal before steady state |

---

## Performance Metrics

| Metric | Description |
|---|---|
| D_alarm (s) | Mean fire-alarm end-to-end delay (FireGen → Server) |
| Alarm E2E (s) | Mean alarm packet sojourn time in gateway queue |
| Telemetry E2E (s) | Mean telemetry packet sojourn time |
| Utilisation | Fraction of time gateway server is busy |
| Queue Lq (pkts) | Mean number of packets waiting in queue |
| Wait Wq (s) | Mean waiting time in queue (Little's law: Wq = Lq / λ_eff) |
| Drop rate | Fraction of arrivals dropped due to full buffer |
| PDR (%) | Packet delivery ratio: alarms received / alarms sent × 100 |

### M/M/1/K Steady-State Formulas Used for Validation

```

ρ  = λ / μ

P0 = (1 − ρ) / (1 − ρ^(K+1))       [ρ ≠ 1]

Pn = P0 · ρ^n

L  = Σ n·Pn   (n = 0..K)

λ_eff = λ · (1 − P_K)

Lq = L − (1 − P0)

Wq = Lq / λ_eff

W  = L  / λ_eff

```

---

## Project Structure

```
ForestFireSim/
├── src/
│   ├── ForestFire.ned          Top-level network topology
│   ├── fire/
│   │   ├── FireGen.ned
│   │   ├── FireGen.h
│   │   └── FireGen.cc          Poisson fire source with zone targeting
│   ├── gateway/
│   │   ├── GatewayQueue.ned
│   │   ├── GatewayQueue.h
│   │   └── GatewayQueue.cc     Two-class priority M/M/1/K queue
│   ├── sensor/
│   │   ├── SensorApp.ned
│   │   ├── SensorApp.h
│   │   └── SensorApp.cc        Telemetry + alarm generation
│   ├── server/
│   │   ├── FireServer.ned
│   │   ├── FireServer.h
│   │   └── FireServer.cc       Packet reception & statistics
│   └── utils/
│       ├── RngUtils.h
│       └── RngUtils.cc         Inverse-transform RNG helpers
├── msg/
│   ├── TelemetryMsg.msg        Telemetry packet fields
│   └── AlarmMsg.msg            Alarm packet fields
├── simulations/
│   ├── omnetpp.ini             All six experiment configurations
│   ├── analyse.py              Python analysis: CI95, theory vs sim
│   └── results/                .sca scalar files (git-tracked)
│       └── *.sca
├── 2638096_2584753.pdf         Project proposal
├── Final Project Report Submission Guidelines.pdf
├── architecture.png            System diagram
├── Makefile
└── README.md
```

---

## Implementation Highlights

**Priority queuing** — `GatewayQueue` maintains two separate FIFO buffers:
- `alarmBuffer` (high-priority, `msg->getKind() == 1`): fire alarms are always dequeued first.
- `telemetryBuffer` (low-priority): routine sensor readings served only when no alarm is waiting.

**Zone-based routing** — nodes are assigned to gateways by zone: `node[i]` connects to
`gatewayQueue[i / (numNodes / numGateways)]`, so each gateway handles exactly one zone's traffic.

**RNG isolation** — each stochastic process uses a dedicated stream index (0–4) ensuring
independent, reproducible variates across modules; Mersenne Twister provides period > 10^6000.

**Inverse transform** — all exponential and Normal (Box-Muller) variates are drawn via
hand-coded inverse-transform methods in `RngUtils`, not OMNeT++ built-ins, to satisfy the
course requirement of implementing RNG techniques from first principles.
