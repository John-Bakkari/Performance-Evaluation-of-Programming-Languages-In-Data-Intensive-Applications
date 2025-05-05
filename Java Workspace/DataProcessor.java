import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class DataProcessor {
    private static class ProcessResult {
        final List<Double> data;
        final int anomalies;
        ProcessResult(List<Double> data, int anomalies) {
            this.data = data;
            this.anomalies = anomalies;
        }
    }

    private static ProcessResult processFile(String filename) {
        final double minVal = 1.0, maxVal = 99.0, rangeVal = maxVal - minVal;
        List<Double> data = new ArrayList<>();
        int anomalies = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            br.readLine();
            String line;
            while ((line = br.readLine()) != null) {
                int comma1 = line.indexOf(',');
                int comma2 = line.indexOf(',', comma1 + 1);
                if (comma2 == -1) continue;
                String val = line.substring(comma2 + 1).split(",", 2)[0].trim();
                if (val.equals("NA")) continue;
                try {
                    double original = Double.parseDouble(val);
                    if (original >= minVal && original <= maxVal) {
                        double transformed = (original - minVal) / rangeVal;
                        data.add(transformed);
                        if (transformed > 0.9) anomalies++;
                    }
                } catch (NumberFormatException ignored) {}
            }
        } catch (IOException e) {
            System.err.println("Error reading " + filename);
            System.exit(1);
        }
        return new ProcessResult(data, anomalies);
    }

    private static Object[] calculateStats(List<Double> data) {
        final int windowSize = 100;
        int n = data.size();
        double sumX = 0, sumX2 = 0;
        for (double x : data) {
            sumX += x;
            sumX2 += x * x;
        }
        double mean = sumX / n;
        double variance = (sumX2 / n) - (mean * mean);
        double stdDev = Math.sqrt(variance);
        double[] prefix = new double[n + 1];
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = prefix[i] + data.get(i);
        }
        int increasing = 0, decreasing = 0;
        double prevMean = prefix[windowSize] / windowSize;
        for (int i = 1; i <= n - windowSize; i++) {
            double currentMean = (prefix[i + windowSize] - prefix[i]) / windowSize;
            if (currentMean > prevMean) increasing++;
            else if (currentMean < prevMean) decreasing++;
            prevMean = currentMean;
        }
        String trend = increasing > decreasing ? "increasing" : 
                      decreasing > increasing ? "decreasing" : "stable";
        return new Object[]{mean, variance, stdDev, trend};
    }

    public static void main(String[] args) {
        String[] files = {
            "small_sensor_data_2024.csv",
            "medium_sensor_data_2024.csv",
            "large_sensor_data_2024.csv"
        };

        for (String filename : files) {
            // Process file and time execution
            long processStart = System.nanoTime();
            ProcessResult result = processFile(filename);
            long processEnd = System.nanoTime();

            // Calculate statistics and time execution
            long calcStart = System.nanoTime();
            Object[] stats = calculateStats(result.data);
            long calcEnd = System.nanoTime();

            // Calculate timing metrics
            double processTime = (processEnd - processStart) / 1e9;
            double calcTime = (calcEnd - calcStart) / 1e9;
            double totalTime = processTime + calcTime;

            // Output test results
            System.out.printf("--- Results for %s ---%n", filename);
            System.out.printf("Mean: %.5f%n", stats[0]);
            System.out.printf("Variance: %.5f%n", stats[1]);
            System.out.printf("Standard Deviation: %.5f%n", stats[2]);
            System.out.printf("Trend: %s%n", stats[3]);
            System.out.printf("Anomalies: %d%n", result.anomalies);
            System.out.printf("Processing: %.5fs%n", processTime);
            System.out.printf("Calculations: %.5fs%n", calcTime);
            System.out.printf("Total: %.5fs%n%n", totalTime);
        }
    }
}