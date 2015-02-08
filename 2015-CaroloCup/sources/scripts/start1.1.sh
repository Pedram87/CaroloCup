#!/bin/bash

home=/home/odroid/CaroloCup/2014-CaroloCup/Legendary/project/scripts
bin=/opt/msv/bin/
caroloCup=$bin/2013/DIT-168/project-template/
pidfile=${0}.pid
serialPort=/dev/ttyACM1
started=0

# Port setting
stty -F $serialPort raw speed 9600

#ifconfig lo multicast
#route add -net 224.0.0.0 netmask 240.0.0.0 dev lo

#Loop
cd $home
while read -n1 line #OPTIONS: -n 10 (Legendary car), -n 1 (Wooden car)
do

   echo $line
   if [[ $line == "0" ]]; then
    if [[ $started == 1 ]]; then
	    echo "STOP"
	    # Stop and kill the processes from the pidfile
	   # for pid in $(tac $pidfile); do
	    #  kill -SIGINT $pid
	    #  sleep 1
	  #  done

	   #Kill all processes
	   killall supercomponent
           echo "Supercomponent is stopped"

	   killall proxy
	   echo "Proxy is stopped"

	   killall driver
 	   echo "Driver is stopped"
	    started=0
    fi
  elif [[ $line == "1" ]]; then
    if [[ $started == 0 ]]; then
	    echo "START LANEFOLLOWING"
#cp configuration1 configuration
	    killall supercomponent
	    # Start the processes
	    
	    cd /opt/msv/bin/
	    nohup ./supercomponent --cid=111 &
	    echo "$!" > $pidfile

        nohup ${caroloCup}/proxy --cid=111 --freq=20 &
        echo "$!" >> $pidfile

        nohup ${caroloCup}/lanedetector --cid=111 --freq=20 &
	    echo "$!" >> $pidfile

#Note that this is only lanedriver!
	    nohup ${caroloCup}/lanedriver --cid=111 --freq=40 &
	    echo "$!" >> $pidfile
	    started=1
    fi

    # Change back to 2 for parking
  elif [[ $line == "2" ]]; then
    if [[ $started == 0 ]]; then
	    echo "START PARKING"
	    cp configuration2 configuration
	    killall supercomponent
	    # Start the processes
	    
	    cd /opt/msv/bin/
	    echo "Starting"
	    nohup ./supercomponent --cid=222 &
	    echo "$!" >> $pidfile
	    echo "Supercomponent has started"

	    cd 2013/DIT-168/project-template/
	    nohup ./proxy --cid=222 --freq=60 &
	    echo "$!" >> $pidfile
	    echo "Proxy has started"
	    echo "Starting Driver in a few seconds"
	    sleep 5 #Waits n seconds before start Driver component  
          
	    nohup ./driver --cid=222 --freq=40 &
	    echo "$!" >> $pidfile
	    echo "Driver has started"
	    started=1
    fi
   fi
done < $serialPort

