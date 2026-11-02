#!/usr/bin/env bash
# Each datagram loop runs a received datagram through the admission gate
# before it reads the datagram (docs/protocol/datagram-sources.md): the
# gameserver's LoginServerManager::run and the loginserver's
# GameServerManager::run. Neither loop has a test of its own, since each
# opens a database connection before it reads, so this audit holds the
# wiring: in each file the gate call must come before the read, and the
# read must sit inside the branch the gate's answer opens.
#
#   bash tests/tools/check_datagram_gate.sh [repo-root]
#
# Exit status: 0 when both loops are gated, 1 naming the file otherwise.

set -u
root=${1:-.}
status=0

check_loop() {
	local file=$1
	local gate read
	gate=$(grep -n 'if (m_Gate.admits(\*pDatagram)) {' "$file" | head -n 1 | cut -d: -f1)
	read=$(grep -n 'pDatagram->read(pDatagramPacket);' "$file" | head -n 1 | cut -d: -f1)
	if [ -z "$gate" ]; then
		echo "ERROR: $file: no 'if (m_Gate.admits(*pDatagram)) {' before the read" >&2
		status=1
		return
	fi
	if [ -z "$read" ]; then
		echo "ERROR: $file: no 'pDatagram->read(pDatagramPacket);'" >&2
		status=1
		return
	fi
	if [ "$gate" -ge "$read" ]; then
		echo "ERROR: $file: the gate (line $gate) does not come before the read (line $read)" >&2
		status=1
		return
	fi
	# The read is the first statement inside the gate's branch: nothing
	# between the two lines but whitespace and comments. An adjacent pair
	# has nothing between, and is not asked: sed prints the first line of
	# an inverted range.
	if [ "$((gate + 1))" -le "$((read - 1))" ] \
		&& sed -n "$((gate + 1)),$((read - 1))p" "$file" | grep -v -E '^[[:space:]]*(//.*)?$' | grep -q .; then
		echo "ERROR: $file: a statement sits between the gate (line $gate) and the read (line $read)" >&2
		status=1
		return
	fi
	echo "OK: $file gates the read (gate at line $gate, read at line $read)"
}

check_loop "$root/src/server/gameserver/LoginServerManager.cpp"
check_loop "$root/src/server/loginserver/GameServerManager.cpp"
exit $status
