import os
import sys
import matplotlib.pyplot as plt
import numpy as np

# We can import analyse.py directly since it is in the same folder
import analyse

output_dir = os.path.join(os.path.dirname(__file__), "results")
os.makedirs(output_dir, exist_ok=True)

def plot_dense_nodes():
    # DenseNodes: N=4, 8, 12, 16, 20
    N_vals = [4, 8, 12, 16, 20]
    
    delays = []
    delays_err = []
    utils = []
    
    for n in N_vals:
        key = ("DenseNodes", f"$N={n}")
        if key not in analyse.groups:
            # Maybe it is just baseline for N=8
            key = ("Baseline", "")
            
        if key in analyse.groups:
            kpis = analyse.extract_kpis(analyse.groups[key])
            vals_d = kpis["Alarm E2E (s)"]
            delays.append(analyse.mean(vals_d))
            delays_err.append(analyse.ci95(vals_d))
            
            vals_u = kpis["Utilisation"]
            utils.append(analyse.mean(vals_u))

    # Plot 1: Alarm E2E Delay
    plt.figure(figsize=(7, 5))
    plt.errorbar(N_vals, delays, yerr=delays_err, fmt='-o', capsize=5, color='darkred', lw=2)
    plt.title('Effect of Node Density on Alarm End-to-End Delay')
    plt.xlabel('Number of Sensor Nodes (N)')
    plt.ylabel('Alarm E2E Delay (s)')
    plt.xticks(N_vals)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'out_density_delay.png'), dpi=300)
    plt.close()

def plot_multi_gateway():
    # MultiGateway: G=1, 2, 3
    G_vals = [1, 2, 3]
    
    utils = []
    utils_th = []
    
    for g in G_vals:
        key = ("MultiGateway", f"$G={g}")
        if key not in analyse.groups:
            key = ("Baseline", "") # Baseline is G=2
            
        if key in analyse.groups:
            kpis = analyse.extract_kpis(analyse.groups[key])
            vals_u = kpis["Utilisation"]
            utils.append(analyse.mean(vals_u))
            
            th = analyse.THEORY.get(key)
            if th:
                utils_th.append(th["utilisation"])
            else:
                utils_th.append(0)

    # Plot 2: Gateway Utilisation
    x = np.arange(len(G_vals))
    width = 0.35
    
    fig, ax = plt.subplots(figsize=(7, 5))
    rects1 = ax.bar(x - width/2, utils, width, label='Simulated', color='steelblue')
    rects2 = ax.bar(x + width/2, utils_th, width, label='Theoretical M/M/1/K', color='lightgray', edgecolor='black', hatch='//')
    
    ax.set_ylabel('Gateway Utilisation (Fraction)')
    ax.set_title('Gateway Utilisation vs Number of Gateways (N=8)')
    ax.set_xticks(x)
    ax.set_xticklabels([f"G={g}" for g in G_vals])
    ax.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'out_gateways_util.png'), dpi=300)
    plt.close()

def plot_fire_rate():
    # HighFireRate: 0.001, 0.005, 0.01, 0.02
    FR_vals = [0.001, 0.005, 0.01, 0.02]
    
    pdr = []
    pdr_err = []
    
    for fr in FR_vals:
        key = ("HighFireRate", f"$FR={fr}")
        if key not in analyse.groups:
            if fr == 0.005:
                key = ("Baseline", "")
            
        if key in analyse.groups:
            kpis = analyse.extract_kpis(analyse.groups[key])
            vals = kpis["PDR (%)"]
            pdr.append(analyse.mean(vals))
            pdr_err.append(analyse.ci95(vals))

    # Plot 3: Packet Delivery Ratio
    plt.figure(figsize=(7, 5))
    plt.errorbar(FR_vals, pdr, yerr=pdr_err, fmt='-s', capsize=5, color='darkgreen', lw=2)
    plt.title('Effect of Fire Event Rate on Packet Delivery Ratio')
    plt.xlabel(r'Fire Event Rate $\lambda_{fire}$ (events/s)')
    plt.ylabel('Packet Delivery Ratio (%)')
    plt.xticks(FR_vals)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'out_firerate_pdr.png'), dpi=300)
    plt.close()

if __name__ == "__main__":
    plot_dense_nodes()
    plot_multi_gateway()
    plot_fire_rate()
    print("Output plots generated successfully!")
