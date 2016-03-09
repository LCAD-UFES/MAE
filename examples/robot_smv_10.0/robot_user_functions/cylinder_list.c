#include <stdlib.h>
#include <float.h>
#include "cylinder_list.h"

#define cylinder_node_get(node)		((node) != NULL ? &((node)->cylinder) : NULL)

#define cylinder_node_previous(node)	((node) != NULL ? ((node)->previous) : NULL)

#define cylinder_node_next(node)	((node) != NULL ? ((node)->next) : NULL)

/*
Alocação & Inicialização
*/

CYLINDER_NODE *
alloc_cylinder_node (CYLINDER cylinder)
{
	CYLINDER_NODE *node = (CYLINDER_NODE*) malloc (sizeof(CYLINDER_NODE));
	if (node != NULL)
	{
		node->next = NULL;
		node->previous = NULL;
		node->cylinder = cylinder;
	}

	return node;
}

CYLINDER_LIST *
alloc_cylinder_list (void)
{
	CYLINDER_LIST *list = (CYLINDER_LIST*) malloc (sizeof(CYLINDER_LIST));
	if (list != NULL)
		init_cylinder_list (list);

	return list;
}

void
init_cylinder_list (CYLINDER_LIST *list)
{	
	list->size  = 0;	
	list->first = NULL;
	list->last  = NULL;

	list->cursor = NULL;
	list->index_cursor = -1;

	list->nearest = NULL;
	list->index_nearest = -1;
}

void
cylinder_list_free (CYLINDER_LIST *list)
{
	cylinder_list_clear (list);
	free (list);
}

/*
Manipulação do Cursor
*/

void
cylinder_list_move_first (CYLINDER_LIST *list)
{	
	list->cursor = list->first;
	list->index_cursor = (list->cursor != NULL ? 0 : -1);
}

void
cylinder_list_move_last (CYLINDER_LIST *list)
{	
	list->cursor = list->last;
	list->index_cursor = list->size - 1;
}

void
cylinder_list_move_previous (CYLINDER_LIST *list)
{	
	list->cursor = cylinder_node_previous (list->cursor);
	if (list->cursor != NULL)
		list->index_cursor--;
	else
		list->index_cursor = -1;
}

void
cylinder_list_move_next (CYLINDER_LIST *list)
{	
	list->cursor = cylinder_node_next (list->cursor);
	if (list->cursor != NULL)
		list->index_cursor++;
	else
		list->index_cursor = -1;
}

void
cylinder_list_move (CYLINDER_LIST *list, int steps)
{
	int i = 0, n = abs (steps);
	if (steps > 0) for (i = 0; i < n; i++)
		cylinder_list_move_next (list);
	else for (i = 0; i < n; i++)
		cylinder_list_move_previous (list);
}

int
is_cylinder_list_out (CYLINDER_LIST *list)
{	
	return list->cursor == NULL;
}

/*
Recuperação de Elementos
*/

CYLINDER *
cylinder_list_get_first (CYLINDER_LIST *list)
{
	return cylinder_node_get (list->first);
}

CYLINDER *
cylinder_list_get_current (CYLINDER_LIST *list)
{
	return cylinder_node_get (list->cursor);
}

CYLINDER *
cylinder_list_get_last (CYLINDER_LIST *list)
{
	return cylinder_node_get (list->last);
}

CYLINDER *
cylinder_list_get_highest (CYLINDER_LIST *list)
{
	double distance_highest = DBL_MIN;
	int index_highest = -1, i = 0, n = cylinder_list_size (list);
	for (i = cylinder_list_index_nearest (list); i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get (list, i);
		double distance = cylinder->y + cylinder_radius (cylinder);
		if (distance > distance_highest)
		{
			distance_highest = distance;
			index_highest = i;
		}
	}

	return cylinder_list_get (list, index_highest);
}

CYLINDER *
cylinder_list_find_nearest (CYLINDER_LIST *list)
{
	int i = 0, index_nearest = 0, n = cylinder_list_size (list);
	double distance_nearest = DBL_MAX;
	CYLINDER *nearest = NULL;
	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get (list, i);
		double distance = cylinder->z - cylinder_radius (cylinder);
		if (distance < distance_nearest)
		{
			distance_nearest = distance;
			index_nearest = i;
		}
	}

	nearest = cylinder_list_get (list, index_nearest);
	list->nearest = list->cursor;
	list->index_nearest = index_nearest;
	return nearest;
}

CYLINDER *
cylinder_list_get_nearest (CYLINDER_LIST *list)
{
	CYLINDER *nearest = cylinder_node_get (list->nearest);
	if (nearest == NULL)
		nearest = cylinder_list_find_nearest (list);
	else
	{
		list->cursor = list->nearest;
		list->index_cursor = list->index_nearest;
	}

	return nearest;
}

CYLINDER *
cylinder_list_get (CYLINDER_LIST *list, int index)
{
	if (is_cylinder_list_out (list))
		cylinder_list_move_first (list);
		
	if (index < 0 || index >= list->size)
		return NULL;
	
	cylinder_list_move (list, index - list->index_cursor);
	
	return cylinder_node_get (list->cursor);
}

/*
Inserção & Exclusão
*/

int
cylinder_list_add (CYLINDER_LIST *list, CYLINDER cylinder)
{
	CYLINDER_NODE *node = alloc_cylinder_node (cylinder);
	if (node == NULL)
		return 0;

	if (list->size == 0)
	{
		list->size = 1;
		list->first = node;
		list->last = node;
		list->cursor = node;
		list->index_cursor = 0;
	}
	else if (list->cursor == list->last)
	{
		list->size++;
		list->last->next  = node;
		node->previous = list->last;
		list->last = node;
		list->cursor = node;
		list->index_cursor++;
	}
	else
	{
		list->size++;
		node->previous = list->cursor;
		node->next = list->cursor->next;
		list->cursor->next->previous = node;
		list->cursor->next = node;
		list->cursor = node;
		list->index_cursor++;
	}
	
	list->nearest = NULL;
	list->index_nearest = -1;
	
	return 1;
}

int
cylinder_list_insert (CYLINDER_LIST *list, CYLINDER cylinder)
{
	CYLINDER_NODE *cursor = list->cursor;
	CYLINDER_NODE *previous = cylinder_node_previous (cursor);

	CYLINDER_NODE *node = alloc_cylinder_node (cylinder);
	if (node == NULL)
		return 0;
	
	node->previous = previous;
	node->next     = cursor;

	if (list->size == 0)
	{
		list->first = node;
		list->last  = node;
		list->index_cursor = 0;
	}
	else if (cursor == list->first)
	{
		cursor->previous = node;
		list->first = node;
	}
	else
	{
		previous->next = node;
		cursor->previous = node;
	}
	
	list->cursor = node;
	list->size++;
	
	list->nearest = NULL;
	list->index_nearest = -1;
	
	return 1;
}

void
cylinder_list_delete_current (CYLINDER_LIST *list)
{
	CYLINDER_NODE *discarded = list->cursor;
	CYLINDER_NODE *previous  = cylinder_node_previous (discarded);
	CYLINDER_NODE *next      = cylinder_node_next     (discarded);

	if (discarded == NULL)
		return;
	else if (list->size == 1)
	{
		list->first = NULL;
		list->last  = NULL;
		list->index_cursor = -1;
	}
	else if (discarded == list->last)
	{
		list->last = previous;
		previous->next = NULL;
		list->index_cursor--;
	}
	else if (discarded == list->first)
	{
		list->first = next;
		next->previous = NULL;
	}
	else
	{
		previous->next = next;
		next->previous = previous;
	}
	
	list->cursor = (next != NULL ? next : previous);
	free (discarded);
	list->size--;
	
	list->nearest = NULL;
	list->index_nearest = -1;
}

void
cylinder_list_clear (CYLINDER_LIST *list)
{
	cylinder_list_move_first (list);
	while (!is_cylinder_list_out (list))
	{
		cylinder_list_delete_current (list);
		cylinder_list_move_next (list);
	}
	
	list->nearest = NULL;
	list->index_nearest = -1;
}

/*
Visualização
*/

void
cylinder_list_print (CYLINDER_LIST *list)
{
	cylinder_list_move_first (list);
	while (!is_cylinder_list_out (list))
	{
		cylinder_print (&(list->cursor->cylinder), "Cylinder");
		cylinder_list_move_next (list);
	}
}

void
cylinder_list_view (CYLINDER_LIST *list, int arcs, int circles)
{
	cylinder_list_move_first (list);
	while (!is_cylinder_list_out (list))
	{
		view_3D_cylinder (&(list->cursor->cylinder), arcs, circles);
		cylinder_list_move_next (list);
	}
}

/*
Propriedades
*/

int
cylinder_list_index (CYLINDER_LIST *list)
{
	return list->index_cursor;
}

int
cylinder_list_index_nearest (CYLINDER_LIST *list)
{
	if (list->index_nearest == -1)
		cylinder_list_find_nearest (list);
	
	return list->index_nearest;
}

int
cylinder_list_size (CYLINDER_LIST *list)
{
	return list->size;
}
