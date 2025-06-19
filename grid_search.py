#!/usr/bin/env python3
import subprocess
import os
import shutil
from itertools import product
import datetime
from pathlib import Path

# Parameter ranges to test
PARAM_RANGES = {
    'mp': [0.05, 0.1, 0.2, 0.5],          # Mutation probability
    'mi': [0.5, 0.75, 1.0, 1.25],         # Mutation intensity
    'ps': [50, 100, 150, 200],            # Population size
    'mr': [1, 2, 3],                      # Max rounds
    'sp': [0.05, 0.1, 0.2, 0.5],          # Select percentage
    'tr': [0.1, 0.5, 1.0]                 # Tournament ratio
}

def compile_project():
    """Compile the project using CMake"""
    try:
        print("Compiling project...")
        subprocess.run(['cmake', '..'], cwd='build', check=True)
        subprocess.run(['make'], cwd='build', check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")
        return False

def run_experiment(params):
    """Run the RF algorithm with given parameters"""
    cmd = ['./build/main']
    for param, value in params.items():
        cmd.extend([f'--{param}', str(value)])
    
    try:
        print(f"Running with parameters: {params}")
        subprocess.run(cmd, check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Experiment failed: {e}")
        return False

def save_results(params, run_id):
    """Save results to a uniquely named directory"""
    # Create results directory name based on parameters
    param_str = '_'.join(f"{k}{v}" for k, v in params.items())
    results_dir = f"results/run_{run_id}_{param_str}"
    
    # Create directory if it doesn't exist
    os.makedirs(results_dir, exist_ok=True)
    
    # Copy results from build/results_RumorFlow to new directory
    source_dir = "build/results_RumorFlow"
    if os.path.exists(source_dir):
        for file in os.listdir(source_dir):
            shutil.copy2(
                os.path.join(source_dir, file),
                os.path.join(results_dir, file)
            )
    
    return results_dir

def analyze_results(results_dir):
    """Run extract.py on the results"""
    try:
        subprocess.run(['python3', 'extract.py', results_dir], check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Analysis failed: {e}")
        return False

def main():
    # Create timestamp for this run
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Create main results directory
    base_results_dir = "grid_search_results"
    os.makedirs(base_results_dir, exist_ok=True)
    
    # Generate all parameter combinations
    param_names = PARAM_RANGES.keys()
    param_values = PARAM_RANGES.values()
    combinations = list(product(*param_values))
    
    print(f"Starting grid search with {len(combinations)} combinations")
    
    # Compile project first
    if not compile_project():
        return
    
    # Run experiments for each parameter combination
    for i, values in enumerate(combinations, 1):
        params = dict(zip(param_names, values))
        print(f"\nRunning combination {i}/{len(combinations)}")
        
        if run_experiment(params):
            results_dir = save_results(params, f"{timestamp}_{i}")
            if analyze_results(results_dir):
                # Save parameters to a metadata file
                with open(os.path.join(results_dir, 'params.txt'), 'w') as f:
                    for param, value in params.items():
                        f.write(f"{param}: {value}\n")
            
            print(f"Results saved to {results_dir}")
        
        print("-" * 80)

if __name__ == "__main__":
    main()
