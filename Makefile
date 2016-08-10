ad-hoc: timer.o receive_packet.o linkLayer.o utils.o send_packet.o path.o interact.o requestID.o  probe.o main.c
	gcc timer.o receive_packet.o linkLayer.o utils.o send_packet.o path.o interact.o requestID.o probe.o main.c -o  ad-hoc -lpthread -lrt

timer.o: timer.c header.h
	gcc -c  timer.c -o  timer.o 

interact.o: interact.c header.h
	gcc -c interact.c -o interact.o 

receive_packet.o:receive_packet.c header.h 
	gcc -c  receive_packet.c -o receive_packet.o 

linkLayer.o: linkLayer.c header.h
	gcc -c  linkLayer.c -o linkLayer.o 

send_packet.o: send_packet.c header.h
	gcc -c  send_packet.c -o send_packet.o 

cache.o: path.c header.h
	gcc -c  path.c -o path.o 

requestID.o: requestID.c header.h
	gcc -c  requestID.c -o requestID.o 

utils.o: utils.c header.h
	gcc -c  utils.c -o utils.o 

probe.o: probe.c header.h
	gcc -c probe.c -o probe.o



.PHONY:clean
clean:
	rm -rf *.o ad-hoc
