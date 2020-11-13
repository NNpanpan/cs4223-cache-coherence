#!/bin/bash

LOG_PATHS=("../results")
OUTPUT_PATH=("./simulation-results.csv")

CONFIG_REGEX="([A-Za-z]+)-([A-Za-z]+)-([0-9]+)-([0-9]+)-([0-9]+)\.time";
TIME_REGEX='real ([0-9]+\.[0-9]+)s'$'\n''user ([0-9]+\.[0-9]+)s'$'\n''sys ([0-9]+\.[0-9]+)'$'\n''CPU ([0-9]+)%'$'\n''context-switches ([0-9]+) vol ([0-9]+) invol'$'\n''page-faults ([0-9]+) maj ([0-9]+) min'$'\n''swaps ([0-9]+)$'

CORE_EXEC_REGEX="Core [0-3]: ([0-9]+) cycle(.*)"
LD_ST_REGEX="Core [0-3]: ([0-9]+) load\(s\), ([0-9]+) store(.*)"
MISS_RATE_REGEX="Core [0-3]: ([0-9]+\.[0-9]+%)(.*)"
ACCESS_REGEX="Core [0-3]: ([0-9]+) / [0-9]+ = ([0-9]+\.[0-9]+%)(.*)"

TOTAL_EXEC_REGEX="Total: ([0-9]+)"
TRAFFIC_REGEX="([0-9]+) byte"
INVALIDN_REGEX="Invalidation: ([0-9]+)"
UPDATE_REGEX="Update: ([0-9]+)"
WRITEBACK_REGEX="Write-back: ([0-9]+)"
TOTAL_ACCESS_REGEX="Total : ([0-9]+) / [0-9]+ = ([0-9]+\.[0-9]+%)"

headers="Log File;Protocol;Benchmark;Cache Size;Associativity;Block Size;Max Exec;Exec 0;Exec 1;Exec 2;Exec 3;Comp 0;Comp 1;Comp 2;Comp 3;LD 0;ST 0;LD 1;ST 1;LD 2;ST 2;LD 3;ST 3;Idle 0;Idle 1;Idle 2;Idle 3;Miss% 0;Miss% 1;Miss% 2;Miss% 3;Traffic;Invalidations;Updates;Write-backs;Priv% T;Priv% 0;Priv% 1;Priv% 2;Priv% 3;Priv T;Priv 0;Priv 1;Priv 2;Priv 3"

# Add the CSV file headers before printing the data of the first row
headers="$headers;Real (s);User (s);Sys (s);CPU (%);Ctx-sw (vol);Ctx-sw (invol);PFault (maj);PFault (min);Swaps"
echo $headers > $OUTPUT_PATH

for dir in "${LOG_PATHS[@]}"
do
for file in $dir/*
do
    if [[ $file =~ $CONFIG_REGEX ]]
    then
        # Process filename and file contents
        filename=$(basename $file)
        log="$(cat $file)"

        echo "Processing $filename"

        # First parse simulation parameters from simulation log filename
        p="${BASH_REMATCH[1]}"
        bm="${BASH_REMATCH[2]}"
        c="${BASH_REMATCH[3]}"
        a="${BASH_REMATCH[4]}"
        b="${BASH_REMATCH[5]}"

        data="$filename;$p;$bm;$c;$a;$b"

        for i in {1..8}
        do
            [[ $log =~ $CORE_EXEC_REGEX ]]
            data="$data;${BASH_REMATCH[1]}"
            log="${BASH_REMATCH[2]}"
            
            if (( $i == 4 ))
            then
                [[ $log =~ $TOTAL_EXEC_REGEX ]]
                data="$data;${BASH_REMATCH[1]}"
            fi
        done

        for i in {1..4}
        do
            [[ $log =~ $LD_ST_REGEX ]]
            data="$data;${BASH_REMATCH[1]};${BASH_REMATCH[2]}"
            log="${BASH_REMATCH[3]}"
        done

        CURR_REGEX=$CORE_EXEC_REGEX
        for i in {1..8}
        do
            [[ $log =~ $CURR_REGEX ]]
            data="$data;${BASH_REMATCH[1]}"
            log="${BASH_REMATCH[2]}"

            if (( $i == 4 )); then CURR_REGEX=$MISS_RATE_REGEX; fi
        done

        [[ $log =~ $TRAFFIC_REGEX ]]
        traffic="${BASH_REMATCH[1]}"

        [[ $log =~ $INVALIDN_REGEX ]]
        ivls="${BASH_REMATCH[1]}"

        [[ $log =~ $UPDATE_REGEX ]]
        upds="${BASH_REMATCH[1]}"
 
        [[ $log =~ $WRITEBACK_REGEX ]]
        wbs="${BASH_REMATCH[1]}"

        [[ $log =~ $TOTAL_ACCESS_REGEX ]]
        private_rate="${BASH_REMATCH[2]}"
 
        data="$data;$traffic;$ivls;$upds;$wbs;$private_rate"

        priv_counts="${BASH_REMATCH[1]}"
        for i in {1..4}
        do
            [[ $log =~ $ACCESS_REGEX ]]
            data="$data;${BASH_REMATCH[2]}"
            priv_counts="$priv_counts;${BASH_REMATCH[1]}"
            log="${BASH_REMATCH[3]}"
        done

        data="$data;$priv_counts"

        # Append simulation execution statistics
        if [[ $log =~ $TIME_REGEX ]]
        then
            real="${BASH_REMATCH[1]}"
            user="${BASH_REMATCH[2]}"
            sys="${BASH_REMATCH[3]}"
            cpu="${BASH_REMATCH[4]}"
            csvol="${BASH_REMATCH[5]}"
            csinvol="${BASH_REMATCH[6]}"
            pfmaj="${BASH_REMATCH[7]}"
            pfmin="${BASH_REMATCH[8]}"
            swaps="${BASH_REMATCH[9]}"
            data="$data;$real;$user;$sys;$cpu;$csvol;$csinvol;$pfmaj;$pfmin;$swaps"
        else
            echo "Simulation log file [$file] has incorrect /usr/bin/time format!"
            exit 1
        fi

        echo $data >> $OUTPUT_PATH
    else
        echo "Ignoring non-simulation log [$file]!"
    fi
done
done

