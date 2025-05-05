use std::fs::File;
use std::io::{BufRead, BufReader};
use std::time::Instant;
use std::process;
use std::str::FromStr;

fn process_file(filename: &str) -> (Vec<f64>, usize) {
    let file = File::open(filename).unwrap_or_else(|_| {
        eprintln!("Error opening: {}", filename);
        process::exit(1);
    });
    let reader = BufReader::new(file);
    const MIN_VAL: f64 = 1.0;
    const MAX_VAL: f64 = 99.0;
    const RANGE_VAL: f64 = MAX_VAL - MIN_VAL;
    let mut anomalies = 0;
    let mut transformed_data = Vec::new();
    for line in reader.lines().skip(1) {
        let line = match line {
            Ok(l) => l,
            Err(_) => continue,
        };
        let mut parts = line.splitn(3, ',');
        parts.next();
        parts.next();
        if let Some(value) = parts.next() {
            let value = value.trim();
            if value == "NA" {
                continue;
            }
            if let Ok(original) = f64::from_str(value) {
                if original >= MIN_VAL && original <= MAX_VAL {
                    let transformed = (original - MIN_VAL) / RANGE_VAL;
                    transformed_data.push(transformed);
                    if transformed > 0.9 {
                        anomalies += 1;
                    }
                }
            }
        }
    }
    transformed_data.shrink_to_fit();
    (transformed_data, anomalies)
}

fn calculations(data: &[f64]) -> (f64, f64, f64, String) {
    let n = data.len();
    let (sum_x, sum_x2) = data.iter().fold((0.0, 0.0), |(s1, s2), &x| (s1 + x, s2 + x * x));
    let mean = sum_x / n as f64;
    let variance = (sum_x2 / n as f64) - mean.powi(2);
    let std_dev = variance.sqrt();
    let window_size = 100;
    let mut prefix = vec![0.0; n + 1];
    data.iter().enumerate().for_each(|(i, &val)| {
        prefix[i + 1] = prefix[i] + val;
    });
    let (mut inc, mut dec) = (0, 0);
    let mut prev_mean = None;
    for i in 0..=n.saturating_sub(window_size) {
        let current_mean = (prefix[i + window_size] - prefix[i]) / window_size as f64;
        if let Some(pm) = prev_mean {
            if current_mean > pm {
                inc += 1;
            } else if current_mean < pm {
                dec += 1;
            }
        }
        prev_mean = Some(current_mean);
    }
    let trend = match inc.cmp(&dec) {
        std::cmp::Ordering::Greater => "increasing",
        std::cmp::Ordering::Less => "decreasing",
        _ => "stable",
    };
    (mean, variance, std_dev, trend.to_string())
}

fn main() {
    let filenames = [
        "small_sensor_data_2024.csv",
        "medium_sensor_data_2024.csv",
        "large_sensor_data_2024.csv",
    ];
    for file in &filenames {
        // Process file and time execution
        let process_start = Instant::now();
        let (data, anomalies) = process_file(file);
        let process_time = process_start.elapsed();

        // Calculate statistics and time execution
        let calc_start = Instant::now();
        let (mean, var, std, trend) = calculations(&data);
        let calc_time = calc_start.elapsed();

        // Calculate timing metrics
        let total_time = process_time.as_secs_f64() + calc_time.as_secs_f64();

        // Output test results
        println!("--- Results for {} ---", file);
        println!("Mean: {:.5}", mean);
        println!("Variance: {:.5}", var);
        println!("Standard Deviation: {:.5}", std);
        println!("Trend: {}", trend);
        println!("Anomalies detected: {}", anomalies);
        println!("Processing time: {:.5}s", process_time.as_secs_f64());
        println!("Calculation time: {:.5}s", calc_time.as_secs_f64());
        println!("Total time: {:.5}s\n", total_time);
    }
}