#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <specific_core_rpcs_handling_functions.h>

#include "jobs_data.h"

// global variable that is cloned for each thread
// so there is one for each thread
// extern JOB_DATA *g_job_data;

// extern PREDICTED_CLASS_AND_BELIEF* g_predicted_classes_and_beliefs;
// extern int g_predicted_classes_and_beliefs_size;

// void read_tagger_parameters( int *w_before, int *w_after, int *tagset_size )
// {
// 	int num_pars;
// 	char buffer[256];
// 	FILE *conf_file;
// 	
// 	if( (conf_file = fopen( CONF_FILE_NAME, "r" )) == NULL )
// 	{
// 		fprintf( stderr, "Couldn't open conf file %s in 'r' mode.\n", CONF_FILE_NAME );
// 		exit(0);
// 	}
// 	
// 	num_pars = 0;
// 	
// 	while( !feof( conf_file ) && num_pars < 3 )
// 	{
// 		fscanf( conf_file, "%s", buffer );
// 		
// 		if( buffer[0] == '#' )
// 		{
// 			fscanf( conf_file, "%[^\n]\n", buffer );
// 		}
// 		else if( strcmp( buffer, "w_before" ) == 0 )
// 		{
// 			fscanf( conf_file, "%d", w_before );
// 			num_pars++;
// 		}
// 		else if( strcmp( buffer, "w_after" ) == 0 )
// 		{
// 			fscanf( conf_file, "%d", w_after );
// 			num_pars++;
// 		}
// 		else if( strcmp( buffer, "tagset_size" ) == 0 )
// 		{
// 			fscanf( conf_file, "%d", tagset_size );
// 			num_pars++;
// 		}
// 		else
// 		{
// 			fprintf( stderr, "Unexpected parameter %s\n", buffer );
// 		}
// 	}
// 	
// 	if( num_pars < 3 )
// 	{
// 		fprintf( stderr, "Incorrect number of parameters\n" );
// 		exit(0);
// 	}
// }

JOB_DATA *create_job_data( int w_before, int w_after, int tagset_size )
{
	int i;
	JOB_DATA *job_data;
	BELIEFS_VECTOR *bvector;
	BELIEFS_VECTOR *tmp;
	
	job_data = (JOB_DATA*) malloc( sizeof(JOB_DATA) );
	job_data->w_before = w_before;
	job_data->w_after = w_after;
	job_data->tagset_size = tagset_size;
	
	tmp = bvector = (BELIEFS_VECTOR*) malloc( sizeof( BELIEFS_VECTOR ) );
	bvector->beliefs = (float*) calloc( tagset_size, sizeof(float) );
	job_data->biliefs_vec = bvector;
	
	for( i=1; i < w_before; i++ )
	{
		bvector = (BELIEFS_VECTOR*) malloc( sizeof( BELIEFS_VECTOR ) );
		bvector->beliefs = (float*) calloc( tagset_size, sizeof(float) );
		tmp->next = bvector;
		tmp = bvector;
	}
	
	tmp->next = job_data->biliefs_vec;
	
	g_job_data = job_data;
	
	return job_data;
}

void free_beliefs_vector( int num_vectors, BELIEFS_VECTOR *biliefs_vec )
{
	int i;
	BELIEFS_VECTOR *tmp;
	
	for( i=0; i < num_vectors; i++ )
	{
		tmp = biliefs_vec;
		biliefs_vec = tmp->next;
		free( tmp->beliefs );
		free( tmp );
	}
}

void free_jobs_data( JOB_DATA *jd )
{
	free_beliefs_vector( jd->w_before, jd->biliefs_vec);
	
	free( jd );
}

void complete_ttv( JOB_DATA *job_data, float *ttv_vector )
{
	int i,j, ttv_index, number_of_substituitions;
	BELIEFS_VECTOR *tmp;
	
	if( ttv_vector[0] < 0 )
	{
		number_of_substituitions = abs( ttv_vector[0] ) - 2;
	}
	else
	{
		number_of_substituitions = job_data->w_before;
	}
	
	tmp = job_data->biliefs_vec;
	ttv_index = job_data->w_before-1;
	
	// seeking the right vector (last vector)
	for( i=0; i<job_data->w_before-1; i++ )
		tmp = tmp->next;
	
	
	for( i=0; i<number_of_substituitions; i++, ttv_index-- )
	{
		for( j=0; j<job_data->tagset_size; j++ )
			ttv_vector[ ttv_index*job_data->tagset_size + j ] = tmp->beliefs[ j ];
	}
}
