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
protected:
    double score;
    tSituation *s;
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
private:
    std::string name;
    std::string factor;
    void *results;
    
    tCarElt *targetCar;
    int nCar;
    std::vector<double> distances;
};

#endif