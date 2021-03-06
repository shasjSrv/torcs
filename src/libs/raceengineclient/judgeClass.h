#ifndef _JUDGE_CLASS_H_
#define _JUDGE_CLASS_H_

#include "judge.h"
#include <raceman.h>
#include <list>
#include <fstream>
#include <iostream>
#include <vector>


typedef struct lengthInfo{
	double			length;
	double			width;
	
}LengthInfo;

class DefaultJudge: public Judge
{
public:
    DefaultJudge(tRmInfo *ReInfo);
    virtual ~DefaultJudge();
    virtual void judge(tCarElt *car);
    
	//you can inherit this function to compute the scores in your judgement
	virtual void figurOut(tCarElt *car);
    
	virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();

    //you can inherit this function to show what judgement's name you design
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y); 

    //you can inherit this function to show what score you figure up          
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
protected:
    double			score;
    tSituation		*s;
    float			m_fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    const char		*m_judge_result;
    const int		BUFSIZE = 1024;
    void			*m_results;
	tRmInfo			*m_ReInfo;
	float			m_segLength;
	std::ofstream	m_outfile;
	int				m_nCar;
	double			m_curTime;
	int				m_maxTime; 
	const int				m_fullScore = 100;			//the full score in our judgement
	const float		m_min = 1.0f;					//the min distance to target driver.
	const float		m_max = 300.0f;					//the max distance to target driver.
	const float		m_penalty = 5;					//To punish the score in some situation.
	const char		*m_targName = "target name";		//Config the target driver.
	const char		*m_targlength = "target length";	//config the length of target name.
};


class FollowJudge:public DefaultJudge
{
public:
    FollowJudge(tRmInfo *ReInfo);
    virtual ~FollowJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string				name;
	std::string				factor;
	tCarElt					*targetCar;
	std::list<double>		m_distances;
	const int				m_expectDis = 50;			//the expect distance to target car in our judgement
};


class LimitImageJudge:public DefaultJudge
{
public:
    LimitImageJudge(tRmInfo *ReInfo);
    virtual ~LimitImageJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string name;
    std::string factor;
    
};


class LimitSensorJudge:public DefaultJudge
{
public:
    LimitSensorJudge(tRmInfo *ReInfo);
    virtual ~LimitSensorJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string name;
    std::string factor;

};


class LightImageJudge:public DefaultJudge
{
public:
    LightImageJudge(tRmInfo *ReInfo);
    virtual ~LightImageJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string name;
    std::string factor;

};



class PassBasicJudge:public DefaultJudge
{
public:
    PassBasicJudge(tRmInfo *ReInfo);
    virtual ~PassBasicJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string				name;
    std::string				factor;
	tCarElt					*targetCar;
	std::vector<LengthInfo>	m_distances;
	//According to our time is per 0.5s to record 1 time ,so our condition is that the car should follow the target car 2 times.
	const int				m_condition = 5;							
	//The max judge times in our judgement config file's name
	const char*				m_judgeNum = "judge pot num";
	//The angle we consider that is suitable in follow action.
	const double			m_angle = 3.1415926/18;
	//The distance we design that should judge the action whether start or end.
	const int				m_safeDistance = 50;
	//The time that overtaking target car which using the queckest speed to take.  
	const int				m_bestRecord = 8;

};



class PassHardJudge :public DefaultJudge
{
public:
    PassHardJudge(tRmInfo *ReInfo);
    virtual ~PassHardJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string				name;
    std::string				factor;
	tCarElt					*targetCar;

};















#endif
