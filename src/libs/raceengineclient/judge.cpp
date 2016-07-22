#include "judge.h"
#include "judgeClass.h"
#include "tgf.h"
#include <iostream>
using namespace std;


Judge* JudgeFactory::createJudgementInst(string classname,tRmInfo *ReInfo)
{
    /* ---------------------------------------------------- */
    /* ---------------------------------------------------- */
    /* insert your proect here */
    /* ---------------------------------------------------- */
    /*
    if(classname=="JudgeName"){
        return new JudgeName(ReInfo);
    }
    */
    /* ---------------------------------------------------- */
 
    if(classname=="FollowJudge"){
        cout<<"create FollowJudge..."<<endl;
        return new FollowJudge(ReInfo);
    }

    //protect
    cout<<"no one matched    create DefaultJudge..."<<endl;
    return new DefaultJudge(ReInfo);
}

Judge::~Judge()
{
    
}
