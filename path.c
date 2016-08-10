/*
 * this file provides four functions to deal with path cache
 *
 * void init_path_cache()
 * 		to initialze the cache table to all zeros ;
 *
 * int insert_path( pathNode_t * pPathNode )
 * 		to insert a path into the pathCache table ;
 *
 * pathNode_t * find_path( IP_t dstIP )
 * 		to find ONE path to dstIP ;
 *
 * void delete_path( IP_t dstIP )
 * 		to delete ALL paths to dstIP ;
 *
*/

#include "header.h"

#define MAX_PATH_CACHE_NUM 1000
#define EXPIRE_TIME_LENGTH pathExpireTime

// pathCache table
static pathNode_t pathCache[ MAX_PATH_CACHE_NUM ] ;

void init_path_cache() {
	int i ;
	for( i=0; i<MAX_PATH_CACHE_NUM; i++ ) {
		pathCache[i].dstIP = 0 ;
	}

}

int insert_path( pathNode_t * pPathNode ) {
	printf( "\n\nCUI::::  ETX: %u\n\n", pPathNode->etxTotal ) ;
	int i ;
	for( i=0; i<MAX_PATH_CACHE_NUM; i++ ) {
		if( pathCache[i].dstIP == 0 ) {
			break ;
		}
		time_t now ;
		time( &now ) ;
		if( pathCache[i].expireTime < now ) {
			break ;
		}
	}
	if( i == MAX_PATH_CACHE_NUM ) {
		print( OUTPUT_ERROR, "insert_path: The pathCache is full \a\n" ) ;
		return  0 ;
	}

	memcpy( pathCache+i, pPathNode, sizeof( pathNode_t ) ) ;

	time( &( pathCache[i].expireTime ) ) ;
	pathCache[i].expireTime += pathExpireTime ;

	return 1 ;
}

pathNode_t * find_path( IP_t dstIP ) {
	if( dstIP == 0 ) return NULL ;

	int i ;

	time_t now;
	time( &now ) ;
	for( i=0; i<MAX_PATH_CACHE_NUM; i++ ) {
		if( pathCache[i].dstIP == dstIP ) {

			if( now > pathCache[i].expireTime ) {
				pathCache[i].dstIP = 0 ;
				continue ;
			}

			return pathCache+i ;
		}
	}

	return NULL ;
}

void delete_path( IP_t dstIP ) {
	int i ;
	for( i=0; i<MAX_PATH_CACHE_NUM; i++ ) {
		if( dstIP == pathCache[i].dstIP ) {
			pathCache[i].dstIP = 0 ;
		}
	}
}
