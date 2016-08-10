#! /bin/bash
iptables -L -n --line-number
iptables -D INPUT 1
#eof
