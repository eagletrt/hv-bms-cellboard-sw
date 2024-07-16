#!/usr/bin/env bash

# Check if file
if [ -f "$1" ]; then
    abs_path=$(realpath "$1")
    file=$(basename "$abs_path")
    folder=$(dirname "$abs_path")

    # Go to the folder and generate FSM sources
    cd "$folder" || exit
    gv_fsm -p cellboard_bms_monitor_fsm -d 'Cellboard FSM that handles all the operations of the BMS monitor' -o bms-monitor-fsm -x bms_monitor_fsm -l "$file"
else
    >&2 echo '[ERROR]: Incorrect number of parameters'
    >&2 echo "Usage: $(basename "$0") dot-file"
fi

