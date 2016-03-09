#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "math.h"




/* --------------------------------------------------------------------------
 *     Clean the vector of class
 * -------------------------------------------------------------------------- */

void
clean_vec_class(int *vec_class, int n_class)
{
	int class_counter;

        for(class_counter = 0; class_counter < n_class; class_counter++)
		vec_class[class_counter] = NOTHING;
	 
	return;
}



/* --------------------------------------------------------------------------
 *     Clean the vector of words
 * -------------------------------------------------------------------------- */

void
clean_vec_words (int *vec_words, int size)
{
	int word_counter;

	for(word_counter = 0; word_counter < size; word_counter++)
		vec_words[word_counter] = BLACK;

	return;

}



/* --------------------------------------------------------------------------
 *     Clean the vector of words
 * -------------------------------------------------------------------------- */

void
build_cnae_file (char *inicial, DESCRIPTION *cdoc, int doc_id, int n_class)
{
	char FileName[256];
	FILE *FileOut = (FILE *) NULL;
	int i;
	char *doc_name;
	int *vec_class;
	
	doc_name = cdoc->name;
	vec_class = cdoc->data;

	sprintf (FileName, "%s%d.cnae", inicial,  doc_id);

        if (!(FileOut = fopen(FileName,"w")))
	{
		fprintf (stderr,"Cannot open input file %s\n",FileName);
	        exit(1);
	}

	fprintf(FileOut,"#Codigo %s\n", doc_name);
       
	for(i = 0; i < n_class ; i++)
	{
		if(vec_class[i] != NOTHING)
		{
			fprintf(FileOut,"%d\n",cdoc->data[i]);
			//printf("%d\n",vec_class[i]);
		}
	}
	
	fclose(FileOut);
	return;
}



/* --------------------------------------------------------------------------
 *    Builds the files .pnm
 * -------------------------------------------------------------------------- */

void
build_image_file (char *inicial, DESCRIPTION *ddoc, int doc_id, int n_words, int dimension)
{
	char FileName[256];
	FILE *FileOut = (FILE *) NULL;
	int i;
	char *doc_name;
	int *vec_words;
	
	doc_name = ddoc->name;
	vec_words = ddoc->data;

        sprintf (FileName, "%s%d.pnm", inicial,  doc_id);

        if (!(FileOut = fopen(FileName,"w")))
        {
	        fprintf (stderr,"Cannot open input file %s\n",FileName);
	        exit(1);
	}
								     
	fprintf(FileOut,"P2\n");
	fprintf(FileOut,"#Documento do Felipe. Codigo %s\n", doc_name);
	fprintf(FileOut,"%d %d\n",dimension, dimension);
	fprintf(FileOut,"255\n");
	
	for(i = 0; i < dimension * dimension; i++)
        {
		if((i+1)% dimension == 0)
			fprintf(FileOut,"%d\n", vec_words[i]);
		else
			fprintf(FileOut,"%d ", vec_words[i]);
	}
  
	fclose(FileOut);
	return;
}


/* --------------------------------------------------------------------------
 *    Allocates memory
 * -------------------------------------------------------------------------- */

void *
alloc_mem (int size, char *error_message)
{
	void *memory_space;
	
	memory_space = (void *) malloc (size);
	if (!memory_space)
	{
		printf ("%s\n", error_message);
		exit (1);
	}
	return (memory_space);
}


/* --------------------------------------------------------------------------
 *    Allocates memory and init vectors 
 * -------------------------------------------------------------------------- */

void
alloc_vectors (DESCRIPTION **vec_desc_obj, DESCRIPTION **vec_desc_class, int n_obj, int n_class, int size)
{
	int i, *aux;
	
	*vec_desc_obj = (DESCRIPTION *) alloc_mem (n_obj * sizeof (DESCRIPTION), "Could not allocate memory for vec_desc_obj in alloc_vectors()");
	for(i = 0; i < n_obj; i++)
	{
		aux = (int *) alloc_mem (size * sizeof (int), "Could not allocate memory in the first aux allocation in alloc_vectors()");
		clean_vec_words (aux, size);
		(*vec_desc_obj)[i].data = aux;
		(*vec_desc_obj)[i].name = (char *) alloc_mem (NAME_SIZE, "Could not allocate memory in the first name allocation in alloc_vectors()");
	}
	
	*vec_desc_class = (DESCRIPTION *) alloc_mem (n_class * sizeof (DESCRIPTION), "Could not allocate memory for vec_desc_class in alloc_vectors()");
	for(i = 0; i < n_class; i++)
	{
		aux = (int *) alloc_mem (size * sizeof (int), "Could not allocate memory in the second aux allocation in alloc_vectors()");		
		clean_vec_words (aux, size);
		(*vec_desc_class)[i].data = aux;
		(*vec_desc_class)[i].name = (char *) alloc_mem (NAME_SIZE, "Could not allocate memory in the second name allocation in alloc_vectors()");
	}
}


/* --------------------------------------------------------------------------
 *    Allocates memory and init vectors 
 * -------------------------------------------------------------------------- */

void
free_vectors (DESCRIPTION *vec_desc_obj, DESCRIPTION *vec_desc_class, int n_obj, int n_class)
{
	int i;
	
	for(i = 0; i < n_obj; i++)
	{
		free (vec_desc_obj[i].data);
		free (vec_desc_obj[i].name);
	}
	free (vec_desc_obj);
	
	for(i = 0; i < n_class; i++)
	{
		free (vec_desc_class[i].data);		
		free (vec_desc_class[i].name);
	}
	free (vec_desc_class);
}


/* --------------------------------------------------------------------------
 *    Read the class's name
 * -------------------------------------------------------------------------- */

void
read_obj_and_class_cnae_names (DESCRIPTION *vec_desc_obj, DESCRIPTION *vec_desc_class, FILE *in_file, int n_obj, int n_class)
{
	int i, j;
	char *aux;
	
	aux = (char *) alloc_mem (1000000 * sizeof (char), "Could not allocate memory for aux in read_obj_and_class_cnae_names()");

	for (i = 0, j = 0; i < n_obj + n_class; i++) // loop to class
	{
		if (i < n_obj)
		{
			fscanf(in_file,"%s ", aux);
			if (strlen (aux) > NAME_SIZE)
			{
				printf ("Name too large in read_obj_and_class_cnae_names()\n");
				exit (1);
			}
			strcpy (vec_desc_obj[i].name, aux);
		}
		else
		{
			fscanf(in_file,"%s ", aux);
			if (strlen (aux) > 255)
			{
				printf ("Name too large in read_obj_and_class_cnae_names()\n");
				exit (1);
			}
			strcpy (vec_desc_class[i - n_obj].name, aux);
		}
	}
}


/* --------------------------------------------------------------------------
 *    Read the matrix of words
 * -------------------------------------------------------------------------- */

void
read_matrix_of_words (DESCRIPTION *vec_desc_obj, DESCRIPTION *vec_desc_class, FILE *in_file, int n_words, int n_obj, int n_class)
{
	int i, ocurrences, col, line, color;

	for(i = 0, col = 0, line = 0;  i < n_words * (n_obj + n_class); i++, col++)
	{
		fscanf(in_file,"%d ",&ocurrences);

		if(ocurrences != 0) /* only the existance or not of an ocurrence is considered */
			color = WHITE;
		else
			color = BLACK;
		
		if((i % (n_obj + n_class) == 0) && i)
		{
			col = 0;
			line++;
		}
			
		if(col < n_obj)
			vec_desc_obj[col].data[line] = color;
		else
			vec_desc_class[col - n_obj].data[line] = color;
	}
}


/* --------------------------------------------------------------------------
 *    Build the image files
 * -------------------------------------------------------------------------- */

void
build_image_files (DESCRIPTION *vec_desc_obj, DESCRIPTION *vec_desc_class, int n_words, int n_obj, int n_class, int dimension)
{
	int i;
	int doc_id_obj = 1, doc_id_class = 1;

	for(i = 0;  i < (n_obj + n_class); i++)
	{
		if(i < n_obj)
		{
			build_image_file ("DOCS_DB/", &vec_desc_obj[i], doc_id_obj, n_words, dimension);
			doc_id_obj++;
		}
		else
		{
			build_image_file ("CNAE_TAB/", &vec_desc_class[i - n_obj], doc_id_class, n_words, dimension);
			doc_id_class++;
		}
	}
}


/* --------------------------------------------------------------------------
 *    Read the matrix of classes
 * -------------------------------------------------------------------------- */

void
read_matrix_of_classes (DESCRIPTION *vec_class_obj, DESCRIPTION *vec_class_class, FILE *in_file, int n_obj, int n_class)
{
	int i, class, col, line;

	for(i = 0, col = 0, line = 1;  i < (n_obj + n_class) * n_class; i++, col++)
	{
		fscanf(in_file,"%d ",&class);

		if((i % (n_obj + n_class) == 0) && i)
		{
			col = 0;
			line++;
		}

		if(class)
		{			
			if(col < n_obj)
				vec_class_obj[col].data[line] = line;
			else
				vec_class_class[col - n_obj].data[line] = line;
		}
	}
}


/* --------------------------------------------------------------------------
 *    Build the cnae files
 * -------------------------------------------------------------------------- */

void
build_cnae_files (DESCRIPTION *vec_class_obj, DESCRIPTION *vec_class_class, int n_obj, int n_class)
{
	int i;
	int doc_id_obj = 1, doc_id_class = 1;

	for (i = 0;  i < (n_obj + n_class); i++)
	{
		if(i < n_obj)
		{
			build_cnae_file ("DOCS_DB/", &vec_class_obj[i], doc_id_obj, n_class);
			doc_id_obj++;
		}
		else
		{
			build_cnae_file ("CNAE_TAB/", &vec_class_class[i - n_obj], doc_id_class, n_class);
			doc_id_class++;
		}
	}
}



/* --------------------------------------------------------------------------
 *    Trace the class file 
 * -------------------------------------------------------------------------- */

void
trace_class_file (FILE *in_file)
{
	DESCRIPTION *vec_class_obj;
	DESCRIPTION *vec_class_class;
	int n_obj, n_class;
			
	fscanf(in_file,"%d %d\n", &n_class, &n_obj);
	
	alloc_vectors (&vec_class_obj, &vec_class_class, n_obj, n_class, n_class);
	read_obj_and_class_cnae_names (vec_class_obj, vec_class_class, in_file, n_obj, n_class);
	read_matrix_of_classes (vec_class_obj, vec_class_class, in_file, n_obj, n_class);
	build_cnae_files (vec_class_obj, vec_class_class, n_obj, n_class);
	free_vectors (vec_class_obj, vec_class_class, n_obj, n_class);
	
	return;
}



/* --------------------------------------------------------------------------
 *    Traces the word file
 * -------------------------------------------------------------------------- */

void
trace_word_file (FILE *in_file)
{
	DESCRIPTION *vec_desc_obj;
	DESCRIPTION *vec_desc_class;
	int n_words, n_obj, n_class, dimension;
			
	fscanf(in_file,"%d %d %d\n", &n_words,  &n_obj, &n_class);
	dimension = (int) sqrt ((double) n_words) + 1; // used to build .pnm files
	
	alloc_vectors (&vec_desc_obj, &vec_desc_class, n_obj, n_class, dimension * dimension);
	read_obj_and_class_cnae_names (vec_desc_obj, vec_desc_class, in_file, n_obj, n_class);
	read_matrix_of_words (vec_desc_obj, vec_desc_class, in_file, n_words, n_obj, n_class);
	build_image_files (vec_desc_obj, vec_desc_class, n_words, n_obj, n_class, dimension);
	free_vectors (vec_desc_obj, vec_desc_class, n_obj, n_class);
	
	return;
}



/* --------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */

int 
main (int argc, char *argv[])
{
	FILE *word_matrix;
	FILE *class_matrix;

	if (argc < 3) 
	{
		fprintf(stderr,"\n\
Faltam parametros. Duas matrizes sao necessarias. Na primeira, cada coluna\n\
representa um objeto social ou uma descriccao de uma classe CNAE, equanto que\n\
cada linha representa uma palavra do dicionario. No arquivo corrente existem\n\
3281 objetos sociais e 764 classes CNAE, logo, esta matriz possui 3281+764 colunas,\n\
e 5114 palavras no dicionario, logo, 5114 linhas. O conteudo desta matriz e' o numero de \n\
ocorrencias de cada palavra do dicionario no objeto social ou na descriccao \n\
da classe CNAE correspondente.\n\n\
\
Na segunda matriz, as colunas tem um mesmo significado das da primeira matriz,\n\
equanto que as linhas representam a classes CNAE. Logo, a segunda matriz possui\n\
764 linhas e 3281+764 colunas. O conteudo desta matriz 'e 1 ou 0, indicando se\n\
uma determinada classe CNAE esta associada ou nao a um objeto social ou classe\n\
CNAE (o trecho da matriz referente 'as classes CNAE forma uma diagonal de 1s).\n\n\
Usage\n data_conversion primeira_matriz.txt segunda_matriz.txt\n");
		exit(1);
	}

	/* abre aquivos manipulados */
	if (!(word_matrix = fopen(argv[1],"r"))) 
	{
		fprintf (stderr,"Cannot open input file %s\n",argv[1]);
	        exit(1);
	}

	if (!(class_matrix = fopen(argv[2],"r"))) 
	{
		fprintf (stderr,"Cannot open input file %s\n",argv[2]);
	        exit(1);
	}
	
	trace_word_file (word_matrix);
	trace_class_file (class_matrix);
	
	fclose (word_matrix);
	fclose (class_matrix);
	
	return 0;
}
