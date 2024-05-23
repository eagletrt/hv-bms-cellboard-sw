#!/usr/bin/env bash

# Check if file
if [ -f "$1" ]; then
    abs_path=$(realpath "$1")
    file=$(basename "$abs_path")
    folder=$(dirname "$abs_path")

    # Go to the folder and generate FSM sources
    cd "$folder" || exit
    gv_fsm -p cellboard_fsm -d 'Main cellboard FSM that handles all the operations of the BMS' -o fsm -x fsm -l "$file"
else
    >&2 echo '[ERROR]: Incorrect number of parameters'
    >&2 echo "Usage: $(basename "$0") dot-file"
fi

