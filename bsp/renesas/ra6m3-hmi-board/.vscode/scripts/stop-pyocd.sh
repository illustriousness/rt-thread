#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PID_FILE="${ROOT_DIR}/.vscode/.pyocd.pid"

if [[ ! -f "${PID_FILE}" ]]; then
    exit 0
fi

PID="$(cat "${PID_FILE}" || true)"
if [[ -n "${PID}" ]] && kill -0 "${PID}" 2>/dev/null; then
    kill "${PID}" || true

    for _ in $(seq 1 30); do
        if ! kill -0 "${PID}" 2>/dev/null; then
            break
        fi
        sleep 0.1
    done

    if kill -0 "${PID}" 2>/dev/null; then
        kill -9 "${PID}" || true
    fi
fi

rm -f "${PID_FILE}"
