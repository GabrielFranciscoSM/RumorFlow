import csv
import os
import glob
import math

#-------------------------------------------------
# Configuration
#-------------------------------------------------
RESULTS_DIR = '../' # Directory containing CSV files
FILE_PATTERN = 'output*.txt'    # Pattern to match files

# --- !!! --- Filter Criteria (Set these to filter) --- !!! ---
# Set to a specific name (e.g., 'AlgoA_Prob1') to filter by name, or None to disable.
FILTER_BY_NAME = None # Example: 'AlgoA_Prob1'

# Set to a specific run_id (filename without extension, e.g., 'algoA_config1')
# to filter by file, or None to disable.
# NOTE: Only one filter (name or run_id) will be actively used in the example
#       generation below, but you can generate multiple tables.
FILTER_BY_RUN_ID = None # Example: 'algoA_config1'
# --- !!! --- End Filter Criteria --- !!! ---


#-------------------------------------------------
# Helper function for number formatting (MODIFIED from previous version)
#-------------------------------------------------
def format_num(num, precision=2, sci_thresh_low=1e-3, sci_thresh_high=1e4):
    """Formats numbers for the table. Handles None."""
    if num is None:
        return 'N/A' # Or '-' or empty string
    try:
        num = float(num)
    except (ValueError, TypeError):
        return str(num)
    if num == 0:
        return f"{0:.{precision}f}"
    abs_num = abs(num)
    if abs_num > 0 and (abs_num < sci_thresh_low or abs_num >= sci_thresh_high):
        return f"{num:.{precision}e}"
    else:
        return f"{num:.{precision}f}"

#-------------------------------------------------
# Function to read data from a single CSV file (MODIFIED from previous version)
#-------------------------------------------------
def read_data_from_file(filepath):
    """
    Reads structured data from a CSV file, handling potential
    missing values or conversion errors more robustly.
    """
    data = []
    try:
        with open(filepath, 'r', newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            expected_headers = ['name', 'bestFitness', 'meanFit', 'meanTime', 'meanEvals']
            if not reader.fieldnames:
                 # print(f"Warning: File '{filepath}' appears to be empty or has no header. Skipping.")
                 return []
            if not all(h in reader.fieldnames for h in expected_headers):
                 print(f"Warning: File '{filepath}' has missing headers. Expected: {expected_headers}. Found: {reader.fieldnames}. Skipping file.")
                 return []

            line_num = 1
            for row in reader:
                line_num += 1
                row_data = {}
                row_data['name'] = row.get('name', f'Unknown_L{line_num}')

                for key, converter in [('bestFitness', float), ('meanFit', float),
                                       ('meanTime', float), ('meanEvals', float)]:
                    raw_value = row.get(key)
                    if raw_value is None or raw_value.strip() == '':
                        row_data[key] = None
                    else:
                        try:
                            row_data[key] = converter(raw_value)
                        except (ValueError, TypeError):
                            print(f"Warning: Invalid value '{raw_value}' for '{key}' in '{filepath}' line {line_num}. Setting to None.")
                            row_data[key] = None

                if row_data.get('meanEvals') is not None:
                    try:
                        row_data['meanEvals'] = int(row_data['meanEvals'])
                    except (ValueError, TypeError):
                        pass # Keep as float if int conversion fails

                data.append(row_data)
    except FileNotFoundError:
        print(f"Error: File not found: {filepath}")
        return []
    except Exception as e:
        print(f"Error reading file {filepath}: {e}")
        return []
    return data

#-------------------------------------------------
# Function to generate LaTeX table code (Mostly Unchanged)
#-------------------------------------------------
def generate_latex_table(data, caption="Performance Results", label="tab:results",
                         sort_by=None, reverse_sort=False,
                         columns=None, column_formats=None,
                         precision=2, sci_thresh_low=1e-3, sci_thresh_high=1e4,
                         add_resizebox=True): # Added option to disable resizebox
    """
    Generates LaTeX code for a table from the structured data.
    Requires LaTeX packages: booktabs, graphicx (if resizebox used)
    """
    if not data:
        return f"% No data provided for table '{label}'\n"

    # Determine default columns if not provided
    if columns is None:
        if data:
            columns = list(data[0].keys())
            default_order = ['run_id', 'name']
            remaining_cols = [c for c in columns if c not in default_order]
            columns = [c for c in default_order if c in columns] + sorted(remaining_cols)
        else:
            columns = []

    # Sorting
    if sort_by:
        if sort_by not in columns:
            print(f"Warning: Sort key '{sort_by}' not found in data keys for table '{label}'. Sorting ignored.")
        else:
            try:
                default_sort_val = float('inf') if not reverse_sort else float('-inf')
                # Make a copy to sort only for this table generation
                data_to_sort = list(data)
                data_to_sort.sort(key=lambda x: x.get(sort_by, default_sort_val), reverse=reverse_sort)
                data = data_to_sort # Use the sorted copy
            except TypeError:
                 print(f"Warning: Could not sort by '{sort_by}' for table '{label}' due to incompatible data types. Sorting ignored.")

    # Column Alignment
    col_align = []
    for col in columns:
        col_align.append('l' if col in ['name', 'run_id'] else 'r')

    # LaTeX Generation
    latex_str = f"% --- Generated LaTeX Table: {label} ---\n"
    latex_str += "\\begin{table}[htbp]\n"
    latex_str += "  \\centering\n"
    latex_str += f"  \\caption{{{caption}}}\n"
    latex_str += f"  \\label{{{label}}}\n"
    if add_resizebox:
        latex_str += "  \\resizebox{\\textwidth}{!}{%\n" # Scale table
    latex_str += f"  \\begin{{tabular}}{{ {' '.join(col_align)} }}\n"
    latex_str += "    \\toprule\n"

    # Headers
    header_names = { 'run_id': 'Run ID', 'name': 'Name', 'bestFitness': 'Best Fitness',
                     'meanFit': 'Mean Fitness', 'meanTime': 'Mean Time (s)', 'meanEvals': 'Mean Evals' }
    header = [f"\\textbf{{{header_names.get(col, col.replace('_', ' ').title())}}}" for col in columns]
    latex_str += "    " + " & ".join(header) + " \\\\\n"
    latex_str += "    \\midrule\n"

    # Data Rows
    for row_data in data:
        row_values = []
        for col in columns:
            value = row_data.get(col, 'N/A')
            if column_formats and col in column_formats:
                 formatted_value = column_formats[col](value) if callable(column_formats[col]) else column_formats[col] % value
            elif isinstance(value, (int, float)):
                formatted_value = format_num(value, precision, sci_thresh_low, sci_thresh_high)
            elif isinstance(value, str):
                 formatted_value = value.replace('\\', '\\textbackslash{}').replace('_', '\\_').replace('%', '\\%').replace('&', '\\&').replace('#', '\\#').replace('$', '\\$').replace('{', '\\{').replace('}', '\\}').replace('~', '\\textasciitilde{}').replace('^', '\\textasciicircum{}')
            else:
                formatted_value = str(value)
            row_values.append(formatted_value)
        latex_str += "    " + " & ".join(row_values) + " \\\\\n"

    # Footer
    latex_str += "    \\bottomrule\n"
    latex_str += "  \\end{tabular}%\n"
    if add_resizebox:
        latex_str += "  } % End resizebox\n"
    latex_str += "\\end{table}\n"
    latex_str += f"% --- End Generated Table {label} ---\n"
    return latex_str

#-------------------------------------------------
# Main Script Logic
#-------------------------------------------------
all_data = []
search_path = os.path.join(RESULTS_DIR, FILE_PATTERN)
file_list = glob.glob(search_path)

if not file_list:
    print(f"No files found matching '{search_path}'")
else:
    print(f"Found {len(file_list)} files to process:")
    for filepath in sorted(file_list):
        print(f"  - Reading {filepath}")
        filename = os.path.basename(filepath)
        run_id, _ = os.path.splitext(filename)
        # Optional: Clean up run_id further if needed
        # run_id = run_id.replace('results_', '').replace('_run', '')

        file_data = read_data_from_file(filepath)
        for record in file_data:
            record['run_id'] = run_id # Add the identifier derived from filename
        all_data.extend(file_data)

print(f"\nTotal records processed: {len(all_data)}")

#-------------------------------------------------
# Generate LaTeX Output - Combined and Filtered
#-------------------------------------------------
latex_all_tables_content = "" # Accumulate table code here

if all_data:
    # --- 1. Generate Table for ALL Data (Example: Sorted by Mean Fitness) ---
    latex_all_tables_content += generate_latex_table(
        all_data,
        caption="Combined Performance Results (All Runs)",
        label="tab:combined_all",
        sort_by='meanFit',
        reverse_sort=True, # Higher fitness is better
        precision=3
    )
    latex_all_tables_content += "\n\\vspace{1em}\n" # Add some vertical space

    # --- 2. Generate Table Filtered by Specific 'name' (if specified) ---
    if FILTER_BY_NAME:
        print(f"\nFiltering data for name: '{FILTER_BY_NAME}'...")
        filtered_data = [row for row in all_data if row.get('name') == FILTER_BY_NAME]

        if filtered_data:
            print(f"Found {len(filtered_data)} records matching the name.")
            # Define columns for this table (maybe exclude 'name' as it's constant)
            name_filtered_cols = ['run_id', 'bestFitness', 'meanFit', 'meanTime', 'meanEvals']
            # Sort by a relevant metric for comparison across runs, e.g., meanTime
            latex_all_tables_content += generate_latex_table(
                filtered_data,
                caption=f"Performance Results for '{FILTER_BY_NAME}' Across Different Runs",
                label=f"tab:filtered_name_{FILTER_BY_NAME.replace('_','').lower()}", # Basic label generation
                columns=name_filtered_cols, # Use specific columns
                sort_by='meanTime', # Example: sort by time
                reverse_sort=False, # Lower time is better
                precision=3,
                add_resizebox=False # Table might be smaller, no need to resize
            )
            latex_all_tables_content += "\n\\vspace{1em}\n"
        else:
            print(f"No data found matching name '{FILTER_BY_NAME}'. Skipping name-filtered table.")

    # --- 3. Generate Table Filtered by Specific 'run_id' (if specified) ---
    #    (Useful for showing results from a single file)
    if FILTER_BY_RUN_ID:
        print(f"\nFiltering data for run_id: '{FILTER_BY_RUN_ID}'...")
        filtered_data = [row for row in all_data if row.get('run_id') == FILTER_BY_RUN_ID]

        if filtered_data:
            print(f"Found {len(filtered_data)} records matching the run_id.")
            # Define columns (maybe exclude 'run_id' as it's constant)
            runid_filtered_cols = ['name', 'bestFitness', 'meanFit', 'meanTime', 'meanEvals']
            # Sort by name within the file, for example
            latex_all_tables_content += generate_latex_table(
                filtered_data,
                caption=f"Performance Results from Run '{FILTER_BY_RUN_ID}'",
                label=f"tab:filtered_run_{FILTER_BY_RUN_ID.replace('_','').lower()}", # Basic label generation
                columns=runid_filtered_cols, # Use specific columns
                sort_by='name', # Sort alphabetically by name within the file
                reverse_sort=False,
                precision=3,
                add_resizebox=True # Might still need resize if many rows/names
            )
            latex_all_tables_content += "\n\\vspace{1em}\n"
        else:
            print(f"No data found matching run_id '{FILTER_BY_RUN_ID}'. Check RESULTS_DIR and filenames. Skipping run_id-filtered table.")

    # --- Saving to a file ---
    if latex_all_tables_content:
        output_filename = "generated_tables.tex"
        try:
            with open(output_filename, "w") as f:
                # Add necessary LaTeX document preamble
                f.write("\\documentclass{article}\n")
                f.write("\\usepackage{booktabs} % For nice table rules\n")
                f.write("\\usepackage{graphicx} % For resizebox\n")
                f.write("\\usepackage[margin=1in]{geometry} % Adjust margins\n")
                f.write("\\usepackage{amsmath}\n")
                f.write("\\usepackage{lmodern} % Use Latin Modern font\n") # Often looks better
                f.write("\\usepackage[T1]{fontenc}\n")
                f.write("\\usepackage{textcomp} % For symbols like textasciitilde\n\n") # Important for escaped chars
                f.write("\\begin{document}\n\n")

                # Write all generated table code
                f.write(latex_all_tables_content)

                # Add the note about standard deviations once at the end
                f.write("\n\\bigskip\n") # More space before the note
                f.write("\\textit{Note: Mean values are averages over runs performed within the original experiment setup that generated the input CSV files. Standard deviations for these means were not available in the input files.}\n\n")

                f.write("\\end{document}\n")
            print(f"\nLaTeX code for selected tables saved to '{output_filename}'")
            print(f"Compile using: pdflatex {output_filename}")
        except IOError as e:
            print(f"\nError writing LaTeX output to file: {e}")
    else:
        print("\nNo tables were generated (perhaps no data or no filters matched).")

else:
    print("\nNo data loaded, skipping LaTeX table generation.")