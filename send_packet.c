/*
 * this file provide two interface to deal with sending packets
 *
 * void init_send_buffer()
 * 		set all the items in buffer to be unused ;
 *
 * int send_packet( IP_t dstIP, char *data,int data_len)
 * 		send out a message to dstIP ;
 *
 */

#include "header.h"
#define WAIT_HACK_TIME 1
#define WAIT_EACK_TIME 3

static int send_data(unsigned int no) ;

static void HTHACK(union sigval sig){
	unsigned int id=GET_ID_FROM_SIGVAL( sig );
	int i=0;
	ERROR_t ERRp;
	print(OUTPUT_ERROR,"waiting for HACK for packet in buffer %d and time out\n",id);
	ERRp.type=ERROR_FLAG;
	ERRp.addrNum=bufferData[id].data.addrNum;
	memcpy(ERRp.addr,bufferData[id].data.addr,sizeof(int)*MAX_HOP_NUM);
	for (i=0;i<ERRp.addrNum;i++){
		if (hostIP==ERRp.addr[i]){
			ERRp.index=i;break;
		}
	}
	delete_timer(bufferData[id].timers[1]);
	send_to_link_layer(hostIP,(char *)&ERRp,sizeof(ERROR_t));
}

void ETEACK(union sigval sig){
	unsigned int id=GET_ID_FROM_SIGVAL( sig );
	print(OUTPUT_ERROR,"waiting for EACK for packet in buffer %d and time out, will resend the data!\n",id);
	delete_timer(bufferData[id].timers[0]);
	send_data(id);
}

void ETERREP(union sigval sig)
{
	unsigned int id=GET_ID_FROM_SIGVAL( sig );
	int i;
	delete_timer(bufferData[id].timers[0]);
	if( find_path( bufferData[id].data.dstIP ) )  return ;
	print(OUTPUT_ERROR,"the RREP packet is time out and this transmission failed\n");
	for( i=0; i<MAX_BUFFER_NUM; i++ )
		if( bufferData[i].isUsed==1) {
			if (bufferData[i].data.srcIP==hostIP&&bufferData[i].data.dstIP==bufferData[id].data.dstIP) {
				bufferData[i].isUsed=0;
			}
		}
}

// init the bufferData and bufferNum ;
void init_send_buffer() {
	int i ;
	for( i=0; i< MAX_BUFFER_NUM; i++ ) {
		bufferData[i].isUsed = 0 ;
	}
}

// this function will invoke send_to_link_layer to send data packets
static int send_true_data( DATA *dataP) {
	int i;
	int returns;
	for(i=0;i<dataP->addrNum;i++) {
		if(hostIP==dataP->addr[i])
			break;
	}
	print( OUTPUT_LOG, "send_true_data: send a data packet to %s\n", ip_to_str(dataP->dstIP ) ) ;
	returns=send_to_link_layer(dataP->addr[i+1], (char *) dataP, sizeof(DATA));
	print(OUTPUT_LOG,"send over\n");
}

static int send_data(unsigned int no) {
	bufferData[no].timers[1]=start_timer(HTHACK, WAIT_HACK_TIME,no);
	print(OUTPUT_LOG,"the HACK timer for data buffer %u has been set up!",no);
	if(bufferData[no].data.srcIP==hostIP) {
		bufferData[no].timers[0]=start_timer(ETEACK, WAIT_EACK_TIME,no);
		print(OUTPUT_LOG,"the EACK timer for data buffer %d has been set up!",no);
	}
	bufferData[no].isUsed=2;
	return send_true_data(&bufferData[no].data);
}

int send_rreq( IP_t dstIP,unsigned int no) {
	RREQ *request = (RREQ*) malloc(sizeof(RREQ)) ;
	request->type=RREQ_FLAG;
	request->request_id = get_ID( dstIP ) ;
	request->dstIP = dstIP ;
	request->addrNum = 1 ;
	request->addr[0] = hostIP ;
	print( OUTPUT_LOG, "send_rreq: send a RREQ packet to %s\n", ip_to_str(  request->dstIP ) ) ;
	print( OUTPUT_LOG, "the packet has been saved in %d buffer\n", no) ;
	bufferData[no].timers[0]=start_timer(&ETERREP,3,no);
	print(OUTPUT_LOG,"the RREQ timer for data buffer %d has been set up!\n",no);
	return send_to_link_layer ( broadcastIP, (char *) request, sizeof( RREQ )) ;
}
// when sending out the RREQ. the message user input must be buffered first
static unsigned int save_to_buffer(DATA *datap){
	int i;
	for( i=0; i<MAX_BUFFER_NUM; i++ ) {
		if( bufferData[i].isUsed==0 ) {
			break ;
		}
	}
	if( i == MAX_BUFFER_NUM ) {
		print( OUTPUT_ERROR, "sendRreq ERROR: bufferData is FULL !!!\n\a" ) ;
		exit(1) ;
	} else {
		memcpy( &bufferData[i].data,datap,sizeof(DATA)) ;
		bufferData[i].isUsed = 1 ;
		return i;
	}
}

int send_packet( DATA * data){
	pathNode_t *pPath ;

	if( pPath=find_path( data->dstIP ) ) {
		print( OUTPUT_LOG, "send_packet: will send a DATA packet\n" ) ;
		data->addrNum = pPath->hopNum;
		memcpy(&data->addr,pPath->hops,sizeof(IP_t)*MAX_HOP_NUM);
		unsigned int no = save_to_buffer(data);
		return send_data(no) ;
	}else {
		print( OUTPUT_LOG, "send_packet: will send a RREQ packet\n" ) ;
		// buffer the message.
		unsigned int no = save_to_buffer(data) ;
		return send_rreq( data->dstIP,no ) ;
	}
}


int forward_data(DATA* data)
{
	print( OUTPUT_LOG, "send_packet: will send a DATA packet\n" ) ;
	unsigned int no=save_to_buffer(data);
	return send_data(no);
}

int send_packet_afterRREP(IP_t srcIP,IP_t dstIP){
	pathNode_t *pPath;
	if (pPath=find_path(dstIP)){
		int i;
		for( i=0; i<MAX_BUFFER_NUM; i++ ) {
			if( bufferData[i].isUsed==1) {
				if (bufferData[i].data.srcIP==hostIP&&bufferData[i].data.dstIP==dstIP) {
					print( OUTPUT_LOG, "send_packet_afterRREP: will send a DATA packet\n" ) ;
					bufferData[i].data.addrNum = pPath->hopNum;
					memcpy(&bufferData[i].data.addr,pPath->hops,sizeof(int)*MAX_HOP_NUM);
					delete_timer(bufferData[i].timers[0]);
					send_data(i);
				}
			}
		}
	}
}
