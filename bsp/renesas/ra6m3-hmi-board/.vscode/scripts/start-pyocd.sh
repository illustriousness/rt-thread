#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PID_FILE="${ROOT_DIR}/.vscode/.pyocd.pid"
LOG_FILE="${ROOT_DIR}/.vscode/.pyocd.log"

PYOCD_BIN="${PYOCD_BIN:-/usr/bin/pyocd}"
TARGET="${PYOCD_TARGET:-r7fa6m3ah}"
GDB_PORT="${PYOCD_GDB_PORT:-50000}"
TELNET_PORT="${PYOCD_TELNET_PORT:-50001}"
FREQUENCY="${PYOCD_FREQUENCY:-4000000}"

if [[ -f "${PID_FILE}" ]]; then
    OLD_PID="$(cat "${PID_FILE}" || true)"
    if [[ -n "${OLD_PID}" ]] && kill -0 "${OLD_PID}" 2>/dev/null; then
        echo "pyOCD already running (pid=${OLD_PID})"
        exit 0
    fi
    rm -f "${PID_FILE}"
fi

rm -f "${LOG_FILE}"
nohup "${PYOCD_BIN}" gdbserver \
    --target "${TARGET}" \
    --port "${GDB_PORT}" \
    --telnet-port "${TELNET_PORT}" \
    --frequency "${FREQUENCY}" \
    >"${LOG_FILE}" 2>&1 &

PID=$!
echo "${PID}" > "${PID_FILE}"

for _ in $(seq 1 80); do
    if grep -q "GDB server listening on port ${GDB_PORT}" "${LOG_FILE}" 2>/dev/null; then
        echo "pyOCD started (pid=${PID}, port=${GDB_PORT})"
        exit 0
    fi

    if ! kill -0 "${PID}" 2>/dev/null; then
        echo "pyOCD exited unexpectedly."
        tail -n 80 "${LOG_FILE}" || true
        exit 1
    fi

    sleep 0.1
done

echo "Timed out waiting for pyOCD to start on port ${GDB_PORT}."
tail -n 80 "${LOG_FILE}" || true
exit 1
