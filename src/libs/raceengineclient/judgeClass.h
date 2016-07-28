#ifndef _JUDGE_CLASS_H_
#define _JUDGE_CLASS_H_

#include "judge.h"
#include <raceman.h>
#include <vector>

class DefaultJudge: public Judge
{
public:
    DefaultJudge(tRmInfo *ReInfo);
    virtual ~DefaultJudge();
    virtual void judge(tCarElt *car);
    virtual void figurOut(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();

    //you can inherit this function to show what judgement's name you design
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y); 

    //you can inherit this function to show what score you figure up          
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
protected:
    double score;
    tSituation *s;
    float m_fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    const char *m_judge_result;
    const int BUFSIZE = 1024;
    void *m_results;
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
    std::string name;
    std::string factor;
    
    
    tCarElt *targetCar;
    int nCar;
    std::vector<double> distances;
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
    std::string name;
    std::string factor;

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
    std::string name;
    std::string factor;

};















#endif
