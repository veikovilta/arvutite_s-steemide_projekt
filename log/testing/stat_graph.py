import os
import re
import csv

def extract_average_delay(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        match = re.search(r"Average Delay: (\d+\.\d+)", content)
        if match:
            return float(match.group(1)) * 1000  # Convert to milliseconds
    return None

def main(directory):
    average_delays = []

    # Read all log files in the directory
    for file_name in os.listdir(directory):
        if file_name.endswith(".txt"):  # Assuming log files have .log extension
            file_path = os.path.join(directory, file_name)
            avg_delay = extract_average_delay(file_path)
            if avg_delay is not None:
                average_delays.append(avg_delay)

    # Write the collected data to data.csv
    with open("data2.csv", "w", newline="") as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow(["Number", "Delay Time (ms)"])  # Write header row
        for index, delay in enumerate(average_delays, start=1):
            csv_writer.writerow([index, delay])

# Change this to your directory containing log files
log_directory = "/home/reval/Documents/reval/arvutite_s-steemide_projekt/log/local/test12_04"
main(log_directory)

