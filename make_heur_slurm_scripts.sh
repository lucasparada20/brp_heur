#!/bin/bash

# Define directories
INSTANCE_DIR=~/work/brp/instances_dhin
RESULTS_DIR=~/work/brp/results_dhin
SCRIPTS_DIR=~/work/brp/scripts
RUN_SCRIPT=run_slurm_scripts.sh

# Ensure the scripts directory exists
mkdir -p "$SCRIPTS_DIR"

script_count=0
# Loop through each file in the instance directory
for file in "$INSTANCE_DIR"/*.txt; do
    if [[ -f "$file" ]]; then
        filename=$(basename "$file")  # Converts : path/to/file.txt into file.txt
        base_name="${filename%.txt}"  # Removes .txt extension
        re_file="re_${base_name}.txt" # Expected result file

        # Check if the corresponding result file exists
        if [[ -f "$RESULTS_DIR/$re_file" ]]; then
            echo "Skipping $filename since $re_file exists in results."
            continue
        fi

        script_name="$SCRIPTS_DIR/${base_name}.sh"

        # Create the script with SLURM directives
        cat > "$script_name" <<EOF
#!/bin/bash -l
#SBATCH --mem=4G
#SBATCH --time=1:00:00
#SBATCH --account=def-cotej
#SBATCH --cpus-per-task=1

build/exec_heur instance_file=$file iterations=25000 > output/${base_name}.out 2> output/${base_name}.err
EOF

        chmod +x "$script_name"
        echo "Created script: $script_name"
        ((script_count++))  # Increment script count
    fi
done

echo "Total scripts created: $script_count"

# Create `run_slurm_scripts.sh` in the current directory
RUN_SCRIPT_PATH="./$RUN_SCRIPT"
echo "#!/bin/bash" > "$RUN_SCRIPT_PATH"

for script in "$SCRIPTS_DIR"/*.sh; do
    if [[ -f "$script" ]]; then
        echo "sbatch $script" >> "$RUN_SCRIPT_PATH"
    fi
done

chmod +x "$RUN_SCRIPT_PATH"

echo "Created SLURM batch submission script: $RUN_SCRIPT_PATH"
echo "Script generation complete."
