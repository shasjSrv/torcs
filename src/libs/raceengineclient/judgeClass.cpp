#include "judgeClass.h"
#include <iostream>
#include <sstream>
#include <vector>

#include <tgfclient.h>
#include <robot.h>
#include <raceman.h>
#include <racescreens.h>
#include <robottools.h>
#include <portability.h>

#include "racemain.h"
#include "racegl.h"
#include "raceinit.h"
#include "raceengine.h"
using namespace std;


DefaultJudge::DefaultJudge(tRmInfo *ReInfo):
                score(100),
                s(NULL)
{
    cout<<"create judge system"<<endl;

    if(ReInfo!=NULL)
    {
        s=ReInfo->s;
    }
}

DefaultJudge::~DefaultJudge()
{
    cout<<"kill judge system"<<endl;
}

void DefaultJudge::judge(tCarElt *car)
{
}

void DefaultJudge::display(tCarElt *car)
{
}

string DefaultJudge::getJudgeName()
{
    return "";
}

string DefaultJudge::getJudgeFactor()
{
    return "";
}

void DefaultJudge::showlable(void *rmScrHdle,int x8,int x9,int y)
{

}

short DefaultJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
    return 0;
}








FollowJudge::FollowJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
                                        name("Follow Result"),
                                        factor("avg.dis"),
                                        results(NULL),
                                        targetCar(NULL),
                                        nCar(0)
{
    if(s!=NULL)
    {
        results=ReInfo->results;

        nCar=s->_ncars;
        cout<<"number of player:  "<<nCar<<endl;
        if(nCar>=2)
        {
            for(int i=0;i<nCar;i++)
            {
                if(strncmp(s->cars[i]->_name, "scr_server 1", 12) == 0)
                {
                    targetCar=s->cars[i];
                    cout<<"target player get"<<endl;
                }
            }
        }
    }
}

FollowJudge::~FollowJudge()
{
    //todo
}

void FollowJudge::judge(tCarElt *car)
{
    if(targetCar!=NULL)
    {
        if(targetCar==car)
        {
            //整秒记录距离
            // if(s->currentTime>(double)(distances.size()))
            // {
                double min_dis_sque=9999999999;
                //确定被跟的车 以及 距离
                for(int i=0;i<nCar;i++)
                {
                    if(s->cars[i]!=targetCar)
                    {
                        double dis_sque=pow(targetCar->_pos_X - s->cars[i]->_pos_X,2)+
                                    pow(targetCar->_pos_Y - s->cars[i]->_pos_Y,2);
                        if(dis_sque<min_dis_sque){
                            min_dis_sque=dis_sque;
                        }
                    }
                }
                //记录
                distances.push_back(sqrt(min_dis_sque));
            // }
        }
    }
}

void FollowJudge::display(tCarElt *car)
{
    double total=0;
    vector<double>::iterator it;
    for(it=distances.begin();it!=distances.end();it++)
    {
        total+=*it;
    }
    if(distances.size()!=0){
        total/=distances.size();
    }
    else{
        total=0;
    }
    
    score=-(1*(total-20)+10*targetCar->_dammage);
    score=(1.0/(1+pow(2.72,-score)))*100;

  
    /* 设置results */
    if(results!=NULL)
    {
        GfParmSetStr(results, RE_SECT_JUDGE, RE_ATTR_JUDGE_NAME,name.c_str());
        GfParmSetStr(results, RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,factor.c_str());        
        GfParmSetNum(results, RE_SECT_JUDGE, RE_ATTR_JUDGE_FACTOR_VAL, NULL, total);        
        GfParmSetNum(results, RE_SECT_JUDGE, RE_ATTR_JUDGE_SCORE, NULL, score);
    }

}

string FollowJudge::getJudgeName()
{
    return name;
}

string FollowJudge::getJudgeFactor()
{
    return factor;
}

void FollowJudge::showlable(void *rmScrHdle,int x8,int x9,int y)
{
    judge_result=GfParmGetStr(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, judge_result,       fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                //fix err which use get function
        GfuiLabelCreateEx(rmScrHdle, "Score",       fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short FollowJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{

    
    judge_result=GfParmGetStr(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(judge_result[0]!='\0'){
        if(strcmp(GfParmGetStr(results, path, RE_ATTR_NAME, ""),"scr_server 1")==0){  //is judge car
            snprintf(buf, BUFSIZE, "%d", (int)(GfParmGetNum(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR_VAL,NULL,0)));
            GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
        else{    //is not judge car, replace 'x'
            GfuiLabelCreate(rmScrHdle, "x", GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
    }

    if(GfParmGetNum(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){            //fix err which use get function
        if(strcmp(GfParmGetStr(results, path, RE_ATTR_NAME, ""),"scr_server 1")==0){  //is judge car
            snprintf(buf, BUFSIZE, "%d", (int)(GfParmGetNum(results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0)));
            GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
                x9, y, GFUI_ALIGN_HC_VB, 0);
        }
        else{    //is not judge car, replace 'x'
            GfuiLabelCreate(rmScrHdle, "x", GFUI_FONT_MEDIUM_C,
                x9, y, GFUI_ALIGN_HC_VB, 0);
        }
    }else
        return  1;
    return  2;
}
