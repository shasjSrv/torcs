#include "judge.h"
#include "judgeClass.h"
#include "tgf.h"
#include <iostream>
using namespace std;

/*
int hash_StrToInt(const char *s){
    if(s!=NULL){
        size_t result=0;
        for(int i=0; s[i]!='\0'; i++)
   	    {
            result=(result*131)+s[i];
        }
        return (int)(result);
    }
    else{
        return -1;
    }
}
*/

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

    if(classname=="DefaultJudge"){
        cout<<"create DefaultJudge..."<<endl;
        return new DefaultJudge(ReInfo);
    }

    //protect
    cout<<"no one matched    create DefaultJudge..."<<endl;
    return new DefaultJudge(ReInfo);
}

Judge::~Judge()
{
    
}