/* 
 * this file deals with requestID in RREQ packets. it provides three interface 
 *
 * void init_record_table()
 * 		to set RREQTable and IDTable to all zeros 
 *
 * unsigned int get_ID( IP_t dstIP ) 
 * 		get a uniques requestID when sending a RREQ for dstIP 
 *
 * int check_new_RREQ( IP_t srcIP, IP_t dstIP, unsigned int requestID ) 
 * 		to check a RREQ with requestID from srcIP to dstIP is new or not 
 *
*/

#include "header.h"

#define MAX_RREQ_RECORD_NUM 1000
#define MAX_ID_RECORD_NUM 1000

static struct {
	IP_t dstIP ;
	IP_t srcIP ;
	unsigned int requestID ;
} RREQTable[ MAX_RREQ_RECORD_NUM ] ;

static struct {
	IP_t dstIP ;
	unsigned int requestID ;
} IDTable[ MAX_ID_RECORD_NUM ] ;

static int RREQTableIndex ; // need to insert new items into RREQTable
static int IDTableIndex ; // same as RREQTableIndex

void init_record_table(){
	int i ;
	for( i=0; i<MAX_RREQ_RECORD_NUM; i++ ) {
		RREQTable[i].dstIP = 0 ;
	}
	
	for( i=0; i<MAX_ID_RECORD_NUM; i++ ) {
		IDTable[i].dstIP = 0 ;
	}

	IDTableIndex = 0 ;
	RREQTableIndex = 0 ;
}

unsigned int get_ID( IP_t dstIP ) {
	int i ;
	unsigned int requestID ;

	unsigned int temp,temp1;
	srand(time(0));
	temp=rand()%256;
	temp1=time(0);
	temp1=temp1<<8;
	temp=temp+temp1;
	requestID=temp;

	RREQTable[ RREQTableIndex ].dstIP = dstIP ;
	RREQTable[ RREQTableIndex ].srcIP = hostIP ;
	RREQTable[ RREQTableIndex ++ ].requestID = requestID ;
	RREQTableIndex %= MAX_RREQ_RECORD_NUM ;

	return requestID ;
}

int check_new_RREQ( IP_t srcIP, IP_t dstIP, unsigned int requestID ) {
	int i ; 
	for( i=0; i<MAX_RREQ_RECORD_NUM; i++ ) {
		if(  RREQTable[i].srcIP == srcIP && RREQTable[i].dstIP == dstIP \
				&& RREQTable[i].requestID == requestID ) {
			return -1 ;
		}
	}

	RREQTable[ RREQTableIndex ].dstIP = dstIP ;
	RREQTable[ RREQTableIndex ].srcIP = srcIP ;
	RREQTable[ RREQTableIndex ++ ].requestID = requestID ;
	RREQTableIndex %= MAX_RREQ_RECORD_NUM ;

	return 0 ;
}
