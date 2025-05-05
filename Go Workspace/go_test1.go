package main

import (
	"bufio"
	"bytes"
	"fmt"
	"math"
	"os"
	"strconv"
	"time"
)

func processFile(filename string) ([]float64, int) {
	const (
		minVal   = 1.0
		maxVal   = 99.0
		rangeVal = maxVal - minVal
	)
	var anomalies int
	var transformedData []float64
	file, err := os.Open(filename)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening: %s\n", filename)
		os.Exit(1)
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	scanner.Buffer(make([]byte, 4*1024*1024), 4*1024*1024)
	scanner.Scan()
	for scanner.Scan() {
		line := scanner.Bytes()
		parts := bytes.SplitN(line, []byte{','}, 4)
		if len(parts) < 3 {
			continue
		}
		valueStr := string(bytes.TrimSpace(parts[2]))
		if valueStr == "NA" {
			continue
		}
		value, err := strconv.ParseFloat(valueStr, 64)
		if err != nil {
			continue
		}
		if value >= minVal && value <= maxVal {
			transformed := (value - minVal) / rangeVal
			transformedData = append(transformedData, transformed)
			if transformed > 0.9 {
				anomalies++
			}
		}
	}
	return transformedData, anomalies
}

func calculations(data []float64, windowSize int) (float64, float64, float64, string) {
	n := len(data)
	var sumX, sumX2 float64
	for _, x := range data {
		sumX += x
		sumX2 += x * x
	}
	mean := sumX / float64(n)
	variance := sumX2/float64(n) - mean*mean
	stdDev := math.Sqrt(variance)
	prefix := make([]float64, n+1)
	for i := 0; i < n; i++ {
		prefix[i+1] = prefix[i] + data[i]
	}
	increasing, decreasing := 0, 0
	prevMean := prefix[windowSize] / float64(windowSize)
	for i := 1; i <= n-windowSize; i++ {
		currentMean := (prefix[i+windowSize] - prefix[i]) / float64(windowSize)
		if currentMean > prevMean {
			increasing++
		} else if currentMean < prevMean {
			decreasing++
		}
		prevMean = currentMean
	}
	trend := "stable"
	switch {
	case increasing > decreasing:
		trend = "increasing"
	case decreasing > increasing:
		trend = "decreasing"
	}
	return mean, variance, stdDev, trend
}

func main() {
	files := []string{
		"small_sensor_data_2024.csv",
		"medium_sensor_data_2024.csv",
		"large_sensor_data_2024.csv",
	}

	for _, filename := range files {
		// Process file and time execution
		processStart := time.Now()
		data, anomalies := processFile(filename)
		processTime := time.Since(processStart)

		// Calculate statistics and time execution
		calcStart := time.Now()
		mean, variance, stdDev, trend := calculations(data, 100)
		calcTime := time.Since(calcStart)

		// Calculate timing metrics
		totalTime := processTime + calcTime

		// Output test results
		fmt.Printf("--- Results for %s ---\n", filename)
		fmt.Printf("Mean: %.5f\n", mean)
		fmt.Printf("Variance: %.5f\n", variance)
		fmt.Printf("Standard Deviation: %.5f\n", stdDev)
		fmt.Printf("Trend: %s\n", trend)
		fmt.Printf("Anomalies detected: %d\n", anomalies)
		fmt.Printf("Processing time: %.5f seconds\n", processTime.Seconds())
		fmt.Printf("Calculation time: %.5f seconds\n", calcTime.Seconds())
		fmt.Printf("Total time: %.5f seconds\n\n", totalTime.Seconds())
	}
}
