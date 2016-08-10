#! /bin/bash
clear
iwconfig wlp3s0 essid "cui" mode ad-hoc
sleep 1 
iwconfig wlp3s0
ifconfig wlp3s0 162.105.1.1  netmask 255.255.255.0 up
sleep 1
ifconfig wlp3s0
#eof
