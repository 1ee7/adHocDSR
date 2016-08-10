/*
 * This file provides two funtions to deal with user interaction ;
 *
 * void send_interaction() ;
 *
 * void receive_interaction( char *data ) ;
 *
 */

#include "header.h"

#define MAX_CMD_LENGTH 10

static int checkIPStr( char IPStr[ IPSTR_LENGTH ] ) {
	int len= strlen( IPStr ) ;

	int i ;
	int nDots=0 ;
	char str[4];
	int num ;
	int index=0 ;
	for( i=0; i<len; i++ ) {
		if( IPStr[i]>='0' && IPStr[i]<='9' ){
			str[index ++] = IPStr[i] ;
		}else if( IPStr[i] == '.' ) {
			nDots ++ ;
			str[index] = '\0' ;
			num = atoi( str ) ;
			if( num>255 || num <0 ) return -1 ;

			index = 0 ;
		}else {
			return -1 ;
		}
	}
	if( nDots != 3 )  return -1 ;
	str[index] = '\0' ;
	num = atoi( str ) ;
	if( num>255 || num <0 ) return -1 ;

	return 0 ;
}

static void expireTime_interact() {
	print( OUTPUT_ELSE, "Please input path expire time( seconds ): " ) ;
	scanf( "%d", &pathExpireTime ) ;
	getchar() ;
}

static void print_interact( ) {
	print( OUTPUT_INFO, "hostIP: %s\n", hostIPStr ) ;
}

static void quit_interact() {
	exit( 0 ) ;
}

// will delete all paths to a certain destination
static void flush_interact() {
	print( OUTPUT_ELSE, "Please input the destinaton IP address: " ) ;
	char dstIPStr[ IPSTR_LENGTH ] ;
	scanf( "%s", dstIPStr ) ;
	getchar() ;

	if( checkIPStr( dstIPStr ) == -1 ) {
		print( OUTPUT_ELSE, "Wrong IP Format!!!\n" ) ;
		return ;
	}

	print( OUTPUT_ELSE, "Caution: all path to this destination will be DELETE, Y/N? : " ) ;
	char toDo=getchar() ;
	while( getchar() != '\n' ) ;
	if( toDo == 'N' || toDo == 'n' ) return ;

	IP_t dstIP = ntohl( inet_addr( dstIPStr ) ) ;
	delete_path( dstIP ) ;

}

// listen mode will not print a promot symbol on the screen
static void listen_interact() {
	print( OUTPUT_INFO, "listen_interact : Entering the listen mode. You will see all the message.\n"\
			"Press ANY key to exit.\n\n" ) ;

	while( getchar() != '\n' ) ;
}

// to send a message to a destination
static void message_interact() {
	char data[ MAX_DATA_LENGTH ] ;
	char dstIP[ IPSTR_LENGTH ] ;

	print( OUTPUT_ELSE, "Please input the destination ip: " ) ;
	scanf( "%s", dstIP ) ;
	getchar() ;
	if( checkIPStr( dstIP ) == -1 ) {
		print( OUTPUT_ELSE, "Wrong IP Format!!!!!\n" ) ;
		return ;
	}

	print( OUTPUT_ELSE, "Please input data: " ) ;
	scanf( "%s", data ) ;
	getchar() ;

	DATA dataP;
	memset(&dataP,0,sizeof(DATA));
	memcpy(dataP.data,data,strlen(data));
	dataP.srcIP=hostIP;
	dataP.dstIP= ntohl( inet_addr( dstIP ) );
	dataP.type=DATA_FLAG;
	dataP.data_len=strlen(data);
	send_packet(&dataP) ;

	listen_interact()  ;
}

static void etx_interact() {
	print_etx() ;
}

void send_interact() {
	while( 1 ) {
		print( OUTPUT_ELSE, "Ad-Hoc Net > " ) ;

		char cmd ;
		while( (cmd=getchar() ) == '\n' ) {
			print( OUTPUT_ELSE, "Ad-Hoc Net > " ) ;
		}

		switch( cmd ) {
			case 'h' :
				while( getchar() != '\n' ) ;
				print( OUTPUT_ELSE, " List of cmd \n"
						"help:\t\t print this message\n"
						"message:\t send a message\n"
						"print:\t\t print information about interface\n"
						"quit:\t\t quit the program\n"
						"flush:\t\t flush the path table\n"
						"listen:\t\t enter the listen mode\n"
						"path:\t set the path expire time\n"
						"etx:\t print all etx values\n"
						) ;
				break ;
			case 'e' :
				etx_interact() ;
				break ;
			case 'm' :
				while( getchar() != '\n' ) ;
				message_interact() ;
				break ;
			case 'p' :
#ifdef __PATH_EXPIRE__
				cmd = getchar() ;
				if( cmd == 'a' ) {
					while( getchar() != '\n' ) ;
					expireTime_interact() ;
					break ;
				}
#endif
				while( getchar() != '\n' ) ;
				print_interact() ;
				break ;
			case 'q' :
				while( getchar() != '\n' ) ;
				quit_interact() ;
				break ;
			case 'f' :
				while( getchar() != '\n' ) ;
				flush_interact() ;
				break ;
			case 'l' :
				while( getchar() != '\n' ) ;
				listen_interact() ;
				break ;
			default :
				while( getchar() != '\n' ) ;
				print( OUTPUT_ERROR, "interact: cmd ERROR, please reinput! \n" ) ;
				break ;
		}
		printf( "\n" ) ;

	}
}

void receive_interact( char *data ) {
	print( OUTPUT_INFO, "receiveInteract: receive data:\v %s\n", data ) ;
}
