#ifndef PIDCONTROL_H_
#define PIDCONTROL_H_

#include <iostream>


class PidControl
{
public:
	PidControl(double CTE, double p, double i, double d);
	//double getAdjustVValue();
	double refresh(double CTE, double delta_t);
	double getSuggest(double delta_t);
	void clearErro();
	void setP(double p);
	void setI(double i);
	void setD(double d);
	double getP(){return proportion;}
	double getI(){return integral;}
	double getD(){return derivative;}
	
private:
	double proportion; // 比例常数Proportional Const
    double integral; // 积分常数Integral Const
    double derivative; // 微分常数Derivative Const
    double cte;        //当前误差（接受新数据前）
	double pte;
    double sumError; // Sums of Errors
};

#endif    /*PIDCONTROL_H_*/