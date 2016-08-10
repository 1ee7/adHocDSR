/* 
 * this file provides two useful( maybe not) functions 
 *
 * void print( output_t output, char *fmt, ...) 
 * 		for output. the main reason of this function is to lock the output. without this function, 
 * 	the output of receive thread may interrupt the output of the send thread, making the output a mess.
 *
 *
 * char *ip_to_str( IP_t ip )
 * 		to convert the ip from number to a human-readable string, mainly used in the interaction module
 *
 */

#include "header.h"

void print( output_t output, char *fmt, ... ) {
	static asize_t num[3]= {0};

	va_list args ;
	va_start( args, fmt ) ;

	LOCK_OUTPUT ; 

	switch( output ) {
		case OUTPUT_INFO : 
			printf( "\n[INFO %d]\t", ++num[0] ) ;
			vprintf( fmt, args )  ;
			printf( "\n" ) ;
			break ;
		case OUTPUT_ERROR :
			printf( "\n[ERROR %d]\t", ++num[1] ) ;
			vprintf( fmt, args ) ;
			printf( "\n" ) ;
			break ;
		case OUTPUT_LOG :
			printf( "\n[LOG] %d]\t", ++num[2] ) ;
			vprintf( fmt, args ) ;
			printf( "\n" ) ;
			break ;
		default :
			vprintf( fmt, args ) ;
			break ;
	}
	va_end( args ) ;
	fflush(stdout);
	FREE_OUTPUT ;
}

char *ip_to_str( IP_t IP ) {
	static char ipStr[ IPSTR_LENGTH ] ;

	sprintf( ipStr, "%u.%u.%u.%u", IP>>24, (IP&0x00ff0000)>>16, (IP&0x0000ff00)>>8, (IP&0x000000ff ) ) ;

	return ipStr ;
}
