#include "DWA.h"
#include "math.h"

DWA::DWA():arg_heading(1),arg_dist(10),arg_vel(0.5)
{
}

t_vw DWA::getSuggVW(CarState const &cs, double target_x, double target_y, double delta_t=0.02)
{
	t_vm result;
	double select_v=0;
	double select_w=0;
	double score=-10000000;
	
	/*
	
	float maxAccel=cs.getMaxAccel();
	double cur_V=cs.getSpeedX();
	cur_V/=3.6;      //m/s
	double max_V=cur_V+maxAccel*delta_t/2;  //avg
	double min_V=cur_V-maxAccel*delta_t/2;  //avg
	
	float maxSteer=cs.getMaxSteer();
	double max_W=cur_V*tan(maxSteer)/cs.getL();  //not perfect
	double min_W=-cur_V*tan(maxSteer)/cs.getL();
	
	//筛选 以自己为原点
	//轨道
	if(cs.getTrackPos()<=-0.75) min_W=0;
	if(cs.getTrackPos()>=0.75) max_W=0;
	//评估
	int n_v=10;
	int n_w=10;
	double delta_v=(max_V-minV)/n_v;
	double delta_w=(max_W-min_W)/n_w;
	for(int i=0;i<=n_v;i++)
	{
		double v=min_V+delta_v*i;
		for(int j=0;j<=n_w;j++)
		{
			double w=min_W+delta_w*j;
			//double r=0;
			double centre_x=0;
			if(w!=0){
				centre_x=w>0?-fabs(v/w):fabs(v/w);
				for(int index_oppo=0;index_oppo<36;index_oppo++)
			    {
				    if(cs.getOpponents(index_oppo)<200){
					    //判断旋转方向
						//判断是否在轨道
						//计算完美度，比较当前完美度，替换
				    }
			    }
			}
			else{
				//直线
				//遍历
				//判断是否在轨道
				//计算完美度，比较当前完美度，替换
			}
		}
	}
	*/
	
	result.v=select_v;
	result.w=select_w;
	return result;
}

bool DWA::isValid(CarState const &cs, double delta_t=0.02)
{
	
	
	
	return true;
}
