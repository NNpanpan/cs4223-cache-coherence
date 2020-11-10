#!/bin/bash

bin_path="../coherence"

time_string="\nreal %es\nuser %Us\nsys %S\nCPU %P\ncontext-switches %c vol %w invol\npage-faults %F maj %R min\nswaps %W\n"
sim_string="/usr/bin/time -f \$(FORMAT) ${bin_path}"

protocols=("MESI" "Dragon")
# protocols=("MESI" "Dragon" "MESIF")
benchmarks=("blackscholes" "bodytrack" "fluidanimate")

# Default simulation parameters
# Word size: 4 B (32 bits)
# Cache size: 4 KB (4096 B)
# Associativity: 2-way set-associative
# Block size: 16 B (4 words)
DEF_SIZE=4096
DEF_ASSOC=2
DEF_BLOCK=16

# Parameters to vary from default
cache_size=(128 256 512 1024 2048 4096 8192 16384 32768 65536)
assoc=(1 2 4 8 16 32 64 128 256 512)
block_size=(4 8 16 32 64 128 256 512 1024 2048)

# Remove existing generated makefile
if [[ -e "Makefile" ]]
then
    rm "Makefile"
fi

recipe_id=0
echo "FORMAT:=\"${time_string}\"" >> Makefile
echo "" >> Makefile

for p in "${protocols[@]}"; do
for bm in "${benchmarks[@]}"; do
    base_string="${p} ${bm}"

    for c in "${cache_size[@]}"; do
    for a in "${assoc[@]}"; do
    for b in "${block_size[@]}"; do
        varied=0
        if (( $c != $DEF_SIZE )); then varied=$((varied + 1)); fi
        if (( $a != $DEF_ASSOC )); then varied=$((varied + 1)); fi
        if (( $b != $DEF_BLOCK )); then varied=$((varied + 1)); fi
 
        if [[ "$1" == "all" || $varied -lt 2 ]]
        then
            # Compute size of set and number of sets in each cache
            set_size=$((a * b))
            num_sets=$((c / set_size))

            # Skip configuration if cache is too small for one set
            if (( $set_size <= $c ))
            then
                param_string="${base_string} ${c} ${a} ${b}"
                log_path="../results/${p}-${bm}-${c}-${a}-${b}.time"
                cmd_string="${sim_string} ${param_string} >${log_path} 2>&1"

                # Build parallel makefile
                recipe_id=$((recipe_id + 1))

                echo "task${recipe_id}:" >> Makefile
                echo -e "\t${cmd_string}" >> Makefile
                echo "" >> Makefile
            fi
        fi
    done
    done
    done
done
done

all_recipe="all:"
for i in $(eval seq 1 $recipe_id)
do
    all_recipe="${all_recipe} task${i}"
done

echo "${all_recipe}

$(cat Makefile)" > Makefile

