#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"

/* Global variables */

char **g_argv;
int g_argc;

TRAIN_DATA_SET *g_train_data_set = NULL;
int g_train_num_samples = 0;
char g_current_data_set_name[1000];

int FIRST_SAMPLE	= 0;
int LAST_SAMPLE 	= 1;
int g_TrainWeekNumber = 0;

TT_DATA_SET **g_tt_data_set = NULL;  // train and test data set
int g_tt_num_samples = 0;
int g_no_extra_predicted_sample = 1;

float g_price_samples[MINUTES_IN_A_WEEK];
int g_moving_average_current_minute = -1;

int g_moving_average_first_day = 0;

int g_trading_system_current_week;

int g_target_day_index;

int g_stop_trading_system = 0;

char *g_state_string[] = 
{
	"INITIALIZE",
	"WAIT_BEGIN_WEEK",
	"SELECT_STOCK",
	"WAIT_MOVING_AVERAGE",
	"WAIT_MIN_TO_BUY",
	"SEND_BUY_ORDER",
	"WAIT_BUY",
	"WAIT_BUY_CANCEL",
	"WAIT_MAX_TO_SELL",
	"SEND_SELL_ORDER",
	"WAIT_SELL",
	"WAIT_SELL_CANCEL",
	"QUIT",
	"INVALID_STATE"
};



/*
*********************************************************************************
* Print error and terminate							*
*********************************************************************************
*/

void
Erro(char *msg1, char *msg2, char *msg3)
{
#ifndef	USE_IPC	
	fprintf(stderr, "Error: %s%s%s\n", msg1, msg2, msg3);
	exit(1);
#else
	char description[1000];
	time_t now;
	struct tm *brokentime;

	fprintf(stderr, "Error: %s%s%s\n", msg1, msg2, msg3);
	
	sprintf(description, "Error: %s%s%s\n", msg1, msg2, msg3);
	now = time(NULL);
	brokentime = localtime(&now);
	publish_control_message(1, description, asctime(brokentime), TS_NAME);
#endif
}



/*
*********************************************************************************
* Alloc memory at building time							*
*********************************************************************************
*/

void *
alloc_mem(size_t size)
{    
	void *pointer;

	if ((pointer = malloc(size)) == (void *) NULL)
		Erro("cannot alloc more memory.", "", "");
	return (pointer);
}
