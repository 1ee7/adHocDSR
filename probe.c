//
//  Send and Receive Probe.c
//  ETX 计算每15s的平均值
//  每个host每0.5秒发送一个probe
//
//  Created by Emilydelluque on 16/8/5.
//
//  Modified by Cui on 16/8/8

/*
 * this file provide one interface to others :
 * void init_Probetimer () : to start the probe process ;
 * void receive_probe( PROBE_t * packet) ;
 */

#include "header.h"

#define MAX_NEIGH_NUM 10
#define ETX_INFINITY 1000000

static int neighNum=0;
static timer_t timerID ;

static struct{
    IP_t neighIP;
    unsigned int head ;
    unsigned int rNum[30];
    unsigned int fNum ;
    ETX_t ETX; // 记录和每一个neighbor的ETX
} ETXlist[ MAX_NEIGH_NUM ] ;

static int send_probe (){ // Broadcast发送probe.
    PROBE_t  probe;
    memset(&probe,0, sizeof(probe));

    probe.type = PROBE_FLAG;
    probe.srcIP = hostIP ;
    probe.neighNum = neighNum ;

    int i=0;
    for (i=0; i<neighNum; i++){
        probe.etx[i].neighIP= ETXlist[i].neighIP;
        probe.etx[i].rNum= ETXlist[i].rNum[0];
    }
    return send_to_link_layer (broadcastIP, (char *) &probe, sizeof(probe));
}

void receive_probe(PROBE_t * p){
    // receive: 在 ETXlist当中找到和probe当中srcIP相同的neighIP, 把该neighbor的前15秒内的Receive都加上1
    int i;
    for (i=0; i<neighNum; i++){
        if (ETXlist[i].neighIP == p->srcIP) {
            int j ;
            for( j=0; j<30; j++) ETXlist[i].rNum[j]++;
            break ;
        }
    }

    if (i== neighNum){
        //如果是第一次收到这个neighbor的，在ETXlist当中添加该neighbor， 并且把fNum表格置成1
        ETXlist[neighNum++].neighIP = p->srcIP;
        int j ;
        for (j=0; j<30; j++) ETXlist[i].rNum[j]=1;
    }

    // forward: 在收到的probe当中找到对应的hostIP, 用probe当中关于host的forward信息更新frNum[0]前15s(30次）内的所有信息 （加上probe当中传递的值);如果没有及时收到来自neighbor的probe, 更新的时候就会使用原来的数值
    int j ;
    for (j=0; j< p->neighNum; j++){
        if (hostIP == p->etx[j].neighIP){
            ETXlist[i].fNum= p->etx[j].rNum;
			break ;
        }
    }

    if( j== p->neighNum ) {
        print( OUTPUT_ERROR, " receive_probe: there is no host in the received probe packet") ;
    }
}

// 计算每一个host当中，关于每一个neighbor的ETX的值
// 过去15s(30次）的关于一个特定的neighbor的forward和receive到的probe的个数:rNum[]为forward, frNum为recieve
static void cal_ETX(){ // calculate ETX 并且更新rNum表
    int i=0;
    for (i=0; i<neighNum; i++){
        int numf = ETXlist[i].rNum[ ETXlist[i].head ];
        int numr = ETXlist[i].fNum;
		if( numf == 0 || numr == 0 ) {
			ETXlist[i].ETX = ETX_INFINITY ;
		}else {
			ETXlist[i].ETX = ( unsigned int ) (30*30/(numf * numr)) ;
		}
    //    ETXlist[i].ETX = 10 ;

        // 利用FIFO, 把rNum数组往前移动,数组的末尾置成0
        ETXlist[i].rNum[ ETXlist[i].head ] = 0 ;
        ETXlist[i].head ++ ;
        ETXlist[i].head %= MAX_NEIGH_NUM ;

    }
}

static void PROBE(union sigval sig){
    cal_ETX();

#ifdef __TEST_ETX_CALCULATE___
    int i;
    for (i=0; i<MAX_NEIGH_NUM); i++){
        printf("%3f, ", ETXlist[i].ETX);
    } // 把etx全部打印下来
#endif

    send_probe();

    probeInterval_t interval;
    int temp;
    srand((unsigned)(time(NULL)) );
    temp = rand()%200;
    interval.nSec = 1;
    interval.nMSec = 400 +temp ;

    delete_timer (timerID);
    timerID=start_probe_timer(PROBE, interval);
}

void init_Probetimer (){
    neighNum = 0 ;
    int i;
    for (i = 0; i < MAX_NEIGH_NUM; i++){
    	ETXlist[i].head = 0 ;
    	int j ;
    	for(j=0; j<30; j++ ){
    	    ETXlist[i].rNum[j] = 30;
    	}
    	ETXlist[i].fNum = 30 ;
    }

    probeInterval_t interval;
    int temp;
    srand((unsigned)(time(NULL)));
    temp = rand()%200;
    interval.nSec = 1;
    interval.nMSec = 400 +temp ;
    timerID = start_probe_timer(PROBE, interval);
}

ETX_t get_etx( IP_t neighIP ) {
    int i ;
    for( i=0; i<MAX_NEIGH_NUM; i++ ){
        if( ETXlist[i].neighIP == neighIP ) {
            return ETXlist[i].ETX ;
        }
    }

    if( i== MAX_NEIGH_NUM ) {
        print( OUTPUT_ERROR, "get_etx: no etx for %s\n", ip_to_str( neighIP )) ;
    }
}

void print_etx( ) {
    printf( "\n\n CUI ETX \n") ;
    int i ;
    for( i=0; i<MAX_NEIGH_NUM; i++ ) {
        printf( "IP %s; ETX %u i\n", ip_to_str( ETXlist[i].neighIP), ETXlist[i].ETX) ;
    }
    printf( "\n CUI ETX END \n\n") ;
}
