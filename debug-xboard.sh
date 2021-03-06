#!/usr/bin/env bash

export DISPLAY=:0

XB_FCP="make -s run"
XB_DEBUG_FILE="xboard.debug"

>"$XB_DEBUG_FILE"

{
	setsid -w                                \
	xboard -fcp "$XB_FCP"                    \
	       -debug                            \
	       -nameOfDebugFile "$XB_DEBUG_FILE" \
	       -engineDebugOutput 1              \
	2>/dev/null

	# kill -SIGTERM $$ &>/dev/null
} &

xb_pid=$(pgrep -P $!)

less -Q +F "$XB_DEBUG_FILE"

kill -SIGTERM $xb_pid &>/dev/null
kill -SIGHUP $xb_pid &>/dev/null

exit 0
