#!/usr/bin/env bash

# Check if file
if [ -f "$1" ]; then
    abs_path=$(realpath "$1")
    file=$(basename "$abs_path")
    folder=$(dirname "$abs_path")

    filename='error'
    # prefix='error_handler'

    # Go to the folder and generate the error handler sources
    cd "$folder" || exit

    # The generated files has to be moved manually inside the correct folder
    # error_gen -o "$filename" -x "$prefix" "$file"
    error_gen -o "$filename" "$file"
else
    >&2 echo '[ERROR]: Incorrect number of parameters'
    >&2 echo "Usage: $(basename "$0") json-file"
fi

