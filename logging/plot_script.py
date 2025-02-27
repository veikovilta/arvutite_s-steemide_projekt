import argparse
import matplotlib.pyplot as plt
import re
import numpy as np  # Import numpy for element-wise operations

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Plot voltage readings over time with optional value division.")
parser.add_argument('filename', type=str, help="The input filename containing voltage data.")
parser.add_argument('divider', type=float, help="The value by which all voltages will be divided.")
args = parser.parse_args()

# Read the file
with open(args.filename, "r") as file:
    lines = file.readlines()

# Extract voltage values and remove negative values
voltages = []
for line in lines:
    match = re.search(r"([-+]?[0-9]*\.?[0-9]+) V", line)
    if match:
        value = float(match.group(1))
        if value >= 0:  # Remove negative values
            voltages.append(value)

# Divide all voltage values by the divider
voltages = np.array([v / args.divider for v in voltages])  # Convert to numpy array

# Find the first index where voltage is >= 2V
start_index = next((i for i, v in enumerate(voltages) if v >= 2), 0)

# Slice data from the first occurrence of 2V
voltages = voltages[start_index:]
time_steps = range(1, len(voltages) + 1)

# Plot the data with improved readability
plt.figure(figsize=(14, 7))

# Fill areas above 1.8V with a color (light green)
plt.fill_between(time_steps, voltages, 1.8, where=(voltages >= 1.8), color='green', alpha=0.3, label='Above 1.8V')

# Fill areas below 1.8V with a color (light red)
plt.fill_between(time_steps, voltages, 1.8, where=(voltages < 1.8), color='red', alpha=0.3, label='Below 1.8V')

# Plot the voltage readings line
plt.plot(time_steps, voltages, linestyle='-', color='b', linewidth=2, label='Voltage (V)')

# Add a bold bright line at 1.8V
plt.axhline(y=1.8, color='r', linewidth=3, linestyle='-', label='1.8V Threshold')

# Set y-axis limit to max 3.3V
plt.ylim(0, 3.3)

plt.xlabel("Time Step", fontsize=14, fontweight='bold')
plt.ylabel("Voltage (V)", fontsize=14, fontweight='bold')
plt.title("Voltage Readings Over Time", fontsize=16, fontweight='bold')
plt.legend(fontsize=12)
plt.grid(True, linestyle='--', alpha=0.6)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.tight_layout()
plt.show()
