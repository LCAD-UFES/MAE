#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <specific_core_rpcs_handling_functions.h>


/******************************************************
 Caso queira inserir um novo CORE, altere o nome do 
 arquivo a ser incluido e/ou inclua mais arquivos .h 
 Este(s) arquivo(s) deveria(m) conter os cabeçalhos 
 das funcoes que serão chamadas neste módulo.
******************************************************/ 
#include "mae.h"
#include "io_memory.h"
#include "wnn_cor_cnae.h"
#include "build_training_testing_images.h"


/******************************************************
 Os defines SUCCESS e FAIL não deveriam ser alterados. 
 Caso queira adiconar variáveis e outros defines, 
 insira-os após o define FAIL
******************************************************/
#define SUCCESS 0
#define FAIL 1

extern int g_nCorrect;
extern int **seq_code_vec_aux;
extern float **probabilites_vec_aux;
extern int *vec_size_aux;
extern int classify_flag;
#define CMLS_PATH	"CMLS_FILES/"



/*
*********************************************************************************
*  Brief: Esta função carregará o treinamento (memória) do CORE da memória	* 
*  secundária da máquina.							*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	string: Nome do Treinamneto					* 
*  Output: 	int  : RESPONSE ? (SUCESS = 0) || (FAIL = 1)                    *
*********************************************************************************
*/

int
reload (char *name)
{
	int response= FAIL;

	response = reload_neural_memory (name);

	if(response == FAIL)
		return response;

	response = reload_neurons_output_table(name);

	if(response == FAIL)
		return response;
	
	return response;
}



/*
*********************************************************************************
*  Brief: Esta função destruirá o treinamento corrente do CORE (existente na	* 
*  memória  principal da máquina). Ela também setará a memória do CORE para um 	*
*  estado inicial.					 			*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	none								* 
*  Output: 	none                                                            *
*********************************************************************************
*/

void
destroy_current_trainining_base (void)
{	
	int response= FAIL;
	
	response = reload_neural_memory ("INITIAL_STATE");

	if(response == FAIL)
		printf("Error while reloading INITIAL STATE\n");

	g_neuronsOutputTable.last = 1;
		
	return;
}



/*
*********************************************************************************
*  Brief: Esta função savará o treinamento (memória) do CORE na memória 	*
*  secundária da máquina							*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	string: Nome do Treinamneto					* 
*  Output: 	int  : RESPONSE ? (SUCESS = 0) || (FAIL = 1)                    *
*********************************************************************************
*/

int
save (char *name)
{
	int response = FAIL;

	response = save_neural_memory (name);

	if(response == FAIL)
		return response;

	response = save_neurons_output_table(name);

	if(response == FAIL)
		return response;
	
	return response;
}



/*
*********************************************************************************
*  Brief: Esta função treinará o CORE com um vetor de entrada (ttv_vector) de 	*
*  tamanho ttv_length e especificará que o target do mesmo é dado pelo vetor	* 
*  cnae_codes_vector de tamanho cnae_codes_length.				*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	float *: TTV_VECTOR	 					* 
*  Input:	int    : TTV_LENGTH						* 
*  Input:	int *  : CNAE_CODES_VECTOR					* 
*  Input:	int    : CNAE_CODES_LENGTH					* 
*  Output: 	none                                                            *
*********************************************************************************
*/

void
train_line (float *ttv_vector, int ttv_length, int *cnae_codes_vector, int cnae_codes_length)
{
	char buf_train[256];

	sprintf(buf_train,"%s%s",CMLS_PATH, "train.cml");
	
	build_training_testing_images (ttv_vector, ttv_length, cnae_codes_vector, cnae_codes_length);
	execute_script (buf_train);

	return;
}



/*
*********************************************************************************
*  Brief: Esta função testará o CORE com um vetor de entrada (ttv_vector) de 	*
*  tamanho ttv_length e target cnae_codes_vector de tamanho cnae_codes_length.	*
*  Caso o vetor seja classificado corretamente, a vairável test assumirá o valor* 
*  1, caso contrário o valor 0.							*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	float *: TTV_VECTOR	 					* 
*  Input:	int    : TTV_LENGTH						* 
*  Input:	int *  : CNAE_CODES_VECTOR					* 
*  Input:	int    : CNAE_CODES_LENGTH					* 
*  Output: 	int    : HIT                                                    *
*********************************************************************************
*/

int
test_line (float *ttv_vector, int ttv_length, int *cnae_codes_vector, int cnae_codes_length)
{
	int test;
	char buf_test[256];
	int corrects_old = g_nCorrect;

	classify_flag = 0;

	sprintf(buf_test,"%s%s",CMLS_PATH, "test.cml");

	build_training_testing_images (ttv_vector, ttv_length, cnae_codes_vector, cnae_codes_length);
	execute_script (buf_test);

	test = g_nCorrect - corrects_old;

	return (test);
}



/*
*********************************************************************************
*  Brief: Esta função chamada pela INTERFACE 8. Ela classificará o vetor	* 
*  user_vector de tamanho vec_length. Antes do processo de classificação ser	*
*  iniciado, SEQ_CODE_VEC (vetor de códigos sequenciais) e PROBABILITIES_VEC 	*
*  (vetor de probabilidaes) deverão ser alocados dinamicamente (o tamanho	*
*  dependerá da quantidade máxima de saídas que o CORE classificador poderá 	*
*  apresentar por vez, este valor deverá ser atribuído ao parmâmetro vec_size).	*
*  Assim, ao final da classificação, PROBABILITIES_VEC[i] deverá representar	*
*  a probabilidade do código sequencial SEQ_CODE_VEC[i] pertencer à descrição 	*
*  do usuário.					*				*
*  Author: Felipe Thomaz Pedroni                                                *
*  Input:	float *: USER_VECTOR	 					* 
*  Input:	int    : VEC_LENGHT						* 
*  Input:	int ** : SEQ_CODE_VEC						* 
*  Input:	float**: PROBABILITES_VEC					* 
*  Input:	int *  : VEC_SIZE						* 
*  Output: 	none                                    	                *
*********************************************************************************
*/

void
classify (float *user_vector, int vec_length, int **seq_code_vec, float **probabilites_vec, int *vec_size)
{
	char buf_classify[256];

	classify_flag = 1;

	sprintf(buf_classify,"%s%s",CMLS_PATH, "classify.cml");	
	
	build_training_testing_images (user_vector, vec_length, NULL, 0);

	execute_script (buf_classify);

	*seq_code_vec = seq_code_vec_aux;
	*probabilites_vec = probabilites_vec_aux;
	*vec_size = vec_size_aux;
	
	return;
}
