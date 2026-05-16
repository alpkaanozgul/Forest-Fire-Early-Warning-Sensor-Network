"""
analyse.py  --  ForestFireSim result analyser
CNG 476 System Simulation, Spring 2025-2026, METU NCC

Reads all .sca result files, computes 95% confidence intervals,
compares against exact M/M/1/K theoretical values, and prints
a report-ready Table 13 (Simulation Results Summary).

Run:  python3 simulations/analyse.py
"""

import os, re, math
from collections import defaultdict

RESULTS_DIR = os.path.join(os.path.dirname(__file__), "results")

# ═══════════════════════════════════════════════════════════════
# M/M/1/K THEORETICAL MODEL
# Parameters (must match omnetpp.ini):
#   meanServiceTime       = 5s   → μ = 0.2 pkt/s
#   capacity              = 10   → K = 10
#   meanTelemetryInterval = 30s
#   fireDetectionProb     = 0.90
#   falseAlarmProb        = 0.01
#   numZones=2, numZonesToAffect=1
# ═══════════════════════════════════════════════════════════════
MU  = 1.0 / 5.0    # service rate (pkt/s)
K   = 10            # buffer capacity
T   = 30.0          # mean telemetry interval (s)
PD  = 0.90          # fire detection probability
PF  = 0.01          # false alarm probability


def mm1k(lam, mu, K):
    """
    Exact M/M/1/K steady-state metrics via balance equations.
    Returns dict with rho, utilisation, Lq, Wq, drop_rate, L, W, lam_eff
    """
    if lam <= 0 or mu <= 0:
        return None
    rho = lam / mu
    if abs(rho - 1.0) < 1e-9:
        P0 = 1.0 / (K + 1)
        Pn = [P0] * (K + 1)
    else:
        P0 = (1.0 - rho) / (1.0 - rho ** (K + 1))
        Pn = [P0 * rho ** n for n in range(K + 1)]

    PK      = Pn[K]
    L       = sum(n * Pn[n] for n in range(K + 1))
    lam_eff = lam * (1.0 - PK)
    Lq      = max(0.0, L - (1.0 - P0))
    Wq      = Lq / lam_eff if lam_eff > 0 else 0.0
    W       = L  / lam_eff if lam_eff > 0 else 0.0
    util    = 1.0 - P0

    return dict(rho=rho, utilisation=util, Lq=Lq, Wq=Wq,
                drop_rate=PK, L=L, W=W, lam_eff=lam_eff)


def lam_gw(N, G, fireRate=0.005, numZones=2):
    """
    Per-gateway arrival rate (pkt/s):
      λ_telem  = nodesPerGW / T
      λ_alarm  = (fireRate/numZones) × nodesPerGW × PD   (true alarms)
      λ_false  = nodesPerGW × PF / T                     (false alarms, 1 per telem cycle)
    """
    npg      = N / G
    lam_t    = npg / T
    lam_a    = (fireRate / numZones) * npg * PD
    lam_f    = npg * PF / T
    return lam_t + lam_a + lam_f


# Pre-compute theory for every (config, itervar) pair
THEORY = {
    ("Baseline",        ""):           mm1k(lam_gw(8,  2),            MU, K),
    ("DenseNodes",      "$N=4"):       mm1k(lam_gw(4,  2),            MU, K),
    ("DenseNodes",      "$N=8"):       mm1k(lam_gw(8,  2),            MU, K),
    ("DenseNodes",      "$N=12"):      mm1k(lam_gw(12, 2),            MU, K),
    ("DenseNodes",      "$N=16"):      mm1k(lam_gw(16, 2),            MU, K),
    ("DenseNodes",      "$N=20"):      mm1k(lam_gw(20, 2),            MU, K),
    ("MultiGateway",    "$G=1"):       mm1k(lam_gw(8,  1),            MU, K),
    ("MultiGateway",    "$G=2"):       mm1k(lam_gw(8,  2),            MU, K),
    ("MultiGateway",    "$G=3"):       mm1k(lam_gw(8,  3),            MU, K),
    ("HighFireRate",    "$FR=0.001"):  mm1k(lam_gw(8,  2, 0.001),     MU, K),
    ("HighFireRate",    "$FR=0.005"):  mm1k(lam_gw(8,  2, 0.005),     MU, K),
    ("HighFireRate",    "$FR=0.01"):   mm1k(lam_gw(8,  2, 0.01),      MU, K),
    ("HighFireRate",    "$FR=0.02"):   mm1k(lam_gw(8,  2, 0.02),      MU, K),
    ("FixedInterval",   ""):           mm1k(lam_gw(8,  2),            MU, K),
    ("AdaptiveInterval",""):           mm1k(lam_gw(8,  2) + 4*(1/15.0 - 1/T), MU, K),
}


# ═══════════════════════════════════════════════════════════════
# .sca FILE PARSER
# ═══════════════════════════════════════════════════════════════
def parse_sca(path):
    config = itervar = ""
    scalars = []   # list of (module, name, value)
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line.startswith("attr configname"):
                config = line.split(None, 2)[2]
            elif line.startswith("attr iterationvars "):
                itervar = line.split(None, 2)[2].strip('"')
            elif line.startswith("scalar "):
                parts = line.split(None, 3)
                if len(parts) == 4:
                    _, module, name, value = parts
                    try:
                        scalars.append((module, name, float(value)))
                    except ValueError:
                        pass
    return config, itervar, scalars


def get_sum(scalars, pattern):
    """Sum all scalar values whose 'module name' matches regex."""
    rx = re.compile(pattern)
    return sum(v for m, n, v in scalars if rx.search(f"{m} {n}"))


def get_avg(scalars, pattern):
    """Average all scalar values whose 'module name' matches regex."""
    rx  = re.compile(pattern)
    vals = [v for m, n, v in scalars if rx.search(f"{m} {n}")]
    return sum(vals) / len(vals) if vals else 0.0


def get_count(scalars, pattern):
    rx = re.compile(pattern)
    return sum(1 for m, n, v in scalars if rx.search(f"{m} {n}"))


# ═══════════════════════════════════════════════════════════════
# STATISTICS HELPERS
# ═══════════════════════════════════════════════════════════════
def mean(vals):
    return sum(vals) / len(vals) if vals else 0.0

def stddev(vals):
    n = len(vals)
    if n < 2:
        return 0.0
    m = mean(vals)
    return math.sqrt(sum((x - m) ** 2 for x in vals) / (n - 1))

def ci95(vals):
    n = len(vals)
    if n < 2:
        return 0.0
    # t_{0.025, 29} = 2.045 for 30 reps
    return 2.045 * stddev(vals) / math.sqrt(n)


# ═══════════════════════════════════════════════════════════════
# KPI EXTRACTION  (one scalar list = one replication)
# Gateway metrics: averaged across all gatewayQueue[*] instances
# Node/server metrics: summed across all matching modules
# ═══════════════════════════════════════════════════════════════
def extract_kpis(scalars_list):
    def col_avg(pat):   # per-gateway metric → average across gateways
        return [get_avg(s, pat) for s in scalars_list]
    def col_sum(pat):   # global metric → sum across modules
        return [get_sum(s, pat) for s in scalars_list]

    util       = col_avg(r"gatewayQueue\[\d+\] utilisation:timeavg$")
    queue_len  = col_avg(r"gatewayQueue\[\d+\] queueLength:timeavg$")
    wait_time  = col_avg(r"gatewayQueue\[\d+\] waitingTime:mean$")
    drop_rate  = col_avg(r"gatewayQueue\[\d+\] dropRate$")
    d_alarm    = col_avg(r"server dAlarm:mean$")
    e2e_alarm  = col_avg(r"server e2eDelay:mean$")
    e2e_telem  = col_avg(r"server telemetryDelay:mean$")
    alarms_rx  = col_sum(r"server alarmsReceived:sum$")
    alarms_tx  = col_sum(r"node\[\d+\] alarmsSent$")
    fire_ev    = col_sum(r"fireGen totalFireEvents$")
    pkts_rx    = col_sum(r"server packetsReceived:sum$")

    pdr = [rx / max(tx, 1) * 100
           for rx, tx in zip(alarms_rx, alarms_tx)]

    return {
        "D_alarm (s)":       d_alarm,
        "Alarm E2E (s)":     e2e_alarm,
        "Telemetry E2E (s)": e2e_telem,
        "Utilisation":       util,
        "Queue Lq (pkts)":   queue_len,
        "Wait Wq (s)":       wait_time,
        "Drop rate":         drop_rate,
        "PDR (%)":           pdr,
        "Fire events":       fire_ev,
        "Alarms RX":         alarms_rx,
    }


# ═══════════════════════════════════════════════════════════════
# COLLECT AND GROUP ALL RUNS
# ═══════════════════════════════════════════════════════════════
runs = []
for fname in sorted(os.listdir(RESULTS_DIR)):
    if not fname.endswith(".sca"):
        continue
    config, itervar, scalars = parse_sca(os.path.join(RESULTS_DIR, fname))
    runs.append((config, itervar, scalars))

if not runs:
    print("No .sca files found in results/  →  run the simulation first.")
    raise SystemExit(0)

groups = defaultdict(list)
for config, itervar, scalars in runs:
    groups[(config, itervar)].append(scalars)

# ═══════════════════════════════════════════════════════════════
# PRINT HELPERS
# ═══════════════════════════════════════════════════════════════
CONFIGS = ["Baseline", "DenseNodes", "HighFireRate",
           "MultiGateway", "FixedInterval", "AdaptiveInterval"]

def fmt(vals):
    if not vals:
        return "N/A"
    return f"{mean(vals):.4f} ± {ci95(vals):.4f}"

THEORY_KEY = {
    "D_alarm (s)":       None,
    "Alarm E2E (s)":     None,
    "Telemetry E2E (s)": None,
    "Utilisation":       "utilisation",
    "Queue Lq (pkts)":   "Lq",
    "Wait Wq (s)":       "Wq",
    "Drop rate":         "drop_rate",
    "PDR (%)":           None,
}

# ═══════════════════════════════════════════════════════════════
# TABLE 13  –  report-ready summary
# ═══════════════════════════════════════════════════════════════
print()
print("=" * 78)
print("  ForestFireSim — TABLE 13: Simulation Results Summary")
print("  CNG 476 System Simulation · Spring 2025-2026 · METU NCC")
print("=" * 78)
print(f"  {'Scenario':<26} {'Metric':<20} {'Mean':>10}  {'95% CI':>10}  {'Std.Dev':>8}")
print("  " + "─" * 76)

for cfg in CONFIGS:
    keys = sorted([k for k in groups if k[0] == cfg], key=lambda k: k[1])
    if not keys:
        continue
    for key in keys:
        label  = key[1] if key[1] else "(baseline)"
        kpis   = extract_kpis(groups[key])
        first  = True
        for metric in ["D_alarm (s)", "Alarm E2E (s)", "Telemetry E2E (s)",
                       "Utilisation", "Queue Lq (pkts)", "Wait Wq (s)",
                       "Drop rate", "PDR (%)"]:
            vals   = kpis[metric]
            m_val  = mean(vals)
            ci_val = ci95(vals)
            sd_val = stddev(vals)
            scen   = f"{cfg} {label}" if first else ""
            first  = False
            print(f"  {scen:<26} {metric:<20} {m_val:>10.4f}  {ci_val:>10.4f}  {sd_val:>8.4f}")
        print("  " + "─" * 76)

# ═══════════════════════════════════════════════════════════════
# DETAILED PER-CONFIG: SIMULATION vs M/M/1/K THEORY
# ═══════════════════════════════════════════════════════════════
print()
print("=" * 78)
print("  SIMULATION vs M/M/1/K THEORY  (per-gateway metrics)")
print("=" * 78)
print(f"  {'Metric':<22} {'Simulation (mean ± 95%CI)':<28} {'Theory M/M/1/K':<16} {'Std.Dev'}")
print()

for cfg in CONFIGS:
    keys = sorted([k for k in groups if k[0] == cfg], key=lambda k: k[1])
    if not keys:
        continue
    print(f"  ── CONFIG: {cfg} " + "─" * (52 - len(cfg)))
    for key in keys:
        label = key[1] if key[1] else "(baseline)"
        kpis  = extract_kpis(groups[key])
        th    = THEORY.get(key) or THEORY.get((cfg, ""))
        n     = len(groups[key])
        rho_s = f"ρ={th['rho']:.3f}" if th else ""
        print(f"  {label}  ({n} reps)  {rho_s}")
        print(f"  {'─'*72}")
        for metric, th_key in THEORY_KEY.items():
            vals    = kpis[metric]
            sim_str = fmt(vals)
            if th and th_key and th_key in th:
                th_str = f"{th[th_key]:.4f}"
            else:
                th_str = "—"
            sd_str = f"{stddev(vals):.4f}"
            print(f"  {metric:<22} {sim_str:<28} {th_str:<16} {sd_str}")
        print()

# ═══════════════════════════════════════════════════════════════
# THEORY-ONLY QUICK REFERENCE TABLE
# ═══════════════════════════════════════════════════════════════
print("=" * 78)
print("  M/M/1/K THEORETICAL QUICK REFERENCE  (μ=0.2/s, K=10)")
print("=" * 78)
print(f"  {'Scenario':<30} {'ρ':>6}  {'Util':>6}  {'Lq':>7}  {'Wq(s)':>8}  {'Drop%':>7}")
print("  " + "─" * 70)

for cfg in CONFIGS:
    keys = sorted([k for k in groups if k[0] == cfg], key=lambda k: k[1])
    if not keys:
        continue
    for key in keys:
        label = key[1] if key[1] else "(baseline)"
        th    = THEORY.get(key) or THEORY.get((cfg, ""))
        if th is None:
            continue
        name = f"{cfg} {label}"
        print(f"  {name:<30} {th['rho']:>6.3f}  {th['utilisation']:>6.3f}  "
              f"{th['Lq']:>7.4f}  {th['Wq']:>8.4f}  {th['drop_rate']*100:>7.3f}")

print()
print("  Note: D_alarm = Alarm E2E because fire notification (sendDirect)")
print("  is delivered with zero propagation delay in OMNeT++ DES model.")
print()
print("Done.")
