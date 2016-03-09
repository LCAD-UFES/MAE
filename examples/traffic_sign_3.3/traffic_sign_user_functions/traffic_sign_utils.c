#include "traffic_sign_utils.h"
#include <stdlib.h>
#include <math.h>

int
mae_int_random(int max)
{
  return (int)(max*(rand()/(RAND_MAX+1.0)));
}

double
mae_double_random(double max)
{
  return max * (double)rand() / (double)RAND_MAX;
}

double
mae_uniform_random(double min, double max)
{
  return min + (rand() / (double)RAND_MAX) * (max - min);
}

double
mae_gaussian_random(double mean, double std)
{
  const double norm = 1.0 / (RAND_MAX + 1.0);
  double u = 1.0 - rand() * norm;                  /* can't let u == 0 */
  double v = rand() * norm;
  double z = sqrt(-2.0 * log(u)) * cos(2.0 * M_PI * v);
  return mean + std * z;
}

double
mae_normal_distribution(double std)
{
	double normal_sample = 0;
	int i;
	for (i = 0; i < 12; i++)
		normal_sample += std * (2.0 * (mae_double_random(1.0) - 0.5));

	return normal_sample / 2.0;
}

double
mae_radians_to_degrees(double theta)
{
  return (theta * 180.0 / M_PI);
}

double
mae_degrees_to_radians(double theta)
{
  return (theta * M_PI / 180.0);
}
