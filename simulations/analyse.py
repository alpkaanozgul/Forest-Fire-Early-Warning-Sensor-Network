"""
analyse.py  --  ForestFireSim result extractor
Reads all .sca files and prints a summary table for the report.
Run with:  python3 simulations/analyse.py
"""

import os, re, math
from collections import defaultdict

RESULTS_DIR = os.path.join(os.path.dirname(__file__), "results")

# ---------- parser ----------
def parse_sca(path):
    """Return (config, itervar, scalars_dict) from a .sca file."""
    config = itervar = ""
    scalars = defaultdict(float)
    counts  = defaultdict(int)

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
                        scalars[(module, name)] += float(value)
                        counts[(module, name)]  += 1
                    except ValueError:
                        pass
    return config, itervar, scalars, counts


def get(scalars, pattern):
    """Sum all scalar values whose (module,name) matches regex pattern."""
    rx = re.compile(pattern)
    return sum(v for (m, n), v in scalars.items() if rx.search(f"{m} {n}"))


# ---------- collect runs ----------
runs = []   # list of (config, itervar, scalars)
for fname in os.listdir(RESULTS_DIR):
    if not fname.endswith(".sca"):
        continue
    config, itervar, scalars, _ = parse_sca(os.path.join(RESULTS_DIR, fname))
    runs.append((config, itervar, scalars))

# ---------- aggregate per (config, itervar) ----------
groups = defaultdict(list)
for config, itervar, scalars in runs:
    groups[(config, itervar)].append(scalars)


def ci95(values):
    n = len(values)
    if n < 2:
        return 0.0
    mean = sum(values) / n
    std  = math.sqrt(sum((x - mean)**2 for x in values) / (n - 1))
    # t=2.045 for df=29 (30 reps), 95% CI
    return 2.045 * std / math.sqrt(n)


def summarise(scalars_list):
    """Extract KPIs from a list of scalar dicts (one per replication)."""

    def col(key_pattern):
        vals = [get(s, key_pattern) for s in scalars_list]
        return vals

    d_alarm       = col(r"server dAlarm:mean$")
    e2e           = col(r"server e2eDelay:mean$")
    utilisation   = col(r"gatewayQueue\[\d+\] utilisation:timeavg$")
    queue_len     = col(r"gatewayQueue\[\d+\] queueLength:timeavg$")
    wait_time     = col(r"gatewayQueue\[\d+\] waitingTime:mean$")
    drop_rate     = col(r"gatewayQueue\[\d+\] dropRate$")
    alarms_rx     = col(r"server alarmsReceived:sum$")
    pkts_rx       = col(r"server packetsReceived:sum$")
    alarms_sent   = col(r"node\[\d+\] alarmsSent$")
    fire_events   = col(r"fireGen totalFireEvents$")

    # PDR = alarms received / alarms sent
    pdr = [rx / max(tx, 1) for rx, tx in zip(alarms_rx, alarms_sent)]

    n = len(scalars_list)

    def fmt(vals):
        m = sum(vals) / n
        c = ci95(vals)
        return f"{m:.4f} ± {c:.4f}"

    return {
        "N_reps":        n,
        "D_alarm (s)":   fmt(d_alarm),
        "E2E delay (s)": fmt(e2e),
        "Utilisation":   fmt(utilisation),
        "Lq (mean)":     fmt(queue_len),
        "Wq (s)":        fmt(wait_time),
        "Drop rate":     fmt(drop_rate),
        "PDR":           fmt(pdr),
        "Alarms RX":     fmt(alarms_rx),
        "Fire events":   fmt(fire_events),
    }


# ---------- print ----------
CONFIGS = ["Baseline", "DenseNodes", "HighFireRate", "MultiGateway",
           "FixedInterval", "AdaptiveInterval"]

for cfg in CONFIGS:
    keys = sorted([k for k in groups if k[0] == cfg],
                  key=lambda k: k[1])
    if not keys:
        continue

    print(f"\n{'='*70}")
    print(f"  CONFIG: {cfg}")
    print(f"{'='*70}")

    for key in keys:
        label = key[1] if key[1] else "(baseline)"
        slist = groups[key]
        s = summarise(slist)
        print(f"\n  {label}  ({s['N_reps']} reps)")
        print(f"    D_alarm      : {s['D_alarm (s)']}")
        print(f"    E2E delay    : {s['E2E delay (s)']}")
        print(f"    Utilisation  : {s['Utilisation']}")
        print(f"    Queue len Lq : {s['Lq (mean)']}")
        print(f"    Wait time Wq : {s['Wq (s)']}")
        print(f"    Drop rate    : {s['Drop rate']}")
        print(f"    PDR          : {s['PDR']}")
        print(f"    Alarms RX    : {s['Alarms RX']}")
        print(f"    Fire events  : {s['Fire events']}")

print("\nDone.")
