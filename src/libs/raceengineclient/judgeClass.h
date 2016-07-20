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
    virtual void display(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
protected:
    double score;
    tSituation *s;
    float fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    const char *judge_result;
    const int BUFSIZE = 1024;
};


class FollowJudge:public DefaultJudge
{
public:
    FollowJudge(tRmInfo *ReInfo);
    virtual ~FollowJudge();
    virtual void judge(tCarElt *car);
    virtual void display(tCarElt *car);
    virtual std::string getJudgeName();
    virtual std::string getJudgeFactor();
    virtual void showlable(void *rmScrHdle,int x8,int x9,int y);
    virtual short resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path);
private:
    std::string name;
    std::string factor;
    void *results;
    
    tCarElt *targetCar;
    int nCar;
    std::vector<double> distances;
};

#endif