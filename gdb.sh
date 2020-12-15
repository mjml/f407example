#!/bin/bash

source ~/bin/functions/pids

openocd_init ()
{
		local pid=$(get_pid openocd)
		if [ -z "$pid" ]; then
				printf "Starting openocd...\n"
				openocd &
				wait_pid openocd
				sleep 0.8
		fi
		
		# Now start up orbuculum and put orbcat in another window
		#printf "Starting orbuculum...\n"
		#orbuculum -f swout.txt &
		#wait_pid orbuculum

}


openocd_finish ()
{
		local pid=$(get_pid openocd)
		if [ -n  "$pid" ]; then
				printf "Shutting down openocd at $pid...\n"
				(printf "tpiu config disable\n"; sleep 0.1; printf "exit\n") | nc localhost 4444

				# Shut down orbuculum / orbcat as well
				kill -6 $pid
				#killthe orbuculum
				orbcat_finish

		
				# Wait for openocd to be confirmed shut down
				wait_gone_pid "$pid"
		fi
}

swoview_init ()
{
		# For now, I use tail instead of orbcat.
		# There's only ever one message type and orbuculum doesn't really benefit this situation
		printf "Starting orbcat/tail...\n"
#		xfce4-terminal --geometry 120x60+0+0 -e "orbcat -c 0,%c" &
		xfce4-terminal --geometry 120x60+0+0 -e "tail -f swout.txt" &
		swoview_pid=$!
}

swoview_finish ()
{
		if [ -n "$swoview_pid" ]; then
				swoview_pid=$(ps aux | grep -P "xfce4-terminal.*tail.*swout\.txt" | awk -e '{print $2}')
		fi
		if [ -n "$swoview_pid" ]; then
				kill -9 $swoview_pid
		fi
}

if [ -n "$REPORT_PID" ]; then
		pid=$(get_pid openocd)
		printf "OpenOCD pid is $pid\n"
		exit
fi


if [ -n "$RESTART_DEBUG_SESSION" ]; then
		openocd_finish
		swoview_finish
fi

openocd_init
swoview_init

# Run gdb
printf "Starting GDB.\n"
arm-none-eabi-gdb $@

swoview_finish

if [ -n $(get_pid openocd) ]; then
		echo OpenOCD is still running...
else
		echo OpenOCD is no longer running.
fi
		

if [ -n $(get_pid orbuculum) ]; then
		echo Orbuculum is still running...
else
		echo Orbuculum is no longer running.
fi

