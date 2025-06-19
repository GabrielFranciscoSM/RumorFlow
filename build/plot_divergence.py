
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import glob
import os
import numpy as np

# Set the style for better-looking plots
plt.style.use('default')
sns.set_theme(style="whitegrid")
plt.rcParams['figure.figsize'] = [10, 6]

# Function categories
function_categories = {
    'Unimodal': list(range(1, 4)),
    'Simple Multimodal': list(range(4, 9)),
    'Hybrid': list(range(9, 14)),
    'Composite': list(range(14, 31))
}

# Create directories for plots
os.makedirs('results_RumorFlow/plots', exist_ok=True)
os.makedirs('results_RumorFlow/plots/individual', exist_ok=True)
os.makedirs('results_RumorFlow/plots/categories', exist_ok=True)

# Find all divergence data files
data_files = glob.glob('results_RumorFlow/divergence_F*.csv')

# Dictionary to store data by function category
category_data = {cat: [] for cat in function_categories.keys()}

# Process each function's data
for file_path in data_files:
    func_num = int(os.path.basename(file_path).split('F')[1].split('.')[0])
    
    # Read the data
    df = pd.read_csv(file_path)
    
    # Calculate statistics for each evaluation point
    stats = df.groupby('Evaluation').agg({
        'Divergence': ['mean', 'std']
    }).reset_index()
    stats.columns = ['Evaluation', 'Mean', 'Std']
    
    # Create individual function plot
    plt.figure(figsize=(10, 6))
    plt.fill_between(stats['Evaluation'], 
                    stats['Mean'] - stats['Std'],
                    stats['Mean'] + stats['Std'],
                    alpha=0.3, label='±1 std')
    plt.plot(stats['Evaluation'], stats['Mean'], 
             label='Mean', linewidth=2)
    
    plt.title(f'Population Divergence for Function {func_num}')
    plt.xlabel('Evaluations')
    plt.ylabel('Divergence')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # Save individual plot
    plt.savefig(f'results_RumorFlow/plots/individual/F{func_num}_divergence.png', 
                dpi=300, bbox_inches='tight')
    plt.close()
    
    # Store data for category plots
    for category, func_range in function_categories.items():
        if func_num in func_range:
            category_data[category].append({
                'function': func_num,
                'data': stats
            })

# Create category plots
for category, functions_data in category_data.items():
    if functions_data:  # Only create plot if we have data for this category
        plt.figure(figsize=(12, 7))
        
        for func_data in functions_data:
            plt.plot(func_data['data']['Evaluation'], 
                    func_data['data']['Mean'],
                    label=f'F{func_data["function"]}',
                    alpha=0.7)
        
        plt.title(f'Population Divergence - {category} Functions')
        plt.xlabel('Evaluations')
        plt.ylabel('Divergence')
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(f'results_RumorFlow/plots/categories/{category.lower().replace(" ", "_")}_divergence.png',
                    dpi=300, bbox_inches='tight')
        plt.close()

# Create summary statistics
with open('results_RumorFlow/divergence_summary.txt', 'w') as f:
    f.write("RumorFlow Divergence Analysis Summary\n")
    f.write("===================================\n\n")
    
    for category, functions_data in category_data.items():
        if functions_data:
            f.write(f"\n{category} Functions\n")
            f.write("-" * len(f"{category} Functions") + "\n")
            
            for func_data in functions_data:
                func_num = func_data["function"]
                stats = func_data["data"]
                
                f.write(f"\nFunction {func_num}:\n")
                f.write(f"  Initial Divergence: {stats['Mean'].iloc[0]:.2f} (±{stats['Std'].iloc[0]:.2f})\n")
                f.write(f"  Final Divergence: {stats['Mean'].iloc[-1]:.2f} (±{stats['Std'].iloc[-1]:.2f})\n")
                f.write(f"  Max Divergence: {stats['Mean'].max():.2f}\n")
                f.write(f"  Min Divergence: {stats['Mean'].min():.2f}\n")
                f.write(f"  Convergence Rate: {((stats['Mean'].iloc[0] - stats['Mean'].iloc[-1]) / stats['Mean'].iloc[0]):.2%}\n")

print("Analysis complete! Results have been saved to:")
print("- Individual function plots: results_RumorFlow/plots/individual/")
print("- Category plots: results_RumorFlow/plots/categories/")
print("- Summary statistics: results_RumorFlow/divergence_summary.txt")