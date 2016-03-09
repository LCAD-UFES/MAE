#ifndef JOBS_DATA_H
#define JOBS_DATA_H

#include <stdlib.h>
#include <stdio.h>

#define CONF_FILE_NAME "tagger.cfg"

typedef struct _beliefs_vector
{
	float *beliefs;
	struct _beliefs_vector *next;
} BELIEFS_VECTOR;

typedef struct
{
	int w_before;
	int w_after;
	int tagset_size;
	BELIEFS_VECTOR *biliefs_vec;	
} JOB_DATA;

// global variable that is cloned for each thread
// so there is one for each thread
extern JOB_DATA *g_job_data;

// Functions prototipes

void read_tagger_parameters( int *w_before, int *w_after, int *tagset_size );

JOB_DATA *create_job_data( int w_before, int w_after, int tagset_size );

void free_beliefs_vector( int num_vectors, BELIEFS_VECTOR *biliefs_vec );

void free_jobs_data( JOB_DATA *jd );

void complete_ttv( JOB_DATA *job_data, float *ttv_vector );

void update_wnn_outputs( JOB_DATA *r_data );

#endif
