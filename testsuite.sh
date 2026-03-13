#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Function to clean up test counter files
cleanup() {
    rm -f .test*-counters.csv
}

# Ensure cleanup happens on exit (success or failure)
trap cleanup EXIT

# Test 1: Create a counter file with initial value 0

test1() {
    ./bin/app --file .test1-counters.csv ticks
    if [ ! -f .test1-counters.csv ]; then
        echo "test1 ... error"
        echo "File .test1-counters.csv not found."
        exit 1
    fi
    content=$(cat .test1-counters.csv)
    if [ "$content" != "ticks,0" ]; then
        echo "test1 ... error"
        echo "Unexpected file contents:"
        echo "$content"
        exit 1
    fi
    echo "test1 ... ok"
}

# Run the tests

test1

# All tests passed
exit 0
