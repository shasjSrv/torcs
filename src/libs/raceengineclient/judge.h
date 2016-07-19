#ifndef _JUDGE_H_
#define _JUDGE_H_

#include <iostream>
#include <raceman.h>

//interface
class Judge
{
public:
    virtual ~Judge();
    virtual void judge(tCarElt *car)=0;
    virtual void display(tCarElt *car)=0;
    virtual std::string getJudgeName()=0;
    virtual std::string getJudgeFactor()=0;
};

//Factory method to create judge class
//by configuration STRING
class JudgeFactory
{
public:
    static Judge* createJudgementInst(std::string classname, tRmInfo *ReInfo);
};

#endif