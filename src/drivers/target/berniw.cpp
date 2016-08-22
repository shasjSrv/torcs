/***************************************************************************

    file                 : berniw.cpp
    created              : Mon Apr 17 13:51:00 CET 2000
    copyright            : (C) 2000-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: berniw.cpp,v 1.38.2.4 2013/08/05 17:22:43 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "berniw.h"
#include <portability.h>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define maxBOTS 10
#define BUFSIZE 255

static MyCar* mycar[maxBOTS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/**** implement of driver ******/

const tdble Driver_berniw::waitToTurn = 1.0; // how long should i wait till i try to turn backwards 
const float Driver_berniw::LIMITED_SPEED_NORMAL = 80.0;
const float Driver_berniw::LIMITED_SPEED_FOLLOW = 99.0;
const float Driver_berniw::ACCEL_NORMAL = 0.13;

Driver_berniw::Driver_berniw(int index)
{
	this->index = index;

	int i;
	for(i=0; i<maxBOTS; i++)
		mycar[i] = NULL;
	ocar = NULL;
	myTrackDesc = NULL;
	specialid[0] = specialid[1] = 0;
}

Driver_berniw::~Driver_berniw()
{
	int i = index - 1;
	if (mycar[i] != NULL) {
		delete mycar[i];
		mycar[i] = NULL;
		//free(botdesc[i]);
		//free(botname[i]);
	}
	if (myTrackDesc != NULL) {
		delete myTrackDesc;
		myTrackDesc = NULL;
	}
	if (ocar != NULL) {
		delete [] ocar;
		ocar = NULL;
	}
}


/* initialize track data, called for every selected driver */
void Driver_berniw::initTrack(tTrack* track, void *carHandle, void **carParmHandle, tSituation * situation)
{
	if ((myTrackDesc != NULL) && (myTrackDesc->getTorcsTrack() != track)) {
		delete myTrackDesc;
		myTrackDesc = NULL;
	}
	if (myTrackDesc == NULL) {
		myTrackDesc = new TrackDesc(track);
	}

	char buffer[BUFSIZE];
	char* trackname = strrchr(track->filename, '/') + 1;

	snprintf(buffer, BUFSIZE, "drivers/berniw/%d/%s", index, trackname);
    *carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);

	if (*carParmHandle == NULL) {
		snprintf(buffer, BUFSIZE, "drivers/berniw/%d/default.xml", index);
	    *carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	}

	/* Load and set parameters */
	float fuel = GfParmGetNum(*carParmHandle, BERNIW_SECT_PRIV, BERNIW_ATT_FUELPERLAP,
		(char*)NULL, track->length*MyCar::MAX_FUEL_PER_METER);
	fuel *= (situation->_totLaps + 1.0);
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, MIN(fuel, 100.0));

	if(index == 1)
	{
		myTrackDesc->SpecialIdgen(2);
		specialid[0] = myTrackDesc->getSpecialId(0);
		specialid[1] = myTrackDesc->getSpecialId(1);
	}
}


/* initialize driver for the race, called for every selected driver */
void Driver_berniw::newRace(tCarElt* car, tSituation *situation)
{
	this->car = car;
	if (ocar != NULL) delete [] ocar;
	ocar = new OtherCar[situation->_ncars];
	for (int i = 0; i < situation->_ncars; i++) {
		ocar[i].init(myTrackDesc, situation->cars[i], situation);
	}

	if (mycar[index-1] != NULL) delete mycar[index-1];
	mycar[index-1] = new MyCar(myTrackDesc, car, situation);
	myc = mycar[index-1];
	mpf = myc->getPathfinderPtr();
	currenttime = situation->currentTime;

	limitspeed = 300;
}

/* pitstop callback */
int Driver_berniw::pitcmd(tSituation *s)
{
	MyCar* myc = mycar[index-1];
	Pathfinder* mpf = myc->getPathfinderPtr();

	car->_pitFuel = MAX(MIN((car->_remainingLaps+1.0)*myc->fuelperlap - car->_fuel, car->_tank - car->_fuel), 0.0);
	myc->lastpitfuel = MAX(car->_pitFuel, 0.0);
	car->_pitRepair = car->_dammage;
	mpf->setPitStop(false, myc->getCurrentSegId());
	myc->loadBehaviour(myc->START);
	myc->startmode = true;
	myc->trtime = 0.0;

	return ROB_PIT_IM; /* return immediately */
}

void Driver_berniw::update(tSituation *s)
{
	/* update some values needed */
	myc->update(myTrackDesc, car, s);

	/* decide how we want to drive */
	if ( car->_dammage < myc->undamaged/3 && myc->bmode != myc->NORMAL) {
		myc->loadBehaviour(myc->NORMAL);
	} else if (car->_dammage > myc->undamaged/3 && car->_dammage < (myc->undamaged*2)/3 && myc->bmode != myc->CAREFUL) {
		myc->loadBehaviour(myc->CAREFUL);
	} else if (car->_dammage > (myc->undamaged*2)/3 && myc->bmode != myc->SLOW) {
		myc->loadBehaviour(myc->SLOW);
	}

	/* startmode */
	if (myc->trtime < 5.0 && myc->bmode != myc->START) {
		myc->loadBehaviour(myc->START);
		myc->startmode = true;
	}
	if (myc->startmode && myc->trtime > 5.0) {
		myc->startmode = false;
		myc->loadBehaviour(myc->NORMAL);
	}

	/* update the other cars just once */
	if (currenttime != s->currentTime) {
		currenttime = s->currentTime;
		for (int i = 0; i < s->_ncars; i++) ocar[i].update();
	}

	/* compute path according to the situation */
	mpf->plan(myc->getCurrentSegId(), car, s, myc, ocar);

	tdble cerrorh;
	cerrorh = sqrt(car->_speed_x*car->_speed_x + car->_speed_y*car->_speed_y);
	if (cerrorh > myc->TURNSPEED) cerror = fabs(car->_speed_x)/cerrorh; else cerror = 1.0;
}

float Driver_berniw::Steer()
{
	tdble steer, targetAngle;

	/* steer to next target point */
	targetAngle = atan2(myc->destpathseg->getLoc()->y - car->_pos_Y, myc->destpathseg->getLoc()->x - car->_pos_X);
	targetAngle -= car->_yaw;
	NORM_PI_PI(targetAngle);
    steer = targetAngle / car->_steerLock;

	if (!mpf->getPitStop()) {
		steer = steer + MIN(0.1, myc->derror*0.02)*myc->getErrorSgn();
		if (fabs(steer) > 1.0) steer/=fabs(steer);
	}

	/* try to control angular velocity */
	double omega = myc->getSpeed()/myc->currentpathseg->getRadius();
	steer += 0.1*(omega - myc->getCarPtr()->_yaw_rate);

	return steer;
}

float Driver_berniw::Gear()
{
	int gear;
	gear = car->_gear;
	// Gear changing.
	if (myc->tr_mode == 0) {
		if (car->_gear <= 0) {
			gear = 1;
		} else {
			float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
			float omega = car->_enginerpmRedLine/gr_up;
			float wr = car->_wheelRadius(2);

			if (omega*wr*myc->SHIFT < car->_speed_x) {
				gear = car->_gear + 1;
			} else {
				float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
				omega = car->_enginerpmRedLine/gr_down;
				if (car->_gear > 1 && omega*wr*myc->SHIFT > car->_speed_x + myc->SHIFT_MARGIN) {
					gear = car->_gear - 1;
				}
			}
		}
	}
	return gear;
}

float Driver_berniw::Brake()
{
	tdble brake;
	tdble b1;							/* brake value in case we are to fast HERE and NOW */
	tdble b2;							/* brake value for some brake point in front of us */
	tdble b3;							/* brake value for control (avoid loosing control) */
	tdble b4;							/* brake value for avoiding high angle of attack */
	tdble b5;							// Brake for the pit;
	b1 = b2 = b3 = b4 = b5 = 0.0;

	if (mpf->getPitStop()) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
		// Check if we are almost in the pit to set brake to the max to avoid overrun.
		tdble dl, dw;
		RtDistToPit(car, myTrackDesc->getTorcsTrack(), &dl, &dw);
		if (dl < 1.0f) {
			b5 = 1.0f;
		}
	}
	
    tdble brakecoeff = 1.0/(2.0*g*myc->currentseg->getKfriction()*myc->CFRICTION);
    tdble brakespeed, brakedist;
	tdble lookahead = 0.0;
	int i = myc->getCurrentSegId();
	brake = 0.0;

	while (lookahead < brakecoeff * myc->getSpeedSqr()) {
		lookahead += mpf->getPathSeg(i)->getLength();
		brakespeed = myc->getSpeedSqr() - mpf->getPathSeg(i)->getSpeedsqr();
		if (brakespeed > 0.0) {
			tdble gm, qb, qs;
			gm = myTrackDesc->getSegmentPtr(myc->getCurrentSegId())->getKfriction()*myc->CFRICTION*myTrackDesc->getSegmentPtr(myc->getCurrentSegId())->getKalpha();
			qs = mpf->getPathSeg(i)->getSpeedsqr();
			brakedist = brakespeed*(myc->mass/(2.0*gm*g*myc->mass + qs*(gm*myc->ca + myc->cw)));

			if (brakedist > lookahead - myc->getWheelTrack()) {
				qb = brakespeed*brakecoeff/brakedist;
				if (qb > b2) {
					b2 = qb;
				}
			}
		}
		i = (i + 1 + mpf->getnPathSeg()) % mpf->getnPathSeg();
	}

	if (myc->getSpeedSqr() > myc->currentpathseg->getSpeedsqr()) {
		b1 = (myc->getSpeedSqr() - myc->currentpathseg->getSpeedsqr()) / (myc->getSpeedSqr());
	}
	
	/* try to avoid flying */
	if (myc->getDeltaPitch() > myc->MAXALLOWEDPITCH && myc->getSpeed() > myc->FLYSPEED) {
		b4 = 1.0;
	}

	/* check if we are on the way */
	if (myc->getSpeed() > myc->TURNSPEED && myc->tr_mode == 0) {
		if (myc->derror > myc->PATHERR) {
			v3d r;
			myc->getDir()->crossProduct(myc->currentpathseg->getDir(), &r);
			if (r.z*myc->getErrorSgn() >= 0.0) {
				float targetAngle = atan2(myc->currentpathseg->getDir()->y, myc->currentpathseg->getDir()->x);
				targetAngle -= car->_yaw;
				NORM_PI_PI(targetAngle);
				double toborder = MAX(1.0, myc->currentseg->getWidth()/2.0 - fabs(myTrackDesc->distToMiddle(myc->getCurrentSegId(), myc->getCurrentPos())));
				b3 = (myc->getSpeed()/myc->STABLESPEED)*(myc->derror-myc->PATHERR)/toborder;
			}
		}
	}


	/* anti blocking and brake code */
	if (b1 > b2) brake = b1; else brake = b2;
	if (brake < b3) brake = b3;
	if (brake < b4) {
		brake = MIN(1.0, b4);
		tdble abs_mean;
		abs_mean = (car->_wheelSpinVel(REAR_LFT) + car->_wheelSpinVel(REAR_RGT))*car->_wheelRadius(REAR_LFT)/myc->getSpeed();
		abs_mean /= 2.0;
    	brake = brake * abs_mean;
	} else {
		brake = MIN(1.0, brake);
		tdble abs_min = 1.0;
		for (int i = 0; i < 4; i++) {
			tdble slip = car->_wheelSpinVel(i) * car->_wheelRadius(i) / myc->getSpeed();
			if (slip < abs_min) abs_min = slip;
		}
    	brake = brake * abs_min;
	}

	float weight = myc->mass*G;
	float maxForce = weight + myc->ca*myc->MAX_SPEED*myc->MAX_SPEED;
	float force = weight + myc->ca*myc->getSpeedSqr();
	brake = brake*MIN(1.0, force/maxForce);
	if (b5 > 0.0f) {
		brake = b5;
	}

	brake *= cerror;

	return brake;
}


float Driver_berniw::Accel(float brake)
{
	/* acceleration / brake execution */
	tdble accel=0.0, shiftaccel = 0.0;

	if (myc->tr_mode == 0) {
		if (brake > 0.0) {
			myc->accel = 0.0;
			accel = 0.0;
		} else {
			if (myc->getSpeedSqr() < mpf->getPathSeg(myc->getCurrentSegId())->getSpeedsqr()) {
				if (myc->accel < myc->ACCELLIMIT) {
					myc->accel += myc->ACCELINC;
				}
				accel = myc->accel/cerror;
			} else {
				if (myc->accel > 0.0) {
					myc->accel -= myc->ACCELINC;
				}
				myc->accel = MIN(myc->accel/cerror, shiftaccel/cerror);
				accel = myc->accel;
			}
			tdble slipspeed = myc->querySlipSpeed(car);
			if (slipspeed > myc->TCL_SLIP) {
				accel = accel - MIN(car->_accelCmd, (slipspeed - myc->TCL_SLIP)/myc->TCL_RANGE);
			}
		}
	}
	return accel;
}

int Driver_berniw::isStuck(tSituation *s)
{
	/* check if we are stuck, try to get unstuck */
	tdble bx = myc->getDir()->x, by = myc->getDir()->y;
	tdble cx = myc->currentseg->getMiddle()->x - car->_pos_X, cy = myc->currentseg->getMiddle()->y - car->_pos_Y;
	parallel = (cx*bx + cy*by) / (sqrt(cx*cx + cy*cy)*sqrt(bx*bx + by*by));

	if ((myc->getSpeed() < myc->TURNSPEED) && (parallel < cos(90.0*PI/180.0))  && (mpf->dist2D(myc->getCurrentPos(), mpf->getPathSeg(myc->getCurrentSegId())->getLoc()) > myc->TURNTOL)) {
		myc->turnaround += s->deltaTime;
	} else myc->turnaround = 0.0;

	if ((myc->turnaround >= waitToTurn) || (myc->tr_mode >= 1)) {
		if (myc->tr_mode == 0) {
			myc->tr_mode = 1;
			return 1;
		}
	}
	return 0;
}

void Driver_berniw::getUnstuck()
{
	//减速降档，直至减速为0，档位减为倒车档
	if ((car->_gearCmd > -1) && (myc->tr_mode < 2)) {
		car->_accelCmd = 0.0;
		if (myc->tr_mode == 1) //tr_mode=1: getUnstuck减速状态
		{
			car->_gearCmd--;
		}
		car->_brakeCmd = 1.0;
	} 
	else {
		myc->tr_mode = 2; 	//tr_mode=2: getUnstuck倒车状态

		//判断是否已拨正车头回到跑道上，若没有，倒车
		if (parallel < cos(90.0*PI/180.0) && (mpf->dist2D(myc->getCurrentPos(), mpf->getPathSeg(myc->getCurrentSegId())->getLoc()) > myc->TURNTOL)) {
			float angle = queryAngleToTrack(car);
			car->_steerCmd = ( -angle > 0.0) ? 1.0 : -1.0;
			car->_brakeCmd = 0.0;

			if (myc->accel < 1.0) {
				myc->accel += myc->ACCELINC;
			}
			car->_accelCmd = myc->accel;
			tdble slipspeed = myc->querySlipSpeed(car);
			if (slipspeed < -myc->TCL_SLIP) {
				car->_accelCmd = car->_accelCmd - MIN(car->_accelCmd, (myc->TCL_SLIP - slipspeed)/myc->TCL_RANGE);
			}
		} else //已拨正，getUnstuck完毕，准备正常起步 
		{
			if (myc->getSpeed() < 1.0) {
				myc->turnaround = 0;
				myc->tr_mode = 0; 	//tr_mode=0: 正常行驶状态
				myc->loadBehaviour(myc->START);
				myc->startmode = true;
				myc->trtime = 0.0;
			}
			car->_brakeCmd = 1.0;
			car->_steerCmd = 0.0;
			car->_accelCmd = 0.0;
		}
	}
}

void Driver_berniw::Pit()
{
	/* uncommenting the following line causes pitstop on every lap */
	//if (!mpf->getPitStop()) mpf->setPitStop(true, myc->getCurrentSegId());
	/* compute fuel consumption */
	if (myc->getCurrentSegId() >= 0 && myc->getCurrentSegId() < 5 && !myc->fuelchecked) {
		if (car->race.laps > 0) {
			myc->fuelperlap = MAX(myc->fuelperlap, (myc->lastfuel+myc->lastpitfuel-car->priv.fuel));
		}
		myc->lastfuel = car->priv.fuel;
		myc->lastpitfuel = 0.0;
		myc->fuelchecked = true;
	} else if (myc->getCurrentSegId() > 5) {
		myc->fuelchecked = false;
	}

	/* decide if we need a pit stop */
	if (!mpf->getPitStop() && (car->_remainingLaps-car->_lapsBehindLeader) > 0 && (car->_dammage > myc->MAXDAMMAGE ||
		(car->priv.fuel < 1.5*myc->fuelperlap &&
		 car->priv.fuel < (car->_remainingLaps-car->_lapsBehindLeader)*myc->fuelperlap)))
	{
		mpf->setPitStop(true, myc->getCurrentSegId());
	}

	if (mpf->getPitStop()) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
	}
}

/* controls the car */
void Driver_berniw::drive_original(tSituation *s)
{	

	update(s);

	/* clear ctrl structure with zero */
	memset(&car->ctrl, 0, sizeof(tCarCtrl));

	Pit();

	if (isStuck(s)) {
		getUnstuck();
	}
	else
	{
		car->_gearCmd = Gear();
		car->_steerCmd = Steer();
		float brake = Brake();
		car->_brakeCmd = brake;
		car->_accelCmd = Accel(brake);
	}
}


void Driver_berniw::drive_normal(tSituation *s)
{
	/* clear ctrl structure with zero */
	memset(&car->ctrl, 0, sizeof(tCarCtrl));

	if(s->currentTime>8 && s->currentTime<10)
	{
		limitspeed = LIMITED_SPEED_NORMAL;
	}

	float angle = RtTrackSideTgAngleL(&(car->_trkPos)) - car->_yaw;
	NORM_PI_PI(angle); // put the angle back in the range from -PI to PI
	angle -= 1.0 * (car->_trkPos.toMiddle/car->_trkPos.seg->width + 0.25);
	
	car->ctrl.steer = angle/car->_steerLock;

	car->ctrl.gear = Gear();    
	float accel = (limitspeed/3.6 - car->_speed_x)/MAX(limitspeed/3.6,car->_speed_x);

	if(accel>0)	
		car->ctrl.accelCmd = MIN(1.0,accel)+ACCEL_NORMAL;  // 加点油门以抵消摩擦力
	else
		car->ctrl.brakeCmd = MIN(1.0,-accel);
}

void Driver_berniw::drive_follow(tSituation *s)
{
	update(s);

	/* clear ctrl structure with zero */
	memset(&car->ctrl, 0, sizeof(tCarCtrl));

	Pit();
	
	if(s->currentTime<5)
	{
		limitspeed = LIMITED_SPEED_FOLLOW;
	}

	if (isStuck(s)) {
		getUnstuck();
	}
	else
	{
		car->_gearCmd = Gear();
		car->_steerCmd = Steer();
		float brake = Brake();
		
		//控制车速，单位m/s
		double b = 0;
		if(specialid[0]<=myc->getCurrentSegId()&&myc->getCurrentSegId()<=specialid[0]+300&&myc->getCurrentSegId()<specialid[1])
		{
			limitspeed = LIMITED_SPEED_FOLLOW*1.5;
		}
		else if (specialid[1]<=myc->getCurrentSegId()&&myc->getCurrentSegId()<=specialid[1]+150)
		{
			limitspeed = LIMITED_SPEED_FOLLOW*0.5;
		}
		else
		{
			limitspeed = LIMITED_SPEED_FOLLOW;
		}
		if (myc->getSpeed() > limitspeed/3.6) {
			b = (myc->getSpeed() - limitspeed/3.6)/ (myc->getSpeed());
			if(specialid[0]+150<=myc->getCurrentSegId()&&myc->getCurrentSegId()<=specialid[0]+200)
				b = 0.05;
		}
		if(b>brake)
			brake = b;

		car->_brakeCmd = brake;
		car->_accelCmd = Accel(brake);
	}
	

	//显示信息
	static int ii=-1;
	if(++ii==0)
	{
		int ntrackid = myTrackDesc->getnTrackSegments();
		printf("the number of track segment: %i \n",ntrackid);
		int specialid[2];
		specialid[0] = myTrackDesc->getSpecialId(0);
		specialid[1] = myTrackDesc->getSpecialId(1);
		printf("the two id of the spcial segment: %i, %i \n",specialid[0],specialid[1]);
	}

}

void Driver_berniw::drive(tSituation *s)
{
	switch(index)
	{
	case 1: drive_follow(s); break;
	case 2: drive_normal(s); break;
	default: drive_original(s); break;
	}
//	drive_original(s);
}
