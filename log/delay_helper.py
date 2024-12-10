import os
import re

def extract_delays(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        # Find all "Delay: <number> ms" patterns and extract the numbers
        delays = re.findall(r"Delay: (\d+\.\d+) ms", content)
        return [float(delay) for delay in delays]  # Convert to floats

def main(directory):
    all_delays = []  # List to hold delays from all files
    max_length = 0   # Track the maximum number of delays in a file

    # Read all log files in the directory
    for file_name in os.listdir(directory):
        if file_name.endswith(".txt"):  # Assuming log files have a .log extension
            file_path = os.path.join(directory, file_name)
            delays = extract_delays(file_path)
            all_delays.append(delays)
            max_length = max(max_length, len(delays))

    # Prepare the formatted data for easy copying into Excel
    formatted_data = []

    # Add header
    header = [f"File {i + 1}" for i in range(len(all_delays))]
    formatted_data.append(" ".join(header))

    # Add delays row by row
    for i in range(max_length):
        row = []
        for delays in all_delays:
            if i < len(delays):
                row.append(f"{delays[i]:.5f}")
            else:
                row.append("")  # Empty cell if no more delays in this file
        formatted_data.append(" ".join(row))

    # Write the formatted data to a text file
    with open('tabel2.txt', 'w') as txtfile:
        txtfile.write("\n".join(formatted_data))

# Change this to your directory containing log files
log_directory = "/home/reval/Documents/reval/arvutite_s-steemide_projekt/log/revalTesting"
main(log_directory)
