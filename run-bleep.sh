#!/bin/bash

# Define the path to the executable
BLEEP_PATH="/home/deck/Coding/bleep/build/Debug/bin/bleep"

# Log file to capture output
LOG_FILE="/home/deck/Coding/bleep/build/Debug/bin/bleep.log"

# Check if the executable exists and is executable
if [ ! -f "$BLEEP_PATH" ]; then
    echo "Error: $BLEEP_PATH does not exist."
    exit 1
fi

if [ ! -x "$BLEEP_PATH" ]; then
    echo "Error: $BLEEP_PATH is not executable."
    exit 1
fi

# Add a log entry with a timestamp
echo "[$(date)] Running bleep..." >> "$LOG_FILE"

# Run the bleep executable and capture its output
"$BLEEP_PATH" "$@" >> "$LOG_FILE" 2>&1

# Check if bleep ran successfully
if [ $? -eq 0 ]; then
    echo "[$(date)] bleep ran successfully." >> "$LOG_FILE"
else
    echo "[$(date)] Error running bleep. Check the log for details." >> "$LOG_FILE"
    exit 1
fi

# Completion message
echo "[$(date)] Finished running bleep." >> "$LOG_FILE"
