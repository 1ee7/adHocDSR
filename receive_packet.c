/*
 * this file deals with receiving packets, it provides one interface
 *
 * void receive_packet(char *packet, int packet_len)
 * 		doing what DSR requires when receiving the packet
 *
 */

#include "header.h"

static void receive_eack(EACK_t * eackP);
static void receive_error(ERROR_t * errorP);
static void receive_hack(HACK_t * hackP);

void RREQWINDOW(union sigval sig){
	unsigned int id=GET_ID_FROM_SIGVAL( sig );
	delete_timer(bufferRREQ[id].timerw);
	bufferRREQ[id].isUsed=0;
	print(OUTPUT_LOG,"receive rreq: it ends up to be %d pathes\n",bufferRREQ[id].multi_path_num);
	return;
}


void init_rreq_buffer()
{
	int i;
	for(i=0;i< MAX_RREQBUFFER_NUM;i++)
	{
		bufferRREQ[i].isUsed=0;
	}
	return;
}

int get_unused_rreqbuf()
{
	int i;
	for(i=0;i< MAX_RREQBUFFER_NUM;i++)
	{
		if(bufferRREQ[i].isUsed==0) break;
	}
	if(i<MAX_RREQBUFFER_NUM)
		return i;
	else
	{
		print(OUTPUT_ERROR,"not enough rreq buffer!\n");
		exit(2);
	}
}

int check_rreq_buf(IP_t srcIP,int request_id)
{
	int i;
	for(i=0;i< MAX_RREQBUFFER_NUM;i++)
	{
		if(bufferRREQ[i].isUsed==1)
			if(bufferRREQ[i].request_id==request_id&&bufferRREQ[i].srcIP==srcIP)
			{
				bufferRREQ[i].multi_path_num++;
				return 1;
			}
	}
	return 0;
}


void RREPWINDOW(union sigval sig){
	unsigned int id=GET_ID_FROM_SIGVAL( sig );
	delete_timer(bufferRREP[id].timerw);
	bufferRREP[id].isUsed=0;
	print( OUTPUT_LOG, "receive_rrep: receive %d RREPs for host in 1sec, will resend the cached data not sent yet\n",bufferRREP[id].multi_path_num) ;
	send_packet_afterRREP(hostIP,bufferRREP[id].dstIP);
	return;
}


void init_rrep_buffer()
{
	int i;
	for(i=0;i< MAX_RREPBUFFER_NUM;i++)
	{
		bufferRREP[i].isUsed=0;
	}
	return;
}

int get_unused_rrepbuf()
{
	int i;
	for(i=0;i< MAX_RREPBUFFER_NUM;i++)
	{
		if(bufferRREP[i].isUsed==0) break;
	}
	if(i<MAX_RREPBUFFER_NUM)
		return i;
	else
	{
		print(OUTPUT_ERROR,"not enough rrep buffer!\n");
		exit(2);
	}
}

int check_rrep_buf(IP_t dstIP)
{
	int i;
	for(i=0;i< MAX_RREPBUFFER_NUM;i++)
	{
		if(bufferRREP[i].isUsed==1)
			if(bufferRREP[i].dstIP==dstIP)
			{
				bufferRREP[i].multi_path_num++;
				return 1;
			}
	}
	return 0;
}

int data_remain(IP_t dstIP)
{
	int i;
	for( i=0; i<MAX_BUFFER_NUM; i++ )
	if( bufferData[i].isUsed==1) {
		if (bufferData[i].data.srcIP==hostIP&&bufferData[i].data.dstIP==dstIP) {
			return 1;
		}
	}
	return 0;
}

static void receive_data( DATA* dataP ) {
	int addrNum = dataP->addrNum;
    int data_len = dataP->data_len;
	int dstIP = dataP->dstIP ;
    int i ;
    for( i=0; i<addrNum; i++ ) {
		if( hostIP == dataP->addr[i] ) {
			break ;
		}
    }
    if( i == addrNum ) {
        print( OUTPUT_ERROR,"receive_data: recevice a data, but cannot find host in the path\n" ) ;
    }
	else {
		HACK_t *hack=(HACK_t *)malloc(sizeof(HACK_t));
		hack->type=HACK_FLAG;
		// inverse?????
		hack->srcIP=dataP->srcIP;
		hack->dstIP=dataP->dstIP;
		hack->id = dataP->id ;
		print( OUTPUT_LOG, " A HACK for %s is sent\n", ip_to_str( dataP->addr[i-1] )) ;
     	send_to_link_layer( dataP->addr[i-1], (char *)hack, sizeof(HACK_t) );
		free(hack);

		if( dstIP == hostIP ) {
			EACK_t *eack=(EACK_t *)malloc(sizeof(EACK_t));
			eack->id=dataP->id;
			eack->srcIP=dataP->dstIP;
			eack->dstIP=dataP->srcIP;
			eack->addrNum=dataP->addrNum;
			memcpy(eack->addr,dataP->addr,eack->addrNum*sizeof(int));
			eack->type=EACK_FLAG;
			receive_interact( dataP->data ) ;
			send_to_link_layer(eack->addr[eack->addrNum-2],(char *)eack,sizeof(EACK_t));
			free(eack);
			return ;
		} else {
			forward_data(dataP);
		}
	}
}

static void receive_rrep( RREP * replyP) {
	int i;
	int path_inserted=0;
	for(i=0;i<replyP->addrNum;i++)
		if(replyP->addr[i]==hostIP)	break;

	if( i == replyP->addrNum ) {
		print( OUTPUT_ERROR, "receive_rrep: no hostIP in addrs\n" ) ;
		return ;
	}

	replyP->etx[i] = replyP->etx[i+1] + get_etx( replyP->addr[i+1] ) ;
	printf( "\n\n CUI RREP \n") ;
	printf( "%u\n", replyP->etx[i] );
	char * p = ( char *) replyP ;
	int j ;
	for( j=0; j<sizeof( RREP ); j++ ){
		printf("%2x ;", p[j] ) ;
	}
	printf( "\nCUI RREP END\n\n");
	/* orig
	pathNode_t path ;
	path.dstIP  = replyP->addr[replyP->addrNum-1] ;// not inverse!
	path.hopNum = replyP->addrNum-i;
	memcpy(path.hops,replyP->addr+i,path.hopNum*sizeof(int));
	path_inserted=insert_path( &path ) ;
	*/
	pathNode_t * pPath = find_path( replyP->addr[ replyP->addrNum -1] ) ;
	if( pPath == NULL ) {
		pathNode_t path ;
		path.dstIP  = replyP->addr[replyP->addrNum-1] ;// not inverse!
		path.hopNum = replyP->addrNum-i;
		path.etxTotal = replyP->etx[i] ;
		memcpy(path.hops,replyP->addr+i,path.hopNum*sizeof(IP_t));
		path_inserted=insert_path( &path ) ;
		pPath = &path ;
	} else {
		path_inserted = 1 ;
		if( pPath->etxTotal > replyP->etx[i] ) {
			pPath->hopNum = replyP->addrNum-i;
			pPath->etxTotal = replyP->etx[i] ;
			memcpy(pPath->hops,replyP->addr+i,pPath->hopNum*sizeof(IP_t));
		}
	}
	IP_t dstIP = pPath->dstIP ;
	print(OUTPUT_LOG,"receive rrep:the insert path, dstIP:%s\n",ip_to_str(dstIP));
	if( replyP->dstIP == hostIP ) {
		if(check_rrep_buf(dstIP)==0) //
		{ // interesting part
			if(path_inserted==1&&data_remain(dstIP)==1){
				int rrep_buf_index=get_unused_rrepbuf();
				bufferRREP[rrep_buf_index].dstIP=dstIP;
				bufferRREP[rrep_buf_index].isUsed=1;
				bufferRREP[rrep_buf_index].timerw=start_timer(RREPWINDOW,1,rrep_buf_index);
				bufferRREP[rrep_buf_index].multi_path_num=1;
			}
		}
	}else{
		send_to_link_layer ( replyP->addr[i-1], (char *) replyP, sizeof(RREP) ) ;
	}
}

static void receive_rreq( RREQ * pRreq) {
	pathNode_t *path ;
	int i ;
	int is_new;
	int rreq_buf_index;
	is_new=check_new_RREQ( pRreq->addr[0], pRreq->dstIP, pRreq->request_id );
	if( pRreq->dstIP == hostIP ) {
		if(is_new==0)// if the rreq is a new one; buf it to open a window to accept multiple rreq
		{
			rreq_buf_index=get_unused_rreqbuf();
			bufferRREQ[rreq_buf_index].isUsed=1;
			bufferRREQ[rreq_buf_index].srcIP=pRreq->addr[0];
			bufferRREQ[rreq_buf_index].request_id=pRreq->request_id;
			bufferRREQ[rreq_buf_index].timerw=start_timer(RREQWINDOW,1,rreq_buf_index);
			bufferRREQ[rreq_buf_index].multi_path_num=1;
			print(OUTPUT_LOG,"open a window long for 1sec to receive multiple RREQ!");
		}
		else
		{
			if(check_rreq_buf(pRreq->addr[0],pRreq->request_id)==0)//not in the receive window;
			{
				return;
			}

		}
		pRreq->addr [ pRreq -> addrNum ++ ] = hostIP ;

		RREP *pRrep = (RREP *)malloc(sizeof(RREP)) ;
		pRrep->type=RREP_FLAG;
		pRrep->dstIP = pRreq->addr[0] ;
		pRrep->addrNum = pRreq->addrNum ;
		pRrep->etx[ pRrep->addrNum-1 ] = 0 ;
		memcpy(pRrep->addr,pRreq->addr,pRrep->addrNum*sizeof(IP_t));

		print( OUTPUT_LOG, "receive_rreq: received a new RREQ from %s, will send a RREP to nextHop %s\n",\
				ip_to_str( pRreq->addr[0] ), ip_to_str( pRrep->addr[pRrep->addrNum-2] ) ) ;

		send_to_link_layer ( pRrep->addr[pRrep->addrNum-2], ( char * ) pRrep, sizeof( RREP)  ) ;
		free(pRrep);
	} else {
		if(is_new==-1) return;
		print( OUTPUT_LOG, "receive_rreq: received a new RREQ from %s looking for %s\n",\
				ip_to_str( pRreq->addr[0] ), ip_to_str( pRreq->dstIP ) ) ;
		if( path=find_path( pRreq->dstIP ))
		{
			print(OUTPUT_LOG,"receive_rreq: There exists a path to %s\n",ip_to_str(pRreq->dstIP));
			pRreq->addr [ pRreq -> addrNum ++ ] = hostIP ;

			RREP *pRrep = (RREP *)malloc(sizeof(RREP)) ;
			pRrep->type=RREP_FLAG;
			pRrep->dstIP = pRreq->addr[0] ;
			pRrep->addrNum = pRreq->addrNum+path->hopNum-1 ;
			memcpy(pRrep->addr,pRreq->addr,pRreq->addrNum*sizeof(IP_t));
			memcpy(pRrep->addr+pRreq->addrNum,path->hops+1,(path->hopNum-1)*sizeof(IP_t));

			print( OUTPUT_LOG, "receive_rreq: will send a RREP to nextHop %s\n", ip_to_str( pRrep->addr[pRreq->addrNum-2] ) ) ;

			send_to_link_layer ( pRrep->addr[pRreq->addrNum-2], ( char * ) pRrep, sizeof( RREP)  ) ;
			free(pRrep);
		}else{
			pRreq->addr [ pRreq -> addrNum ++ ] = hostIP ;
			send_to_link_layer ( broadcastIP, (char *)pRreq,sizeof(RREQ)) ;
		}
	}
	return;
}
static void receive_hack(HACK_t * hackP)
{
	int i;
	print(OUTPUT_LOG,"receive hack!\n");
	for( i=0; i<MAX_BUFFER_NUM; i++ )
	if( bufferData[i].isUsed==2) {
		if (bufferData[i].data.srcIP==hackP->srcIP&&bufferData[i].data.dstIP==hackP->dstIP&&\
			bufferData[i].data.id==hackP->id) {
			break;
		}
	}
	if(hostIP!=hackP->srcIP)
		bufferData[i].isUsed=0;//此包成功发给了下一个人，从buffer中清除
	delete_timer(bufferData[i].timers[1]);
}
static void receive_eack(EACK_t * eackP)
{
	if( eackP->dstIP == hostIP ) {
		int i;
		print( OUTPUT_LOG, "receive_eack: receive a EACK for host\n" ) ;
		//close the buffer corresponds to the eack;

		for( i=0; i<MAX_BUFFER_NUM; i++ )
			if( bufferData[i].isUsed == 2 ) {
				if (bufferData[i].data.srcIP==eackP->dstIP&&\
					bufferData[i].data.dstIP==eackP->srcIP&&bufferData[i].data.id==eackP->id) {
					break;
				}
			}
		bufferData[i].isUsed=0;
		delete_timer(bufferData[i].timers[0]);
	} else {
		int i ;
		for(i=0;i<eackP->addrNum;i++)
		{
			if(eackP->addr[i]==hostIP)
			{
				break;
			}
		}
		if( i == eackP->addrNum ) {
			print( OUTPUT_ERROR, "receive_eack: no hostIP in addrs\n" ) ;
			return ;
		}

		print( OUTPUT_LOG, "receive_eack: receive a EACK for %s, will send it to %s \n", ip_to_str( eackP->dstIP ), ip_to_str( eackP->addr[i-1] ) ) ;
		send_to_link_layer ( eackP->addr[i-1], (char *) eackP, sizeof(EACK_t) ) ;
	}
}
static void receive_error(ERROR_t * errorP)
{
	int i,j;
	int first=-1;
	//路径上各点对error包反应，并只删除到dst端的路径
	for(j=0;j<errorP->addrNum;j++)
		if(errorP->addr[j]==hostIP)
			break;
	if(errorP->addr[0]==hostIP)//源端清楚此路径上所有包等待EACK的计时器并进入重发阶段
	{
		delete_path(errorP->addr[errorP->addrNum-1]);
		for( i=0; i<MAX_BUFFER_NUM; i++ )
			if( bufferData[i].isUsed==2) {
				if (bufferData[i].data.srcIP==hostIP&&\
					bufferData[i].data.dstIP==errorP->addr[errorP->addrNum-1]) {

					delete_timer(bufferData[i].timers[0]);
					bufferData[i].isUsed=1;
					memset(bufferData[i].data.addr,0,bufferData[i].data.addrNum);
					bufferData[i].data.addrNum=0;
					if(first==-1)	first=i;
				}
			}
		//usleep(1000*1000*20);
		if( first != -1 ) {
			send_rreq(errorP->addr[errorP->addrNum-1],first);
		}
		//只需要rreq引回来rrep一次，所有同目的地的包都会被重发
	}
	else
	{
		send_to_link_layer( errorP->addr[j-1], (char *) errorP, sizeof(ERROR_t) ) ;
	}
}
void receive_packet(char *packet, int packet_len){
	char Packet_Type=packet[0];
	switch( Packet_Type) {
		case DATA_FLAG :
			receive_data( (DATA *)packet);
			break ;
		case RREP_FLAG :
			receive_rrep( (RREP *)packet);
			break ;
		case RREQ_FLAG :
			receive_rreq( (RREQ *)packet);
			break ;
		case HACK_FLAG :
			receive_hack( (HACK_t *)packet);
			break;
		case EACK_FLAG :
			receive_eack( (EACK_t *)packet);
			break;
		case ERROR_FLAG :
			receive_error( (ERROR_t *)packet);
			break;
		case PROBE_FLAG :
			receive_probe( (PROBE_t *) packet ) ;
			break ;
		default :
			print( OUTPUT_ERROR, "receieve_packet ERROR: Wrong packetType" ) ;
			exit ( 2 ) ;
	}
}
