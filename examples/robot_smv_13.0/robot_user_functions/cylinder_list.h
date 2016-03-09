#ifndef CYLINDER_LIST_H
#define CYLINDER_LIST_H

#include "cylinder.h"

/*
Tipos de Dados
*/

struct _cylinder_node
{
	CYLINDER cylinder;
	struct _cylinder_node *next;
	struct _cylinder_node *previous;
};

typedef struct _cylinder_node CYLINDER_NODE;

struct _cylinder_list
{
	int size;

	CYLINDER_NODE *first;
	CYLINDER_NODE *last;
	
	CYLINDER_NODE *cursor;
	int index_cursor;
	
	CYLINDER_NODE *nearest;
	int index_nearest;
};

typedef struct _cylinder_list CYLINDER_LIST;

/*
Alocação & Inicialização
*/

CYLINDER_NODE * alloc_cylinder_node (CYLINDER cylinder);

CYLINDER_LIST * alloc_cylinder_list (void);

void init_cylinder_list (CYLINDER_LIST *list);

void cylinder_list_free (CYLINDER_LIST *list);

/*
Recuperação de Elementos
*/

CYLINDER * cylinder_list_get_first (CYLINDER_LIST *list);

CYLINDER * cylinder_list_get_last (CYLINDER_LIST *list);

CYLINDER * cylinder_list_get (CYLINDER_LIST *list, int index);

/*
Inserção & Exclusão
*/

int cylinder_list_append(CYLINDER_LIST *list, CYLINDER cylinder);

int cylinder_list_copy(CYLINDER_LIST *list, CYLINDER_LIST *source);

int cylinder_list_insert(CYLINDER_LIST *list, CYLINDER cylinder);

void cylinder_list_delete(CYLINDER_LIST *list, int index);

void cylinder_list_clear(CYLINDER_LIST *list);

/*
Visualização
*/

void cylinder_list_print (CYLINDER_LIST *list);

void cylinder_list_view (CYLINDER_LIST *list, int arcs, int circles);

void cylinder_list_view_transformed(CYLINDER_LIST *list, double *eulerAngles, double *displacement, double *massCenter);

/*
Propriedades
*/

int cylinder_list_index (CYLINDER_LIST *list);

int cylinder_list_size (CYLINDER_LIST *list);

#endif
