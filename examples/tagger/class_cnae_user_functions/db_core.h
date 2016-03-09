#ifndef _DB_H
#define _DB_H
/* SCAE Memory DB_CORE */
#include <stdio.h>
#include <string.h>

#define CSV_HEADER_SIZE				1000
#define CSV_MAX_COLUMN_SIZE			1000000

#define FIRST_COLUMN 				1
#define MIDDLE_COLUMN 				2
#define LAST_COLUMN 				3

#define LINE_READ 				1
#define END_OF_FILE 				0
#define SEMICOLON_WITHIN_LAST_COLUMN		-1
#define EOF_WITHIN_DOUBLE_QUOTES		-2
#define EOF_WITHIN_QUOTES			-3
#define EOF_WITHIN_DOUBLE_QUOTES_WITHIN_QUOTES	-4
#define CSV_MAX_COLUMN_SIZE_EXCEEDED		-5

#define INVALID_WORD_CODE			-1
#define INVALID_LINE				-1

#define CSV_PATH				"CSV_FILES/"
#define TABLES_PATH				"DB_TABLES/"
#define DEFAULT					"DEFAULT"
#define ADD						"ADD"
#define OR						"OR"
#define OPERATION_ADD				0
#define OPERATION_OR				1
#define TF						"TF"
#define TFIDF					"TFIDF"
#define NULL_VEC				1


#define CODIGOS_CNAE(table_name,table,line)	((strstr(table_name, "SUBCLASSE") != NULL) ? (((CNAE_SUBCLASSE_LINE *)table)[line].codigo_cnae):(((CNAE_ATIVIDADE_LINE *)table)[line].codigo_cnae))

#define TWO_BYTE_UTF8_CHAR(c)		(((unsigned char)(c) == 194) || ((unsigned char)(c) == 197) || ((unsigned char)(c) == 198) || ((unsigned char)(c) == 203) || ((unsigned char)(c) == 206) || ((unsigned char)(c) == 207))


typedef struct
{
	int name_size;
	char *name;
	int line_size;
	int num_lines;
	int columns_names_size;
	char *columns_names;		/* column names separated by ";" */
	int columns_types_size;
	char *columns_types;		/* i == (int); I == (int *); f == (float); F == (float *); c (char); C (char *) */
	int associations_size;
	char *associations;		/* "TABLE_NAME TABLE_NAME-COLUMN_NAME ..." */
	void *table;
} TABLE;

struct _table_list
{
	TABLE* table;
	struct _table_list *next;
};

typedef struct _table_list TABLE_LIST;


typedef struct
{
	int codigo_sub_sec;
	int   codigo_sec_size;
	char *codigo_sec;
	int codigo_div;
	int codigo_grp;
	int codigo_cla;
	int codigo_sub;
	int dv_sub;
	int   descricao_sub_size;	/* the columns that are vectors must have their size noted to easy saving in the database */
	char *descricao_sub;
	int   compre_sub_size;
	char *compre_sub;
	int   tcompre_sub_size;
	char *tcompre_sub;
	int   ncompre_sub_size;
	char *ncompre_sub;
	int   notas_sub_size;
	char *notas_sub;
	int codigo_cnae_subclasse;
	int codigo_cnae;
} CNAE_SUBCLASSE_LINE;

#define CNAE_200_SUBCLASSE_CVS_FILE_NAME 	"cnae_200_subclasse.csv"
#define	CNAE_200_SUBCLASSE_NAME			"CNAE_200_SUBCLASSE"
#define CNAE_110_SUBCLASSE_CVS_FILE_NAME 	"cnae_110_subclasse.csv"
#define	CNAE_110_SUBCLASSE_NAME			"CNAE_110_SUBCLASSE"
#define	CNAE_SUBCLASSE_COLUMNS_TYPES		"iiCiiiiiiCiCiCiCiCii"		/* Always let the last column to codigo_cnae */


typedef struct
{
	int codigo_sub_sec;
	int codigo_sec_size;
	char *codigo_sec;
	int codigo_div;
	int codigo_grp;
	int codigo_cla;
	int codigo_sub;
	int codigo_cla_seq;
	int   descricao_ati_size;	/* the columns that are vectors must have their size noted to easy saving in the database */
	char *descricao_ati;
	int   pesquisa_ati_size;
	char *pesquisa_ati;
	int codigo_cnae_subclasse;
	int codigo_cnae;
} CNAE_ATIVIDADE_LINE;

#define CNAE_200_ATIVIDADE_CSV_FILE_NAME 	"cnae_200_atividade.csv"
#define	CNAE_200_ATIVIDADE_NAME			"CNAE_200_ATIVIDADE"
#define CNAE_110_ATIVIDADE_CSV_FILE_NAME 	"cnae_110_atividade.csv"
#define	CNAE_110_ATIVIDADE_NAME			"CNAE_110_ATIVIDADE"
#define	CNAE_ATIVIDADE_COLUMNS_TYPES	"iiCiiiiiiCiCii"		/* Always let the last column to codigo_cnae */


typedef struct
{
	int no_identificador;
	int   objeto_social_size;
	char *objeto_social;
	int   cnae_fiscal_size;
	char *cnae_fiscal;
	int  vetor_codigos_cnae_size;
	int *vetor_codigos_cnae;						/* Always let the last column to vetor_codigos_cnae */
} DADOS_LINE;

#define DADOS_110_CSV_FILE_NAME 	"dados_vitoria_110.csv"
#define	DADOS_110_NAME			"DADOS_110"
#define DADOS_200_CSV_FILE_NAME 	"dados_vitoria_200.csv"
#define	DADOS_200_NAME			"DADOS_200"
#define	DADOS_COLUMNS_TYPES		"iiCiCiI"
#define DADOS_HEADER			"NO_IDENTIFICADOR;OBJETO_SOCIAL;CNAE_FISCAL;VETOR_CODIGOS_CNAE\n"


typedef struct
{
	int  word_code_vector_size;
	int *word_code_vector;
} WORD_CODES_LINE;

#define	WORD_CODES_COLUMNS_TYPES			"iI"


typedef struct
{
	int frequencia;
	int palavra;
	int lexicon_code;
} LEXICON_LINE;

#define	LEXICON_COLUMNS_TYPES				"iii"


typedef struct
{
	int  training_and_testing_vector_size;
	float *training_and_testing_vector;
} TRAINING_AND_TESTING_VECTORS_LINE;

#define	TRAINING_AND_TESTING_VECTORS_COLUMNS_TYPES	"iF"

typedef struct
{
	int  cnae_codes_count_vector_size;
	int *cnae_codes_count_vector;
} CNAE_CODES_VECTORS_LINE;

#define	CNAE_CODES_VECTORS_COLUMNS_TYPES		"iI"


typedef struct
{
	int  training_and_testing_vector_size;
	float *training_and_testing_vector;
	int  cnae_codes_vector_size;
	int *cnae_codes_vector;
} TTV_LINE;

#define	TTV_COLUMNS_TYPES		"iFiI"
#define TTV_HEADER			"TTV_VECTOR;CNAE_CODES_VECTOR\n"


typedef struct
{
	int ttv_table_name_size;
	char *ttv_table_name;
	int lexicon_name_size;
	char *lexicon_name;
	int table_name_size;
	char *table_name;
	int number_of_columns;
	int table_column_names_size;
	char *table_column_names;
	int build_type_size;
	char *build_type;
	int weight_type_size;
	char *weight_type;
} KNOWN_TTVS_LINE;

#define KNOWN_TTVS_CSV_FILE_NAME	"KNOWN_TTVS_saved.csv"
#define KNOWN_TTVS_NAME			"KNOWN_TTVS"
#define KNOWN_TTVS_COLUMNS_TYPES	"iCiCiCiiCiCiC"
#define KNOWN_TTVS_HEADER		"TTV_TABLE_NAME;LEXICON_NAME;TABLE_NAME;NUMBER_OF_COLUMNS;TABLE_COLUMN_NAMES;BUILD_TYPE;WEIGHT_TYPE"


typedef struct
{
	int table_name_size;
	char *table_name;
	int table_source_name_size;
	char *table_source_name;
	int table_filename_size;
	char *table_filename;
	int table_type_size;
	char *table_type;
	int table_subtype_size;
	char *table_subtype;
	int encodable_columns_size;
	char *encodable_columns;
} KNOWN_TABLES_LINE;

#define KNOWN_TABLES_CSV_FILE_NAME 			"known_tables.csv"
#define	KNOWN_TABLES_NAME				"KNOWN_TABLES"
#define	KNOWN_TABLES_COLUMNS_TYPES			"iCiCiCiCiCiC"

typedef struct
{
	int lexicon_name_size;
	char *lexicon_name;
	int description_size;
	char *description;
	int number_of_columns;
	int tables_and_columns_and_limits_size;
	char *tables_and_columns_and_limits;
	int gramatical_classes_size;
	char *gramatical_classes;
	int pfs;
	int dictionary_name_size;
	char *dictionary_name;
} KNOWN_LEXICONS_LINE;

#define KNOWN_LEXICONS_CSV_FILE_NAME 			"KNOWN_LEXICONS_saved.csv"
#define	KNOWN_LEXICONS_NAME				"KNOWN_LEXICONS"
#define	KNOWN_LEXICONS_COLUMNS_TYPES			"iCiCiiCiCiiC"
#define KNOWN_LEXICONS_HEADER				"LEXICON_NAME;DESCRIPTION;NUMBER_OF_COLUMNS;TABLES_AND_COLUMNS_AND_LIMITS;GRAMATICAL_CLASSES;PFS;DICTIONARY_NAME\n"


typedef struct
{
	int core_name_size;
	char *core_name;
	int training_name_size;
	char *training_name;
	int description_size;
	char *description;
	int ttvs_and_limits_size;
	char *ttvs_and_limits;
} KNOWN_TRAININGS_LINE;

#define KNOWN_TRAININGS_CSV_FILE_NAME 			"KNOWN_TRAININGS_saved.csv"
#define	KNOWN_TRAININGS_NAME				"KNOWN_TRAININGS"
#define	KNOWN_TRAININGS_COLUMNS_TYPES			"iCiCiCiC"
#define KNOWN_TRAININGS_HEADER				"CORE_NAME;TRAINING_NAME;DESCRIPTION;TTV_AND_LIMITS\n"


typedef struct
{
	int core_name_size;
	char *core_name;
	int test_name_size;
	char *test_name;
	int training_name_size;
	char *training_name;
	int description_size;
	char *description;
	int n_tests;
	int n_hits;
	int ttvs_and_limits_size;
	char *ttvs_and_limits;
} KNOWN_TESTS_LINE;

#define KNOWN_TESTS_CSV_FILE_NAME 			"KNOWN_TESTS_saved.csv"
#define	KNOWN_TESTS_NAME				"KNOWN_TESTS"
#define	KNOWN_TESTS_COLUMNS_TYPES			"iCiCiCiCiiiC"
#define KNOWN_TESTS_HEADER				"CORE_NAME;TEST_NAME;TRAINING_NAME;DESCRIPTION;TESTS;HITS;TTV_AND_LIMITS\n"

/* global variables */

/* prototypes */

void * alloc_memory (int size);
int get_csv_file_header (FILE *csv_file, char *header);
int get_csv_file_header_num_columns (char *header);
int read_csv_file_line (char **line, FILE *csv_file, int num_columns); 
int build_user_vector(float **user_vector, int *vec_length, char *input_text, char *lexicon_name, char **tables_and_columns_and_limits, int tables_and_columns_and_limits_size, char *weight_type);
int file_exists (char *file_name);
int get_csv_file_num_lines (char *file_name);
void save_csv (char *table_name);
void create_csv (char *csv_file_name, char *header);
TABLE * get_table_by_name (char *name);
TABLE * add_or_replace_table (char *name, int line_size, int num_lines, char *columns_names, char *columns_types, char *associations, void * table);
void load_tables ();
TABLE * load_table_by_name (char *table_name);
TABLE * load_csv_cnae_subclasse (char *table_name, char *csv_file_name);
TABLE * load_csv_cnae_atividade (char *table_name, char *csv_file_name);
TABLE * load_csv_dados (char *table_name, char *csv_file_name, char *table_source_name);
TABLE * load_csv_wf (char *table_name, char *csv_file_name);
void create_cnae_codes_vectors_table (char *table_name, char *build_type);
char * group_array_in_one_string (char **array, int size, char delimiter);
char ** ungroup_string_to_array (char *string, int *size, char delimiter);
char * get_known_table_source_name (char *table_name);
char * get_known_table_type (char *table_name);
char * get_known_table_csv_filename (char *table_name, char *column_name);
char ** get_encodable_column_list (char *table_name, int *encodable_columns_list_size);
char ** get_known_tables_list_by_type (char *table_type, int *table_list_size);
int get_ttv_info (char *ttv_table_name, char **lexicon_name, char **table_name, int *number_of_columns, char ***table_column_names, char **build_type, char **weight_type);
char ** get_known_ttv_list (int *table_list_size);
int get_lexicon_info (char *lexicon_name, int *tables_and_columns_and_limits_size, char ***tables_and_columns_and_limits, char **gramatical_classes, int *pfs, char **description, char **dictionary_name);
char ** get_known_lexicon_list (int *table_list_size);
TABLE * load_known_table (char *table_name);
void save_tables ();
int get_column_by_name (char *header, char *column_name);
char * get_char_pointer_column (TABLE *table, int line, int column);
int * get_int_pointer_column (TABLE *table, int line, int column, int *num_itens);
float * get_float_pointer_column (TABLE *table, int line, int column, int *num_itens);
void add_link_between_tables (TABLE *table, char *associated_table_name);
int get_table_num_lines (char *name);
int get_table_and_column (TABLE **table, int *column, char *table_and_column);
void get_cnae_codes (int **cnae_codes, int *cnae_codes_size, char *table_name, int line, char *build_type);
void get_train_and_test_vector (float **vts, int *vts_size, char *lexicon_name, char *table_name, char *column_name, int line, char *build_type);
char get_column_type (TABLE *table, int column);
int get_int_column (TABLE *table, int line, int column);
int db_core (int num_args, char **args);
void create_word_vectors_table (char *table_and_column);
void get_text_from_column (char **field_text, char *table_column_cat, int line);
void get_codification_from_column (int **word_codes_vector, int *word_codes_length, char *table_column_cat, int line);
void get_words_and_codes_from_lexicon (char *lexicon_name, int *lexicon_size, int **word_freqs_vector, char ***words_vector, int **word_codes_vector);
int search_info (TABLE *table, char *core_name, char *name, char *first_col, char *sec_col, char *header);
void extract_vts_features (float *vts, int vts_size, int **vts_n_null_word_code, char ***vts_n_null_word, int **vts_n_null_index, float **vts_n_null_weight, int *vts_n_null_count, char *lexicon_name);
char * concat_table_and_column(char *table_name, char *table_column_name);
int separete_table_and_column (char *table_and_column, char **table_name, char **table_column_name);
int create_lexicon (char *lexicon_name, char *description, int number_of_tables, char **tables_and_columns, char *gramatical_classess, int pfs);
void create_training_and_testing_vectors_table (char *lexicon_name, char *table_and_column, char *build_type);
int create_ttv (char *ttv_table_name, char *lexicon_name, char *table_name, int number_of_columns, char **table_column_names, char *build_type, char *weight_type);
int register_ttv (char *ttv_table_name, char *lexicon_name, char *table_name, int number_of_columns, char **table_column_names, char *build_type, char *weight_type);
int separate_string_and_limits (char *string_and_limits, char **string, int *min, int *max);
void add_training_info (char *core_name, char *training_name, char *description, int n_ttvs, char **ttvs_and_limits);
void add_test_info (char *core_name, char *test_name, char *training_name, char *description, int n_ttvs, char **ttvs_and_limits, int n_tests, int n_hits);
void add_or_replace_training_info (TABLE *known_trainings_table, int line, char *core_name, char *training_name, char *description, int n_ttvs, char **ttvs_and_limits);
void add_or_replace_test_info (TABLE *known_tests_table, int line, char *core_name, char *test_name, char *training_name, char *description, int n_ttvs, char **ttvs_and_limits, int n_tests, int n_hits);
TABLE *load_csv_known_trainings_table (char *table_name, char *csv_file_name);
TABLE *load_csv_known_tests_table (char *table_name, char *csv_file_name);
void get_training_list (char ***cores, char ***training, int *length);
void get_test_list (char ***cores, char ***tests, int *length);
float* get_ttv_from_ttv_table (char *ttv_name, int line, int *length);
int* get_cnae_codes_from_ttv_table (char *ttv_name, int line, int *length);
int get_ttv_table_name_from_training (char *core_name, char *training_name, char **ttv_name, int pos);
void compute_terms_weight_in_ttv (char *lexicon_name, TTV_LINE *table_lines, int num_lines, char *weight_type);
void compute_tfidf (char *lexicon_name, TTV_LINE *table_lines, int num_lines);
char *get_word_from_string (char **word, char *string);
void load_csv_known_tables (char *table_name, char *csv_file_name);
__inline char my_tolower (char c);



int check_table (char *words_frequency_table_name, char *table_name, int number_of_columns, char **table_column_names);
int compute_tables_words_frequency (char *words_frequency_table_name, int number_of_tables, char **tables_and_columns);

#endif
