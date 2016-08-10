#include "judgeClass.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <tgfclient.h>
#include <robot.h>
#include "raceman.h"
#include <racescreens.h>
#include <robottools.h>
#include <portability.h>
#include "tgf.h"
#include <string>

#include "racemain.h"
#include "racegl.h"
#include "raceinit.h"
#include "raceengine.h"
using namespace std;


DefaultJudge::DefaultJudge(tRmInfo *ReInfo):
                score(100),
                s(NULL),
                m_judge_result(NULL),
                m_results(NULL),
				m_ReInfo(ReInfo),
				m_nCar(0)	
{
    cout<<"create judge system"<<endl;

    if(ReInfo!=NULL)
    {
        s=ReInfo->s;
    }

    if (s != NULL)
    {
        m_results = ReInfo->results;
    }
	
	double tracklength = 0.0;
	tTrackSeg* first = ReInfo->track->seg;
	tTrackSeg* seg = first;
	do {
		tracklength += seg->length;
		seg = seg->next;
	} while (seg != first);
	m_segLength = (float)tracklength;
	
	string			buf;
	buf = GetLocalDir();
	buf = buf.erase(buf.rfind(".torcs/"),7);
	buf += "distancs.txt";
	m_outfile.open(buf);
	
	m_nCar = ReInfo->s->_ncars;
	m_curTime = GfTimeClock();
}

DefaultJudge::~DefaultJudge()
{
    cout<<"kill judge system"<<endl;
	m_outfile.close();
}

void DefaultJudge::judge(tCarElt *car)
{
}

void DefaultJudge::figurOut(tCarElt *car)
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


//choose default model to show result lable
void DefaultJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short DefaultJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}








FollowJudge::FollowJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
                                        name("Follow Result"),
                                        factor("avg.dis"),
                                        targetCar(NULL)
{
    if(s!=NULL)
    {
        m_results=ReInfo->results;

        //nCar=s->_ncars;
        cout<<"number of player:  "<<m_nCar<<endl;
        if(m_nCar>=2)
        {
            for(int i=0;i<m_nCar;i++)
            {
                if(strncmp(s->cars[i]->_name, "berniw 1", 4) == 0)
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
			if((GfTimeClock() - m_curTime> (double) 1) && targetCar->race.laps >=1)
			{
                //double min_dis_sque=9999999999;
                //确定被跟的车 以及 距离
				double dis_sque = 0.0;
				for(int i=0;i<m_nCar;i++)
				{
					if(s->cars[i]!=targetCar)
					{
						dis_sque = (targetCar->pub.trkPos.seg->lgfromstart + targetCar->race.laps * m_segLength) -
							(s->cars[i]->pub.trkPos.seg->lgfromstart + s->cars[i]->race.laps * m_segLength);
					}
				}
                //记录
                distances.push_back(dis_sque);
				if(m_outfile.is_open()){							
					m_outfile<<"distances:"<<dis_sque<<"time:"<<GfTimeClock()-m_curTime<<endl;								
					m_outfile<<"dis_tag:"<<targetCar->pub.trkPos.seg->lgfromstart<<"cutlap:"<<targetCar->race.laps<<"trackNseg:"<<m_segLength<<endl;
					m_outfile<<endl;
				}

				m_curTime = GfTimeClock();
			}	
        }
    }
}

void FollowJudge::figurOut(tCarElt *car)
{
	double total=0;
    vector<double>::iterator it;
	double avg = 0;
	double deviation = 0;
    double max = 0;
	double min = 0;
	if (targetCar != NULL){
		for(it=distances.begin();it!=distances.end();it++)
		{
			total+=*it;

		}
		if(distances.size()!=0){
			avg = total/distances.size();
		}
		else{
			avg = 1;
		}
		max = *(std::max_element(distances.begin(),distances.end()));
		min = *(std::min_element(distances.begin(),distances.end()));
		for_each(begin(distances),end(distances),[&](const double d){				//compute variance
					deviation += (d-avg) * (d-avg);
					m_outfile<<"deviation:"<<deviation<<endl;
					}
				);
		deviation = sqrt(deviation/(distances.size()-1));

		if(min < 1)
		  score = 10000/avg - deviation - max/distances.size() - 5;
		else  
		  score = 10000/avg - deviation - max/distances.size();
	}
	/* 设置m_results */
	if(m_results!=NULL)
    {
        GfParmSetStr(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_NAME,name.c_str());
        GfParmSetStr(m_results, RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,factor.c_str());        
        GfParmSetNum(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_FACTOR_VAL, NULL, avg);        
        GfParmSetNum(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_SCORE, NULL, score);
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
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                //fix err which use get function
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short FollowJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{

    
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),"berniw 1")!=0){  //is judge car
            snprintf(buf, BUFSIZE, "%d", (int)(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR_VAL,NULL,0)));
            GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
        else{    //is not judge car, replace 'x'
            GfuiLabelCreate(rmScrHdle, "x", GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
    }

    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){            
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),"berniw 1")!=0){  //is judge car
            snprintf(buf, BUFSIZE, "%d", (int)(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0)));
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





LimitImageJudge::LimitImageJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
										name("Limit Result"),
                                        factor("time")

{
	if(s!=NULL)
	{
		m_results=ReInfo->results;

		//nCar=s->_ncars;
		cout<<"number of player:  "<<m_nCar<<endl;
		
	}
}

LimitImageJudge::~LimitImageJudge()
{
}

void LimitImageJudge::judge(tCarElt *car)
{
}

void LimitImageJudge::figurOut(tCarElt *car)
{
}

string LimitImageJudge::getJudgeName()
{
    return name;
}

string LimitImageJudge::getJudgeFactor()
{
    return factor;
}


void LimitImageJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short LimitImageJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}




LimitSensorJudge::LimitSensorJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
										name("Limit Result"),
                                        factor("time")

{
}

LimitSensorJudge::~LimitSensorJudge()
{
}

void LimitSensorJudge::judge(tCarElt *car)
{
}

void LimitSensorJudge::figurOut(tCarElt *car)
{
}

string LimitSensorJudge::getJudgeName()
{
    return name;
}

string LimitSensorJudge::getJudgeFactor()
{
    return factor;
}


//choose default model to show result lable
void LimitSensorJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short LimitSensorJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}



LightImageJudge::LightImageJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
                                        name("Light Result"),
                                        factor("time")

{
}



LightImageJudge::~LightImageJudge()
{
}

void LightImageJudge::judge(tCarElt *car)
{
}

void LightImageJudge::figurOut(tCarElt *car)
{
}

string LightImageJudge::getJudgeName()
{
    return name;
}

string LightImageJudge::getJudgeFactor()
{
    return factor;
}


//choose default model to show result lable
void LightImageJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short LightImageJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}



PassBasicJudge::PassBasicJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
                                        name("Limit Result"),
                                        factor("time")

{
}



PassBasicJudge::~PassBasicJudge()
{
}

void PassBasicJudge::judge(tCarElt *car)
{
}

void PassBasicJudge::figurOut(tCarElt *car)
{
}

string PassBasicJudge::getJudgeName()
{
    return name;
}

string PassBasicJudge::getJudgeFactor()
{
    return factor;
}


//choose default model to show result lable
void PassBasicJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short PassBasicJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}



PassHardJudge::PassHardJudge(tRmInfo *ReInfo):DefaultJudge(ReInfo),
                                        name("Limit Result"),
                                        factor("time")

{
}



PassHardJudge::~PassHardJudge()
{
}

void PassHardJudge::judge(tCarElt *car)
{
}

void PassHardJudge::figurOut(tCarElt *car)
{
}

string PassHardJudge::getJudgeName()
{
    return name;
}

string PassHardJudge::getJudgeFactor()
{
    return factor;
}


//choose default model to show result lable
void PassHardJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != 0){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short PassHardJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	return 0;
}
