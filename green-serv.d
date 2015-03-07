#!/bin/sh
#
# Subsystem file for green-serv
#
# chkconfig: 036 95 05
# description: green-serv
#

# To install: 
#   chmod +x, mv to /etc/init.d/green-serv, then chkconfig --add green-serv

RETVAL=0
prog="green-serv"
PORT="80"
if [ $# -eq 2 ]; then
	PORT=$2
fi
PID_FILE=/var/run/green-serv/$PORT/GREENSERV_PID.pid
PORT_FILE=/var/run/green-serv/$PORT/GREENSERV.port
CMD=/var/run/green-serv/$PORT/green-serv
LOGFILE=/var/log/green-serv.log

start() {
	echo -n $"Staring $prog:"
	echo $PORT > $PORT_FILE
	nohup $CMD >> $LOGFILE 2>&1 &
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
		echo $"Usage: $0 {start [port]|stop [port]|restart}"
		RETVAL=1
esac
exit $RETVAL