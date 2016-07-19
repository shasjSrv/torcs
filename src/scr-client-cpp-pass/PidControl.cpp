#include "PidControl.h"
#include <iostream>
using namespace std;

#define INTEGRAL_HOLDTHREAD 0.001

PidControl::PidControl(double CTE, double p, double i, double d)
            :cte(CTE),
			pte(CTE),
			proportion(p),
			integral(i),
			derivative(d),
			sumError(CTE)
			{}

double PidControl::refresh(double CTE, double delta_t)
{
	this->pte=this->cte;
	this->cte=CTE;
	sumError+=CTE;
	
	if(CTE<INTEGRAL_HOLDTHREAD&&CTE>-INTEGRAL_HOLDTHREAD) sumError=0;    //clear
	
	return proportion*cte
	       +integral*sumError
	       -derivative*(pte-cte)/delta_t;
}

double PidControl::getSuggest(double delta_t)
{
	return proportion*cte
	       +integral*sumError
	       -derivative*(pte-cte)/delta_t;
}

void PidControl::clearErro()
{
	sumError=0;
}

void PidControl::setP(double p)
{
	proportion=p;
}

void PidControl::setI(double i)
{
	integral=i;
}

void PidControl::setD(double d)
{
	derivative=d;
}