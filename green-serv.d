#!/bin/sh
#
# /etc/init.d/example
# Subsystem file for green-serv
#
# chkconfig: 036 95 05
# description: green-serv
#

# Be sure this priority comes after mysql is up and running
# To install: 
#   chmod +x, mv to /etc/init.d/green-serv, then chkconfig --add green-serv

RETVAL=0
prog="green-serv"
PID_FILE=/var/run/green-serv/GREENSERV_PID.pid
CMD=/var/run/green-serv/green-serv

start() {
	echo -n $"Staring $prog:"
	nohup $CMD &
	echo
}

stop() {
	echo -n $"Stopping $prog:"
	if [ -f $PID_FILE ]; then kill `cat $PID_FILE`; fi
	echo
}

if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		stop
		start
		;;
	*)
		echo $"Usage: $0 {start|stop|restart}"
		RETVAL=1
esac
exit $RETVAL