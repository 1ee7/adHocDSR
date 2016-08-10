/*
 * the file provides three functions to deal with staff about link layer
 *
 * int send_to_link_layer ( IP_t nextHopIP, char *segment, asize_t seg_len )
 * 		to send a packet out.
 *
 * void init_sockets()
 * 		to set the socket.
 *
 * void *receive_from_link_layer( void * noUse)
 * 		to receive a packet from link layer. it promises that receive_packet()
 * 	will be invoked when receiving a packet .
 *
 */

#include "header.h"

#define BROADCAST_IP_STR ("162.105.1.255")
//#define PORT_OUT 		9998
#define PORT_IN			9934

static struct sockaddr_in sockSendToAddr, sockReceiveFromAddr ;
static int sockSend, sockReceive ;

static void set_local_ip() {
	broadcastIP = ntohl( inet_addr( BROADCAST_IP_STR ) ) ;
	print( OUTPUT_INFO, "set_local_ip: BROADCAST_IP: %s\n", BROADCAST_IP_STR ) ;

	struct ifaddrs *pIfAddr = NULL ;
	getifaddrs( & pIfAddr ) ;

	while( pIfAddr != NULL ) {
		if( pIfAddr->ifa_addr->sa_family == AF_INET  \
				&& pIfAddr->ifa_name[0] == 'w' ) {

			inet_ntop( AF_INET, \
					&( (struct sockaddr_in *) pIfAddr->ifa_addr	)->sin_addr,\
					hostIPStr, INET_ADDRSTRLEN ) ;
			hostIP = ntohl( inet_addr( hostIPStr ) ) ;

			print( OUTPUT_INFO, "set_local_ip: hostIP: %s\n", ip_to_str( hostIP ) ) ;

			return ;
		}

		pIfAddr = pIfAddr -> ifa_next ;
	}

	print( OUTPUT_ERROR, "set_local_ip: no interface \n" ) ;
}

int send_to_link_layer ( IP_t nextHopIP, char *segment, asize_t seg_len ) {
	char packet[ MAX_PACKET_LENGTH ] ;
	asize_t packet_len = seg_len + sizeof( IP_t ) ;
	*( (IP_t *) packet) = nextHopIP ;
	memcpy( packet+sizeof( IP_t ), segment, seg_len ) ;
	int status = sendto( sockSend, packet, packet_len, 0, \
								(struct sockaddr *) &sockSendToAddr, sizeof( struct sockaddr_in ) ) ;
	//print( OUTPUT_LOG, "send_to_link_layer: send a packet, the status is %d\n", status ) ;

	// to print the content of packets in hexadecimal
	if( segment[0] == PROBE_FLAG ) {
		return status ;
	}
#ifdef __TEST_DEBUG__
	LOCK_OUTPUT ;

	int i ;
	unsigned char * p = ( unsigned char *) packet ;
	printf( "\n send_to_link_layer : send a packet\n" ) ;
	for( i=0; i<packet_len; i++ ) {
		printf( " %02x", p[i] ) ;
	}
	printf( "\n SEND PACKET END\n\n" ) ;

	FREE_OUTPUT ;
#endif

	return status ;
}

void init_sockets() {
	set_local_ip() ;

	int status ;

	sockSend = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ;

	int yes=1 ;
	if( setsockopt( sockSend, SOL_SOCKET, SO_BROADCAST, &yes, sizeof( int ) ) < 0 ) {
		print( OUTPUT_ERROR, "init_sockets: setsockopt error \n" ) ;
   		exit( 1 ) ;
	}

	memset( &sockSendToAddr, 0, sizeof( struct sockaddr_in ) ) ;
	sockSendToAddr.sin_family = AF_INET ;
	sockSendToAddr.sin_addr.s_addr = htonl( broadcastIP ) ;
	sockSendToAddr.sin_port = htons( PORT_IN ) ;


	memset( &sockReceiveFromAddr, 0, sizeof( struct sockaddr_in ) ) ;
	sockReceiveFromAddr.sin_family = AF_INET ;
	sockReceiveFromAddr.sin_addr.s_addr = htonl( INADDR_ANY ) ;
	sockReceiveFromAddr.sin_port = htons( PORT_IN ) ;

	sockReceive = socket( AF_INET, SOCK_DGRAM, 0 ) ;
	if( bind( sockReceive, (struct sockaddr *) &sockReceiveFromAddr, sizeof( struct sockaddr_in ) ) < 0 ) {
		print( OUTPUT_ERROR, "init_sockets: bind error \n" ) ;
		exit( 1 ) ;
	}
}

static void receive_handler( char *packet, asize_t packet_len ) {
	IP_t hopIP = * ( (IP_t *) packet ) ;

	if( ( *( packet + sizeof( IP_t ) ) ) == PROBE_FLAG ) {
		if(  (( PROBE_t *) ( packet +sizeof( IP_t ) ) )->srcIP == hostIP ) {
			return ;
		}
	}

	if( hopIP == hostIP || hopIP == broadcastIP ) {

#ifdef __TEST_DEBUG__
if( (*( packet + sizeof( IP_t ) ) ) == PROBE_FLAG ) {
	 ;
}else {
		LOCK_OUTPUT ;

		int i ;
		unsigned char * p = ( unsigned char *) packet ;
		printf( "\n receive: receive a packet: \n" ) ;
		for( i=0; i<packet_len; i++ ) {
			printf( " %02x", p[i] ) ;
		}
		printf( "\n RECEIVE PACKET END \n\n" ) ;

		FREE_OUTPUT ;
}
#endif

		receive_packet( packet+sizeof( IP_t ), packet_len - sizeof( IP_t ) ) ;
	}
}

void *receive_from_link_layer( void * noUse) {
	char buffer[ MAX_PACKET_LENGTH ] ;

	struct sockaddr_in  srcAddr ;
	int srcAddrLength ;

	asize_t sockAddrLen = sizeof( struct sockaddr_in ) ;
	asize_t length = 0 ;
	while( 1 ) {
		memset( buffer, 0, MAX_PACKET_LENGTH ) ;
		if( (length = recvfrom( sockReceive, buffer, MAX_PACKET_LENGTH, 0,
					( struct sockaddr *) &srcAddr, &srcAddrLength ) ) < 0 ){
			print( OUTPUT_ERROR, "\n[Error] receive_thread: recvfrom error \n" ) ;
			exit( 1 ) ;
		}

		receive_handler( buffer, length ) ;
	}

	print( OUTPUT_ERROR, "\n[Error] receive_thread: should never show\n" ) ;
}
