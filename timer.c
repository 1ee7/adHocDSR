#include "header.h"

timer_t start_timer( timerFunc_t timerFunc, unsigned int nSec, unsigned int id ) {

    /* Variable Definition */
    struct sigevent ent;
    struct itimerspec value;
	timer_t timerID ;
    /* Init */
    memset(&ent, 0x00, sizeof(struct sigevent));
    memset(&value, 0x00, sizeof(struct itimerspec));
    /* create a timer */
    ent.sigev_notify = SIGEV_THREAD;
    ent.sigev_notify_function = timerFunc ;
	ent.sigev_notify_attributes = NULL ;
	ent.sigev_value.sival_int = id ;
    if( timer_create(CLOCK_REALTIME, &ent, &timerID ) == -1 ) {
		print(OUTPUT_ERROR, "ERROR timer_create \n" ) ;
		exit( 1 ) ;
	}
    /* start a timer */
    value.it_value.tv_sec = nSec ;
    value.it_value.tv_nsec = 0;
    if( timer_settime(  timerID, 0, &value, NULL) ) {
		print(OUTPUT_ERROR, "ERROR timer_settime \n" ) ;
		exit( 1 ) ;
	}

	return timerID ;
}

timer_t start_probe_timer( timerFunc_t timerFunc, probeInterval_t interval ){ 
    struct sigevent ent;
    struct itimerspec value;
	timer_t timerID ;

    memset(&ent, 0x00, sizeof(struct sigevent));
    memset(&value, 0x00, sizeof(struct itimerspec));

    ent.sigev_notify = SIGEV_THREAD;
    ent.sigev_notify_function = timerFunc ;
	ent.sigev_notify_attributes = NULL ;
    if( timer_create(CLOCK_REALTIME, &ent, &timerID ) == -1 ) {
		print(OUTPUT_ERROR, "ERROR timer_create \n" ) ;
		exit( 1 ) ;
	}

    value.it_value.tv_sec = interval.nSec ;
    value.it_value.tv_nsec = 1000 * interval.nMSec ;
    if( timer_settime(  timerID, 0, &value, NULL) ) {
		print(OUTPUT_ERROR, "ERROR timer_settime \n" ) ;
		exit( 1 ) ;
	}

	return timerID ;
}

void delete_timer( timer_t timerID ) {
	timer_delete( timerID ) ;
}
