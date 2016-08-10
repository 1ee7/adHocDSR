//the main function

#include "header.h"

static void init() {
	outputLock = 0 ;

	pathExpireTime = 100 ;

	init_Probetimer () ;
	init_sockets() ;
	init_send_buffer() ;
	init_rreq_buffer();
	init_rrep_buffer();
	init_record_table() ;
	init_path_cache() ;
}

int
main( int argc, char *argv) {
	init() ;

	pthread_t receiveThread ;
	pthread_create( &receiveThread, NULL, receive_from_link_layer, NULL ) ;

	send_interact() ;

	return 0 ;
}
