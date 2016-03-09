#ifndef LINE_H
#define LINE_H

#include <stdlib.h>

#include <vector>
using std::vector;

/*
Tipos de Dados
*/

struct _line
{
	double x0, y0, z0;
	
	double a, b, c;
};

typedef struct _line LINE;

/*
Alocação & Inicialização
*/

LINE *line_alloc  (double x0, double y0, double z0, double a, double b, double c);

LINE *line_alloc2 (double x0, double y0, double z0, double alpha, double betha);

LINE* line_alloc_slope(double x0, double y0, double z0, double m);

void line_init (LINE *line, double x0, double y0, double z0, double a, double b, double c);

LINE *line_fit (double x0, double y0, double z0, vector<double>& x, vector<double>& y);

/*
Comparação
*/

double line_distance (LINE *l1, LINE *l2);

/*
Visualização
*/

void line_3D_view (LINE *line, int length);

/*
Propriedades
*/

double line_get_cos_alpha (LINE *line);

double line_get_cos_betha (LINE *line);

double line_get_cos_gama (LINE *line);

void line_set_origin (LINE *line, double x0, double y0, double z0);

void line_set_direction (LINE *line, double a, double b, double c);

void line_set_direction_angles (LINE *line, double alpha, double betha);

#endif
