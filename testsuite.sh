#!/usr/bin/env bash

# Exit immediately if any command fails
set -e

# Clean up any test counter files on exit
cleanup() {
    rm -f .test*-counters.csv
}
trap cleanup EXIT

# ------------------------------
# Test 1 – create counter1 with default value
# ------------------------------

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

# ------------------------------
# Test 2 – invoke app twice, creating two counters
# ------------------------------

test2() {
    # First invocation creates counter1
    ./bin/app --file .test2-counters.csv counter1
    content=$(cat .test2-counters.csv)
    if [ "$content" != "counter1,0" ]; then
        echo "test2 ... error"
        echo "After first call expected counter1,0 but got:"
        echo "$content"
        exit 1
    fi

    # Second invocation should add counter2
    ./bin/app --file .test2-counters.csv counter2
    content=$(cat .test2-counters.csv)
    expected=$'counter1,0
counter2,0'
    if [[ "$content" != "$expected" ]]; then
        echo "test2 ... error"
        echo "After second call expected two lines but got:"
        echo "$content"
        exit 1
    fi
    echo "test2 ... ok"
}

# ------------------------------
# Test 3 – set counter value to 10
# ------------------------------

test3() {
    # Ensure the file starts clean
    rm -f .test3-counters.csv
    ./bin/app --file .test3-counters.csv counter1
    ./bin/app --file .test3-counters.csv --set 10 counter1
    content=$(cat .test3-counters.csv)
    if [ "$content" != "counter1,10" ]; then
        echo "test3 ... error"
        echo "Expected counter1,10 but got:"
        echo "$content"
        exit 1
    fi
    echo "test3 ... ok"
}

# Run the tests

test1
test2
test3

# All tests passed
exit 0
