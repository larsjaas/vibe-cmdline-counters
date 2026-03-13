#!/usr/bin/env bash

# Exit immediately if any command fails
set -e

# Function to clean up any test counter files
cleanup() {
    rm -f .test*-counters.csv
}
trap cleanup EXIT

# -------------------------------------------------------------------------
# Test 1 – create counter file with default value
# -------------------------------------------------------------------------

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

# -------------------------------------------------------------------------
# Test 2 – invoke app twice, creating two counters
# -------------------------------------------------------------------------

test2() {
    ./bin/app --file .test2-counters.csv counter1
    content=$(cat .test2-counters.csv)
    if [ "$content" != "counter1,0" ]; then
        echo "test2 ... error"
        echo "After first call expected counter1,0 but got:"
        echo "$content"
        exit 1
    fi
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

# -------------------------------------------------------------------------
# Test 3 – set counter to 10 explicitly
# -------------------------------------------------------------------------

test3() {
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

# -------------------------------------------------------------------------
# Test 4 – single call with --set to create counter at 10
# -------------------------------------------------------------------------

test4() {
    rm -f .test4-counters.csv
    ./bin/app --file .test4-counters.csv --set 10 counter
    content=$(cat .test4-counters.csv)
    if [ "$content" != "counter,10" ]; then
        echo "test4 ... error"
        echo "Expected counter,10 but got:"
        echo "$content"
        exit 1
    fi
    echo "test4 ... ok"
}

# -------------------------------------------------------------------------
# Test 5 – update counter after initial creation
# -------------------------------------------------------------------------

test5() {
    rm -f .test5-counters.csv
    ./bin/app --file .test5-counters.csv counter
    content=$(cat .test5-counters.csv)
    if [ "$content" != "counter,0" ]; then
        echo "test5 ... error"
        echo "After first call expected counter,0 but got:"
        echo "$content"
        exit 1
    fi
    ./bin/app --file .test5-counters.csv --update 3 counter
    content=$(cat .test5-counters.csv)
    if [ "$content" != "counter,3" ]; then
        echo "test5 ... error"
        echo "After second call expected counter,3 but got:"
        echo "$content"
        exit 1
    fi
    echo "test5 ... ok"
}

# -------------------------------------------------------------------------
# Test 6 – delete counter after creation
# -------------------------------------------------------------------------

test6() {
    rm -f .test6-counters.csv
    ./bin/app --file .test6-counters.csv counter
    content=$(cat .test6-counters.csv)
    if [ "$content" != "counter,0" ]; then
        echo "test6 ... error"
        echo "After first call expected counter,0 but got:"
        echo "$content"
        exit 1
    fi
    ./bin/app --file .test6-counters.csv --delete counter
    # file should become empty (0‑size)
    if [ -s .test6-counters.csv ]; then
        echo "test6 ... error"
        echo "File should be empty after deletion. Current size: $(stat -c %s .test6-counters.csv)"
        exit 1
    fi
    echo "test6 ... ok"
}

# -------------------------------------------------------------------------
# Test 7 – negative values with --set and --update
# -------------------------------------------------------------------------

test7() {
    rm -f .test7-counters.csv
    # 1st call: create 'counter' with 0
    ./bin/app --file .test7-counters.csv counter
    content=$(cat .test7-counters.csv)
    if [ "$content" != "counter,0" ]; then
        echo "test7 ... error"
        echo "After first call expected counter,0 but got:"
        echo "$content"
        exit 1
    fi
    # 2nd call: create counter2 with -3
    ./bin/app --file .test7-counters.csv --set -3 counter2
    content=$(cat .test7-counters.csv)
    if [ "$content" != $'counter,0
counter2,-3' ]; then
        echo "test7 ... error"
        echo "After second call expected counter2,-3 but got:"
        echo "$content"
        exit 1
    fi
    # 3rd call: --update -1 on counter2, new value should be -4
    ./bin/app --file .test7-counters.csv --update -1 counter2
    content=$(cat .test7-counters.csv)
    if [ "$content" != $'counter,0
counter2,-4' ]; then
        echo "test7 ... error"
        echo "After third call expected counter2,-4 but got:"
        echo "$content"
        exit 1
    fi
    # Verify counter outputs
    out1=$(./bin/app --file .test7-counters.csv counter)
    if [ "$out1" != "0" ]; then
        echo "test7 ... error"
        echo "Expected output '0' for counter, got: $out1"
        exit 1
    fi
    out2=$(./bin/app --file .test7-counters.csv counter2)
    if [ "$out2" != "-4" ]; then
        echo "test7 ... error"
        echo "Expected output '-4' for counter2, got: $out2"
        exit 1
    fi
    echo "test7 ... ok"
}

# Run

test1
test2
test3
test4
test5
test6
test7

exit 0
