function process_file(filename::String)
    min_val, max_val = 1.0, 99.0
    range_val = max_val - min_val
    anomalies = 0
    transformed_data = Vector{Float64}()
    try
        open(filename, "r") do file
            readline(file)
            for line in eachline(file)
                fields = split(line, ',', limit=3)  
                length(fields) < 3 && continue
                value_str = strip(fields[3])
                value_str == "NA" && continue
                original = tryparse(Float64, value_str)
                isnothing(original) && continue
                if min_val ≤ original ≤ max_val
                    transformed = (original - min_val) / range_val
                    push!(transformed_data, transformed)
                    transformed > 0.9 && (anomalies += 1)
                end
            end
        end
    catch e
        println("Error opening: $filename")
        exit(1)
    end
    return transformed_data, anomalies
end

function calculations(data::Vector{Float64}, window_size::Int=100)
    n = length(data)
    sum_x = sum_x2 = 0.0
    @inbounds @simd for x in data
        sum_x += x
        sum_x2 += x * x
    end
    mean = sum_x / n
    variance = (sum_x2 - sum_x^2 / n) / n
    std_dev = sqrt(variance)
    prefix = Vector{Float64}(undef, n+1)
    prefix[1] = 0.0
    @inbounds @simd for i in 1:n
        prefix[i+1] = prefix[i] + data[i]
    end
    increasing = decreasing = 0
    prev_mean = prefix[window_size] / window_size
    @inbounds for i in 2:(n - window_size + 1)
        current_mean = (prefix[i+window_size-1] - prefix[i-1]) / window_size
        diff = current_mean - prev_mean
        if diff > 0
            increasing += 1
        elseif diff < 0
            decreasing += 1
        end
        prev_mean = current_mean
    end
    trend = increasing > decreasing ? "increasing" : 
            decreasing > increasing ? "decreasing" : "stable"
    return mean, variance, std_dev, trend
end

function main()
    files = [
        "small_sensor_data_2024.csv",
        "medium_sensor_data_2024.csv",
        "large_sensor_data_2024.csv"
    ]
    
    for filename in files
        # Process file and time execution
        process_start = time_ns()
        data, anomalies = process_file(filename)
        process_end = time_ns()
        
        # Calculate statistics and time execution
        calc_start = time_ns()
        mean, variance, std_dev, trend = calculations(data)
        calc_end = time_ns()
        
        # Calculate timing metrics
        process_time = (process_end - process_start) / 1e9
        calc_time = (calc_end - calc_start) / 1e9
        total_time = process_time + calc_time
        
        # Output test results
        println("--- Results for $filename ---")
        println("Mean: $(round(mean, digits=5))")
        println("Variance: $(round(variance, digits=5))")
        println("Standard Deviation: $(round(std_dev, digits=5))")
        println("Trend: $trend")
        println("Anomalies detected: $anomalies")
        println("Processing time: $(round(process_time, digits=5)) seconds")
        println("Calculation time: $(round(calc_time, digits=5)) seconds")
        println("Total time: $(round(total_time, digits=5)) seconds\n")
    end
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end