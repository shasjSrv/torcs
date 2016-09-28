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
                score(-1),
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
	m_maxTime = GfParmGetNum(ReInfo->params, RE_SECT_JUDGE, "MaxTime", NULL, 0);
}

DefaultJudge::~DefaultJudge()
{
    cout<<"kill judge system"<<endl;
	m_outfile.close();
}

void DefaultJudge::judge(tCarElt *car)
{
	if(m_nCar >= 1){
		cout<<"CurTime:"<<GfTimeClock() - m_curTime<<endl;
		if(s->cars[0] && GfTimeClock() - m_curTime >= m_maxTime){
			cout<<"time out!"<<endl;
			s->cars[0]->_state |= RM_CAR_STATE_FINISH;
			s->cars[0]->_curTime = GfTimeClock() - m_curTime;
			//ReInfo->_reRunning = 0;
			/*int mode = ReRaceEnd();*/
			//if (mode & RM_NEXT_STEP) {
				//ReInfo->_reState = RE_STATE_POST_RACE;
			//} else if (mode & RM_NEXT_RACE) {
				//ReInfo->_reState = RE_STATE_RACE_START;
			/*}*/

		}
	}
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
        GfuiLabelCreateEx(rmScrHdle, "Vaildtime",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
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
                if(strncmp(s->cars[i]->_name, GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""), \
								GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, m_targlength, NULL, 0)) == 0)
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
                m_distances.push_back(dis_sque);
				if(m_outfile.is_open()){							
					m_outfile<<"distances:"<<dis_sque<<"time:"<<GfTimeClock()-m_curTime<<endl;								
					m_outfile<<"dis_tag:"<<targetCar->pub.trkPos.seg->lgfromstart<<"cutlap:"<<targetCar->race.laps<<"trackNseg:"<<m_segLength<<endl;
					m_outfile<<"width:"<<targetCar->pub.trkPos.toRight + targetCar->pub.trkPos.toLeft<<endl;
					m_outfile<<endl;
				}

				m_curTime = GfTimeClock();
			}	
        }
    }
}

void FollowJudge::figurOut(tCarElt *car)
{
	const char *race = m_ReInfo->_reRaceName;
	const int BUFSIZE = 1024;
	char path[BUFSIZE];
	int	   demage = 0;
	double total=0;
    list<double>::iterator it;
	double avg = 0;
	double deviation = 0;
    double max = 0;
	double min = 0;
	if (targetCar != NULL){
		for(it=m_distances.begin();it!=m_distances.end();it++)
		{
			total+=*it;

		}
		if(m_distances.size()!=0){
			avg = total/m_distances.size();
		}
		else{
			avg = 1;
		}
		max = *(std::max_element(m_distances.begin(),m_distances.end()));
		min = *(std::min_element(m_distances.begin(),m_distances.end()));
		for_each(begin(m_distances),end(m_distances),[&](const double d){				//compute variance
					deviation += (d-avg) * (d-avg);
					}
				);
		deviation = sqrt(deviation/(m_distances.size()-1));
		m_outfile<<"deviation:"<<deviation<<endl;
		for(int i=0;i<m_nCar;i++){
			snprintf(path, BUFSIZE, "%s/%s/%s/%s/%d", m_ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
			if(strstr(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),"scr_server")!=0)
				demage = (int)(GfParmGetNum(m_results, path, RE_ATTR_DAMMAGES, NULL, 0));
			m_outfile<<"i:"<<i<<",    demage:"<<demage<<endl;
			m_outfile<<GfParmGetStr(m_results, path, RE_ATTR_NAME, "")<<endl;
		}	

		float w1 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight1", NULL, 0);
		float w2 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight2", NULL, 0);
		float w3 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight3", NULL, 0);
		

		if(min < m_min && max > m_max)
			score = (std::exp(-std::fabs(m_expectDis-avg)/100))* m_fullScore* w1 + std::exp(-deviation/100.0) * m_fullScore * w2 - 2 * m_penalty - w3 * demage; 
		else if(min < m_min || max > m_max)
			score = (std::exp(-std::fabs(m_expectDis-avg)/100))* m_fullScore* w1 + std::exp(-deviation/100.0) * m_fullScore * w2 - m_penalty - w3 * demage; 
		else
			score = (std::exp(-std::fabs(m_expectDis-avg)/100))* m_fullScore* w1 + std::exp(-deviation/100.0) * m_fullScore * w2 -w3 * demage; 

	}
	if(score < 0 )
		score = 0;
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
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""))!=0){  //is judge car
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
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""))!=0){  //is judge car
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
                                        name("Pass Result"),
                                        factor("time")

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
				if(strncmp(s->cars[i]->_name, GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""), \
								GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, m_targlength, NULL, 0)) == 0)
				{
					targetCar=s->cars[i];
					cout<<"target player get"<<endl;
				}
			}
		}
	}
}



PassBasicJudge::~PassBasicJudge()
{
}

void PassBasicJudge::judge(tCarElt *car)
{
	if(targetCar!=NULL)
	{
		if(targetCar==car)
		{
			//per 0.1 second record
			if((GfTimeClock() - m_curTime> (double) 0.2) && targetCar->race.laps >=1)
			{
				//确定被跟的车 以及 距离
				LengthInfo dis_sque ;
				for(int i=0;i<m_nCar;i++)
				{
					if(s->cars[i]!=targetCar)
					{
						dis_sque.length = (targetCar->pub.trkPos.seg->lgfromstart + targetCar->race.laps * m_segLength) -
							(s->cars[i]->pub.trkPos.seg->lgfromstart + s->cars[i]->race.laps * m_segLength);
						dis_sque.width = targetCar->pub.trkPos.toLeft - s->cars[i]->pub.trkPos.toLeft;
					}
				}
				//记录
				m_distances.push_back(dis_sque);
				if(m_outfile.is_open()){							
					m_outfile<<"distances.length:"<<dis_sque.length<<endl;								
					m_outfile<<"distances.width:"<<dis_sque.width<<endl;
					m_outfile<<"m_distances.size()"<<m_distances.size()<<endl;
					m_outfile<<endl;
				}

				m_curTime = GfTimeClock();
			}	
		}
	}
}

void PassBasicJudge::figurOut(tCarElt *car)
{
	const char *race = m_ReInfo->_reRaceName;
	const int	BUFSIZE = 1024;
	char		path[BUFSIZE];
	int			demage = 0;
	vector<LengthInfo>::iterator it;
	unsigned int	time = m_distances.size();
	short		cons = 0;
	double		angle = 0;
	int			judgenum = m_distances.size();
	if (targetCar != NULL){
		float w1 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight1", NULL, 0);
		float w2 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight2", NULL, 0);
		float w3 = GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, "wight3", NULL, 0);
		for(unsigned int i = 25; i < m_distances.size(); i++)				//after 25s ,we begin to judge the game.
		{
			if(m_distances[i].length != 0)
				angle = m_distances[i].width / m_distances[i].length;
			if(angle < 0 && m_distances[i].length < -m_safeDistance ) 
				break;
			//static constant time which is the condition jumo into overtake judge
			if(angle < std::tan(m_angle) && angle > 0 && m_distances[i].length < m_safeDistance){						
				cons++;
				m_outfile<<"cons:"<<cons<<endl;
				
			}
			else{ 
				cons = 0;
			}
			if(cons == m_condition){			
				if (i >= m_distances.size()) 
					break;
				judgenum = (m_distances.size() -i + m_condition > GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, m_judgeNum, NULL, 0)) \
						   ? GfParmGetNum(m_ReInfo->params, RE_SECT_JUDGE, m_judgeNum, NULL, 0) : (m_distances.size() - i + m_condition);
				for(unsigned int j = i; j < judgenum + i; j++){
					if(m_distances[j].length != 0)
						angle = m_distances[j].width / m_distances[j].length;
					m_outfile<<"angle:"<<angle<<"   tan(m_angle):"<<std::tan(m_angle)<<endl;
					if(m_distances[j].length < -m_safeDistance ){
						m_outfile<<"break j:"<<j<<endl;
						break;		
					}
					if(angle < 0 && angle > -std::tan(m_angle) && m_distances[j].length < 0){
						time = (m_condition + j - i) < time ? (m_condition + j - i) : time;
						m_outfile<<"break time:"<<time<<endl;
						m_outfile<<"break j:"<<j<<endl;
						break;
					}

				}
				cons = 0;
			}

		}
		for(int i=0;i<m_nCar;i++){
			snprintf(path, BUFSIZE, "%s/%s/%s/%s/%d", m_ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
			if(strstr(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),"scr_server")!=0)
			  demage = (int)(GfParmGetNum(m_results, path, RE_ATTR_DAMMAGES, NULL, 0));
			m_outfile<<"i:"<<i<<",    demage:"<<demage<<endl;
			m_outfile<<GfParmGetStr(m_results, path, RE_ATTR_NAME, "")<<endl;
		}	

		judgenum = judgenum + m_condition;	
		m_outfile<<"judgenum:"<<judgenum<<endl;
		if(time == m_distances.size())
			time = judgenum + m_bestRecord;
		else
			time += 2;			//add the judging condition's times
		
		//the max speed is 300km/h ,the target car's speed is 80km/h ,so it must record m_bestRecord times in judge time.	
		score = w1 * m_fullScore + w2 * (1 - (float) (time - m_bestRecord) / judgenum) * m_fullScore - w3 * demage;		

		if(score < 0)
			score = 0;
	}
	/* 设置m_results */
	if(m_results!=NULL)
    {
        GfParmSetStr(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_NAME,name.c_str());
        GfParmSetStr(m_results, RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,factor.c_str());        
        GfParmSetNum(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_FACTOR_VAL, NULL, time);        
        GfParmSetNum(m_results, RE_SECT_JUDGE, RE_ATTR_JUDGE_SCORE, NULL, score);
    }
}

string PassBasicJudge::getJudgeName()
{
    return name;
}

string PassBasicJudge::getJudgeFactor()
{
    return factor;
}


void PassBasicJudge::showlable(void *rmScrHdle,int x8,int x9,int y)           
{
    m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        GfuiLabelCreateEx(rmScrHdle, m_judge_result,       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Pit",       m_fgcolor, GFUI_FONT_MEDIUM_C, x8, y, GFUI_ALIGN_HC_VB, 0);
    }
    
    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != -1){                
        GfuiLabelCreateEx(rmScrHdle, "Score",       m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HC_VB, 0);
    }
    else{
        GfuiLabelCreateEx(rmScrHdle, "Penalty",   m_fgcolor, GFUI_FONT_MEDIUM_C, x9, y, GFUI_ALIGN_HR_VB, 0); 
    }
}

short PassBasicJudge::resualt(void *rmScrHdle,int x8,int x9,int y,int i,char * buf,char * path)
{
	m_judge_result=GfParmGetStr(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR,"");
    if(m_judge_result[0]!='\0'){
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""))!=0){  //is judge car
            snprintf(buf, BUFSIZE, "%d", (int)(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_FACTOR_VAL,NULL,0)));
            GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
        else{    //is not judge car, replace 'x'
            GfuiLabelCreate(rmScrHdle, "x", GFUI_FONT_MEDIUM_C,
                x8, y, GFUI_ALIGN_HC_VB, 0);
        }
    }

    if(GfParmGetNum(m_results,RE_SECT_JUDGE,RE_ATTR_JUDGE_SCORE,NULL,0) != -1){            
        if(strcmp(GfParmGetStr(m_results, path, RE_ATTR_NAME, ""),GfParmGetStr(m_ReInfo->params, RE_SECT_JUDGE, m_targName,""))!=0){  //is judge car
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
