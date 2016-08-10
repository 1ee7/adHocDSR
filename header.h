#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <math.h>
#include <errno.h>

#define __TEST_DEBUG__  		// put all the packet send or receive

#define IPSTR_LENGTH 20
#define MAX_DATA_LENGTH 100
#define MAX_PACKET_LENGTH 500
#define MAX_HOP_NUM 10
#define MAX_NEIGH_NUM 10
typedef unsigned int IP_t ;
typedef unsigned int ETX_t;
typedef unsigned int asize_t ;

// For timer
#define GET_ID_FROM_SIGVAL( sig ) ( (sig).sival_int )
typedef void (* timerFunc_t )( union sigval val )  ;
timer_t start_timer( timerFunc_t timerFunc, unsigned int nSec, unsigned int id);
typedef struct {
    unsigned int nSec ;
    unsigned int nMSec ;
} probeInterval_t ;
timer_t start_probe_timer( timerFunc_t timerFunc, probeInterval_t interval );
void delete_timer( timer_t timerID ) ;

// For output
#define LOCK_OUTPUT do{ \
   						while( outputLock ) usleep( 0 ) ; \
						outputLock = 1 ;\
					}while( 0 )
#define FREE_OUTPUT do{ \
						outputLock = 0 ; \
					}while(0 )

// different types of output
typedef enum {
	OUTPUT_ERROR, OUTPUT_LOG, OUTPUT_INFO, OUTPUT_ELSE
} output_t ;

// to seperate the output of receive thread from the output of main thread
char outputLock ;

// For path expire
unsigned int pathExpireTime ;

IP_t hostIP, broadcastIP ;
char hostIPStr[ IPSTR_LENGTH ] ;

#pragma pack(1)

// Route request
typedef struct{
	char type;
    IP_t dstIP;
    asize_t addrNum;	//The number of address which make sense in the following array
    IP_t addr[MAX_HOP_NUM];
    int request_id;
}RREQ;

//Route reply
typedef struct{
	char type;
	IP_t dstIP;
    asize_t addrNum;
    IP_t addr[MAX_HOP_NUM];
	ETX_t etx[ MAX_HOP_NUM ] ;
}RREP;

//Data packet
typedef struct{
	char type;
	unsigned int id;
	unsigned int srcIP;
	unsigned int dstIP;
    asize_t addrNum;
    IP_t addr[MAX_HOP_NUM];
	int data_len;
	char data[MAX_DATA_LENGTH];
}DATA;

typedef struct {
	char type;
	unsigned int id ;
	IP_t srcIP ;
	IP_t dstIP ;
}HACK_t;

typedef struct {
	char type;
	unsigned int id ;
	IP_t srcIP ;
	IP_t dstIP ;
	asize_t addrNum ;
	IP_t addr[ MAX_HOP_NUM ] ;
}EACK_t;

typedef struct {
	char type;
	asize_t addrNum ;
	IP_t addr[ MAX_HOP_NUM ] ;
	asize_t index ;
}ERROR_t;

typedef struct {
    IP_t neighIP ;
    unsigned int rNum ;
}probeETX_t ;

typedef struct {
    char type ;
	IP_t srcIP ;
	asize_t neighNum ;
	probeETX_t etx[ MAX_NEIGH_NUM ] ;
}PROBE_t ;


#define RREQ_FLAG 1
#define RREP_FLAG 2
#define DATA_FLAG 3
#define HACK_FLAG 4
#define EACK_FLAG 5
#define ERROR_FLAG 6
#define PROBE_FLAG 7


// For path cache
typedef struct {
	IP_t dstIP ;
	asize_t hopNum ;
	IP_t hops[ MAX_HOP_NUM ] ;
	time_t expireTime ;
    ETX_t etxTotal ;
} pathNode_t ;

// For data buffer
#define MAX_BUFFER_NUM 1000
typedef struct {
	int isUsed;
	DATA data;
	timer_t timers[2];//0: RREP or E2EACK timer ,1:H2HACK timer;
} buffer_t ;

buffer_t bufferData[ MAX_BUFFER_NUM ] ;

#define MAX_RREQBUFFER_NUM 10
typedef struct{
	char isUsed;
	IP_t srcIP;
	int request_id;
	timer_t timerw;
	int multi_path_num;
} rreq_buffer_t;

rreq_buffer_t bufferRREQ[ MAX_RREQBUFFER_NUM];

#define MAX_RREPBUFFER_NUM 10
typedef struct{
	char isUsed;
	IP_t dstIP;
	timer_t timerw;
	int multi_path_num;
} rrep_buffer_t;
rrep_buffer_t bufferRREP[ MAX_RREPBUFFER_NUM];

//packet buffer
#define MAX_BUFFER_NUM 1000
#define NOT_USED 0
//is source
#define TO_SEND 1
#define WAIT_RREP 2
#define WAIT_HACK 3
#define WAIT_EACK 4

//is relay
#define R_TO_SEND 11
#define R_WAIT_HACK 12

void init_path_cache() ;

int insert_path( pathNode_t * ) ;

pathNode_t * find_path( IP_t ) ;

void delete_path( IP_t dst ) ;

// For send packets
void init_send_buffer() ;
int send_packet(  DATA * data ) ;
int forward_data(DATA* data) ;
int send_packet_afterRREP(IP_t srcIP,IP_t dstIP) ;
int send_rreq( IP_t dstIP,unsigned int no) ;


// For receive packets
void receive_packet( char *segment, int seg_len ) ;
void init_rreq_buffer() ;
void init_rrep_buffer() ;


// For link_layer
int send_to_link_layer ( IP_t nextHopIP, char *segment, asize_t seg_len ) ;

void init_sockets() ;

void * receive_from_link_layer( void * noUse ) ;

// For interaction

void send_interact() ;
void receive_interact() ;


// For request ID

void init_record_table() ;

unsigned int get_ID( IP_t dstIP ) ;

int check_new_RREQ( IP_t srcIP, IP_t dstIP, unsigned int requestID ) ;

// two useful( maybe not ) functions

void print( output_t output, char *fmt, ... ) ;

char *ip_to_str( IP_t IP )  ;

// probe
void receive_probe(PROBE_t * p) ;
ETX_t get_etx( IP_t neighIP ) ;
void init_Probetimer () ;
