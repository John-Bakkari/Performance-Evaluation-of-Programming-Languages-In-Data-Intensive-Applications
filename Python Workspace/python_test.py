import time
import sys

def process_file(filename):
    min_val, max_val = 1.0, 99.0
    range_val = max_val - min_val
    anomalies = 0
    transformed_data = []
    try:
        with open(filename, 'r') as file:
            next(file)
            for line in file:
                try:
                    value = line.strip().split(',', 3)[2]
                    if value != 'NA':
                        original = float(value)
                        if min_val <= original <= max_val:
                            transformed = (original - min_val) / range_val
                            transformed_data.append(transformed)
                            if transformed > 0.9:
                                anomalies += 1
                except (IndexError, ValueError):
                    continue    
    except OSError as e:
        print(f"Error opening: {filename}")
        sys.exit(1)
    return transformed_data, anomalies

def calculations(data, window_size = 100):
    n = len(data)
    sum_x = 0.0
    sum_x2 = 0.0
    for x in data:
        sum_x += x
        sum_x2 += x * x
    mean = sum_x / n
    variance = (sum_x2 / n) - (mean * mean)
    std_dev = variance ** 0.5
    increasing = decreasing = 0
    prev_mean = None
    prefix = [0.0] * (n + 1)
    for i in range(n):
        prefix[i+1] = prefix[i] + data[i]
    for i in range(n - window_size + 1):
        current_mean = (prefix[i+window_size] - prefix[i]) / window_size
        if prev_mean is not None:
            if current_mean > prev_mean:
                increasing += 1
            elif current_mean < prev_mean:
                decreasing += 1
        prev_mean = current_mean
    trend = 'stable'
    if increasing > decreasing:
        trend = 'increasing'
    elif decreasing > increasing:
        trend = 'decreasing'
    return mean, variance, std_dev, trend

def main():
    files = [
        "small_sensor_data_2024.csv",
        "medium_sensor_data_2024.csv",
        "large_sensor_data_2024.csv"
    ]
    for filename in files:
        # Process file and time execution
        process_start = time.perf_counter()
        data, anomalies = process_file(filename)
        process_end = time.perf_counter()

        # Calculate statistics and time execution
        calc_start = time.perf_counter()
        mean, variance, std_dev, trend = calculations(data)
        calc_end = time.perf_counter()

        # Calculate timing metrics
        process_time = process_end - process_start
        calc_time = calc_end - calc_start
        total_time = process_time + calc_time

        # Output test results
        print(f"--- Results for {filename} ---")
        print(f"Mean: {mean:.5f}")
        print(f"Variance: {variance:.5f}")
        print(f"Standard Deviation: {std_dev:.5f}")
        print(f"Trend: {trend}")
        print(f"Anomalies detected: {anomalies}")
        print(f"Processing time: {process_time:.5f} seconds")
        print(f"Calculation time: {calc_time:.5f} seconds")
        print(f"Total time: {total_time:.5f} seconds\n")

if __name__ == '__main__':
    main()