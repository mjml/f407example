#!/bin/bash
openocd_pid=$(ps aux | awk '$11 == "openocd" { print $2 }')
if [[ -n $openocd_pid ]]; then
		echo "Killing openocd at $openocd_pid."
		kill -9 $openocd_pid
else
		echo "Couldn't find openocd running."
fi
