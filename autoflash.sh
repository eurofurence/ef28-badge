#!/bin/sh

# Example usage: ./autoflash.sh /dev/ttyACM0 autoflash_ACM0.log

PORT=${1:-/dev/ttyACM0}
OUTFILE=${2:-autoflash_ACM0.log}

while true; do
  echo ""
  echo "# Waiting for new device on ${PORT}…"
  while [ ! -e "${PORT}" ]; do
    sleep 0.5
  done
  echo "# Found new device!"
  sleep 1

  echo "" | tee -a ${OUTFILE}
  echo -n "========== " | tee -a ${OUTFILE}
  echo -n $(date -Iseconds) | tee -a ${OUTFILE}
  echo -n " ${PORT}" | tee -a ${OUTFILE}
  echo " ==========" | tee -a ${OUTFILE}

  pio run --upload-port ${PORT} --target upload 2>&1 | tee -a ${OUTFILE}

  sleep 1
  echo "# Waiting for device disconnect…"
  while [ -e "${PORT}" ]; do
    sleep 0.5
  done
done
