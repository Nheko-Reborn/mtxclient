#!/bin/bash

set -ex

cd build/

# Capture coverage info.
lcov --directory . --capture --output-file coverage.info 

# Filter out external code.
lcov --remove coverage.info \
    '/usr/*' \
    '*third-party*' \
    '*tests*' \
    --output-file coverage.info 

# Display results.
lcov --list coverage.info 
