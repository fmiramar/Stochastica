import numpy as np
import matplotlib.pyplot as plt

def rulkov_map(alpha=4.1, mu=0.001, sigma=-1.2, x0=0.0, y0=-2.9, steps=80000):
    x = np.zeros(steps)
    y = np.zeros(steps)
    x[0] = x0
    y[0] = y0
    for i in range(1, steps):
        x[i] = (alpha / (1.0 + x[i-1]**2)) + y[i-1]
        y[i] = y[i-1] - mu * (x[i-1] - sigma)
    return x, y

x, y = rulkov_map(steps=150000)

# Create a figure with a banner aspect ratio (e.g. 16:6)
fig, ax = plt.subplots(figsize=(16, 6), facecolor='#ffffff')
ax.set_facecolor('#f8f9fa')

# Make the dots more visible
ax.scatter(x, y, s=1.5, c='#0366d6', alpha=0.6, edgecolors='none')

# Grid more visible
ax.grid(True, color='#d1d5da', linestyle='-', linewidth=1.0)
ax.set_axisbelow(True)

# Remove spines but keep grid and ticks slightly visible or remove ticks
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.spines['left'].set_color('#d1d5da')
ax.spines['bottom'].set_color('#d1d5da')
ax.tick_params(colors='#586069')

plt.tight_layout()
plt.savefig('../docs/assets/rulkov_banner.png', dpi=150, bbox_inches='tight')
print("Banner saved to docs/assets/rulkov_banner.png")
