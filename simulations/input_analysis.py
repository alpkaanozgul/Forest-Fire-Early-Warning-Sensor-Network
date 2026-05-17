import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats
import os

# Create results directory if it doesn't exist
output_dir = os.path.join(os.path.dirname(__file__), "results")
os.makedirs(output_dir, exist_ok=True)

def u01(n):
    # Ensure no exact 0s as in RngUtils
    u = np.random.uniform(0, 1, n)
    u[u == 0] = 1e-12
    return u

def exponentialRV(lam, n):
    """ Inverse Transform for Exponential """
    return -1.0 / lam * np.log(u01(n))

def normalRV(mu, sigma, n):
    """ Box-Muller Transform for Normal """
    U1 = u01(n)
    U2 = u01(n)
    Z0 = np.sqrt(-2.0 * np.log(U1)) * np.cos(2.0 * np.pi * U2)
    return mu + sigma * Z0

def generate_plots():
    N = 10000
    
    # 1. Telemetry Interval (Exponential, mean=30 -> lam=1/30)
    lam_telem = 1.0 / 30.0
    telem_samples = exponentialRV(lam_telem, N)
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    axes[0].hist(telem_samples, bins=50, density=True, alpha=0.6, color='blue', label='Empirical (Inverse Transform)')
    x = np.linspace(0, np.max(telem_samples), 100)
    axes[0].plot(x, stats.expon.pdf(x, scale=1.0/lam_telem), 'r-', lw=2, label='Theoretical PDF')
    axes[0].set_title('Histogram: Telemetry Interval (Exp, $\mu=30$s)')
    axes[0].set_xlabel('Time (s)')
    axes[0].set_ylabel('Density')
    axes[0].legend()
    
    # QQ-plot
    stats.probplot(telem_samples, dist="expon", sparams=(0, 1.0/lam_telem), plot=axes[1])
    axes[1].set_title('QQ-Plot: Telemetry Interval')
    axes[1].get_lines()[1].set_color('red') # Theoretical line color
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'input_analysis_telem.png'), dpi=300)
    plt.close()

    # 2. Temperature Reading (Normal, mu=25, sigma=3)
    mu_temp = 25.0
    sigma_temp = 3.0
    temp_samples = normalRV(mu_temp, sigma_temp, N)
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    axes[0].hist(temp_samples, bins=50, density=True, alpha=0.6, color='green', label='Empirical (Box-Muller)')
    x = np.linspace(np.min(temp_samples), np.max(temp_samples), 100)
    axes[0].plot(x, stats.norm.pdf(x, mu_temp, sigma_temp), 'r-', lw=2, label='Theoretical PDF')
    axes[0].set_title('Histogram: Temperature Reading (Normal, $\mu=25^\circ C$, $\sigma=3^\circ C$)')
    axes[0].set_xlabel('Temperature ($^\circ C$)')
    axes[0].set_ylabel('Density')
    axes[0].legend()
    
    stats.probplot(temp_samples, dist="norm", sparams=(mu_temp, sigma_temp), plot=axes[1])
    axes[1].set_title('QQ-Plot: Temperature Reading')
    axes[1].get_lines()[1].set_color('red')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'input_analysis_temp.png'), dpi=300)
    plt.close()

    # 3. Gateway Service Time (Exponential, mean=5s -> lam=0.2)
    lam_service = 0.2
    service_samples = exponentialRV(lam_service, N)
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    axes[0].hist(service_samples, bins=50, density=True, alpha=0.6, color='orange', label='Empirical (Inverse Transform)')
    x = np.linspace(0, np.max(service_samples), 100)
    axes[0].plot(x, stats.expon.pdf(x, scale=1.0/lam_service), 'r-', lw=2, label='Theoretical PDF')
    axes[0].set_title('Histogram: Gateway Service Time (Exp, $\mu=5$s)')
    axes[0].set_xlabel('Time (s)')
    axes[0].set_ylabel('Density')
    axes[0].legend()
    
    stats.probplot(service_samples, dist="expon", sparams=(0, 1.0/lam_service), plot=axes[1])
    axes[1].set_title('QQ-Plot: Gateway Service Time')
    axes[1].get_lines()[1].set_color('red')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'input_analysis_service.png'), dpi=300)
    plt.close()

if __name__ == "__main__":
    generate_plots()
    print("Plots generated successfully in simulations/results/")
