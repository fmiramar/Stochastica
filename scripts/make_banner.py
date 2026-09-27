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

fig, ax = plt.subplots(figsize=(16, 6), facecolor='#ffffff')
ax.set_facecolor('#f8f9fa')

ax.scatter(x, y, s=1.5, c='#0366d6', alpha=0.6, edgecolors='none')

ax.grid(True, color='#d1d5da', linestyle='-', linewidth=1.0)
ax.set_axisbelow(True)

ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.spines['left'].set_color('#d1d5da')
ax.spines['bottom'].set_color('#d1d5da')
ax.tick_params(colors='#586069')

# Add "Stochastica" text in the center
ax.text(0.5, 0.5, 'Stochastica', transform=ax.transAxes,
        fontsize=90, fontfamily='monospace', fontweight='bold',
        ha='center', va='center', color='#24292e',
        bbox=dict(facecolor='#ffffff', alpha=0.75, edgecolor='none', pad=15, boxstyle='round,pad=0.2'))

plt.tight_layout()
plt.savefig('../docs/assets/rulkov_banner.png', dpi=150, bbox_inches='tight')
