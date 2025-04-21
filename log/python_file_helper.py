import os
import re
import csv

def extract_delays(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()
        # Accept negative delays too (e.g. Delay: -123.456 ms)
        delays = re.findall(r"Delay:\s*(-?\d+\.\d+)\s*ms", content)
        return [float(delay) for delay in delays]

def extract_timestamp(file_name):
    # Extract the time from a filename like "log_10:56:33.txt"
    match = re.search(r"log_(\d{2}:\d{2}:\d{2})\.txt", file_name)
    return match.group(1) if match else ""

def main(directory):
    all_delays = []      # List to hold delays from all files
    timestamps = []      # Timestamps extracted from file names
    max_length = 0       # Track the maximum number of delays in a file

    # Read all log files in the directory
    for file_name in sorted(os.listdir(directory)):
        if file_name.endswith(".txt"):
            file_path = os.path.join(directory, file_name)
            delays = extract_delays(file_path)
            all_delays.append(delays)
            timestamps.append(extract_timestamp(file_name))
            max_length = max(max_length, len(delays))

    # Prepare header rows
    header = [f"File {i + 1}" for i in range(len(all_delays))]

    # Write to CSV
    with open('delays_adacel.csv', 'w', newline='', encoding='utf-8') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow(header)     # First row: File 1, File 2, ...
        csv_writer.writerow(timestamps) # Second row: timestamps

        # Write delays row by row
        for i in range(max_length):
            row = []
            for delays in all_delays:
                if i < len(delays):
                    row.append(f"{delays[i]:.5f}")
                else:
                    row.append("")  # Empty cell
            csv_writer.writerow(row)

if __name__ == "__main__":
    # Get the directory of the current script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Define the relative path to the log directory
    relative_log_path = os.path.join("2025-04-21")

    # Construct the absolute path
    log_directory = os.path.join(script_dir, relative_log_path)
    print(log_directory)
    # Run the main function
    main(log_directory)
