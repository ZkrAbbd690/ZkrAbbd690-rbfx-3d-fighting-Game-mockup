#!/bin/bash

# Output file
output="updated-physics.txt"

# Clear the output file if it exists
> "$output"

# List of files to process
files=(
    "CameraController.h" 
    "Combatant.h" 
    "CombatSystem.cpp" 
    "CombatSystem.h"
    "Fighter.cpp" 
    "Fighter.h" 
    "main.cpp" 
    "Zombie.cpp" 
    "Zombie.h" 
    "CameraController.cpp" 
    "last-ref-main.cpp" 
    "ZombieAI.cpp"
    "ZombieManager.cpp"
    "ZombieAI.h"
    "ZombieManager.h"
    
    
    
)

# Loop through each file
for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        # Write filename in quotes
        echo "\"$file\"" >> "$output"
        # Write separator
        echo "---------" >> "$output"
        # Write file content
        cat "$file" >> "$output"
        # Add a blank line between files for readability
        echo "" >> "$output"
    else
        echo "Warning: $file not found, skipping..." >&2
    fi
done

echo "Done! Output written to $output"