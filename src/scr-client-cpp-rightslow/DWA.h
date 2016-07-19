#ifndef _DWA_H_
#define _DWA_H_

#include "CarState.h"

typedef struct
{
	double v;
	double w;
}t_vw;

class DWA
{
public:
    DWA();
	t_vw getSuggVW(CarState const &cs, double target_x, double target_y, double delta_t=0.02);
	bool isValid(CarState const &cs, double delta_t=0.02);
private:
    double arg_heading;
	double arg_dist;
	double arg_vel;
}

#endif