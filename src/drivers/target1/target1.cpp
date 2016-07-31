/***************************************************************************

    file                 : target1.cpp
    created              : 2016年 07月 29日 星期五 10:35:12 CST
    copyright            : (C) 2002 xxs

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include "target1.h"

static tTrack	*curTrack;

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *situation); 
static void newrace(int index, tCarElt* car, tSituation *s); 
static void drive(int index, tCarElt* car, tSituation *situation); 
static void endrace(int index, tCarElt *car, tSituation *s);
static void shutdown(int index);
static int  InitFuncPt(int index, void *pt); 


/* 
 * Module entry point  
 */ 
extern "C" int 
target1(tModInfo *modInfo) 
{
    memset(modInfo, 0, 10*sizeof(tModInfo));

    modInfo->name    = strdup("target1");		/* name of the module (short) */
    modInfo->desc    = strdup("target1 just run");	/* description of the module (can be long) */
    modInfo->fctInit = InitFuncPt;		/* init function */
    modInfo->gfId    = ROB_IDENT;		/* supported framework version */
    modInfo->index   = 1;

    return 0; 
} 

/* Module interface initialization. */
static int 
InitFuncPt(int index, void *pt) 
{ 
    tRobotItf *itf  = (tRobotItf *)pt; 

    itf->rbNewTrack = initTrack; /* Give the robot the track view called */ 
				 /* for every track change or new race */ 
    itf->rbNewRace  = newrace; 	 /* Start a new race */
    itf->rbDrive    = drive;	 /* Drive during race */
    itf->rbPitCmd   = NULL;
    itf->rbEndRace  = endrace;	 /* End of the current race */
    itf->rbShutdown = shutdown;	 /* Called before the module is unloaded */
    itf->index      = index; 	 /* Index used if multiple interfaces */
    return 0; 
} 

static MyCar* mycar[BOTS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static OtherCar* ocar = NULL;
static TrackDesc* myTrackDesc = NULL;
static double currenttime;
static const tdble waitToTurn = 1.0; /* how long should i wait till i try to turn backwards */
/* Called for every track change or new race. */

static void  
initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *situation) 
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
//	float fuel = GfParmGetNum(*carParmHandle, BERNIW_SECT_PRIV, BERNIW_ATT_FUELPERLAP,
//		(char*)NULL, track->length*MyCar::MAX_FUEL_PER_METER);  //取消注释target目标消失
//	fuel *= (situation->_totLaps + 1.0);
//	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, MIN(fuel, 100.0));
} 

/* Start a new race. */
static void  
newrace(int index, tCarElt* car, tSituation *s) 
{ 
} 

/* Called before the module is unloaded */
static void
shutdown(int index)
{
}

/* End of the current race */
static void
endrace(int index, tCarElt *car, tSituation *s)
{
}


/* Drive during race. */
static void  
drive(int index, tCarElt* car, tSituation *situation) 
{ 
	tdble angle;
	tdble brake;
	tdble b1;							/* brake value in case we are to fast HERE and NOW */
	tdble b2;							/* brake value for some brake point in front of us */
	tdble b3;							/* brake value for control (avoid loosing control) */
	tdble b4;							/* brake value for avoiding high angle of attack */
	tdble b5;							// Brake for the pit;
	tdble steer, targetAngle, shiftaccel;

	MyCar* myc = mycar[index-1];
	Pathfinder* mpf = myc->getPathfinderPtr();

	b1 = b2 = b3 = b4 = b5 = 0.0;
	shiftaccel = 0.0;

	/* update some values needed */
	myc->update(myTrackDesc, car, situation);

	/* decide how we want to drive */
	if ( car->_dammage < myc->undamaged/3 && myc->bmode != myc->NORMAL) {
		myc->loadBehaviour(myc->NORMAL);
	} else if (car->_dammage > myc->undamaged/3 && car->_dammage < (myc->undamaged*2)/3 && myc->bmode != myc->CAREFUL) {
		myc->loadBehaviour(myc->CAREFUL);
	} else if (car->_dammage > (myc->undamaged*2)/3 && myc->bmode != myc->SLOW) {
		myc->loadBehaviour(myc->SLOW);
	}

	/* update the other cars just once */
	if (currenttime != situation->currentTime) {
		currenttime = situation->currentTime;
		for (int i = 0; i < situation->_ncars; i++) ocar[i].update();
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

	/* compute path according to the situation */
	mpf->plan(myc->getCurrentSegId(), car, situation, myc, ocar);

	/* clear ctrl structure with zeros and set the current gear */
	memset(&car->ctrl, 0, sizeof(tCarCtrl));
	car->_gearCmd = car->_gear;

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
		// Check if we are almost in the pit to set brake to the max to avoid overrun.
		tdble dl, dw;
		RtDistToPit(car, myTrackDesc->getTorcsTrack(), &dl, &dw);
		if (dl < 1.0f) {
			b5 = 1.0f;
		}
	}

	/* steer to next target point */
	targetAngle = atan2(myc->destpathseg->getLoc()->y - car->_pos_Y, myc->destpathseg->getLoc()->x - car->_pos_X);
	targetAngle -= car->_yaw;
	NORM_PI_PI(targetAngle);
    steer = targetAngle / car->_steerLock;

	/* brakes */
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

	if (!mpf->getPitStop()) {
		steer = steer + MIN(0.1, myc->derror*0.02)*myc->getErrorSgn();
		if (fabs(steer) > 1.0) steer/=fabs(steer);
	}
	
	/* check if we are on the way */
/*	if (myc->getSpeed() > myc->TURNSPEED && myc->tr_mode == 0) {
		if (myc->derror > myc->PATHERR) {
			v3d r;
			myc->getDir()->crossProduct(myc->currentpathseg->getDir(), &r);
			if (r.z*myc->getErrorSgn() >= 0.0) {
//				targetAngle = atan2(myc->currentpathseg->getDir()->y, myc->currentpathseg->getDir()->x);//取消注释target目标消失
				targetAngle -= car->_yaw;
				NORM_PI_PI(targetAngle);
				double toborder = MAX(1.0, myc->currentseg->getWidth()/2.0 - fabs(myTrackDesc->distToMiddle(myc->getCurrentSegId(), myc->getCurrentPos())));
				b3 = (myc->getSpeed()/myc->STABLESPEED)*(myc->derror-myc->PATHERR)/toborder;
			}
		}
	}
*/
	/* try to control angular velocity */
	double omega = myc->getSpeed()/myc->currentpathseg->getRadius();
	steer += 0.1*(omega - myc->getCarPtr()->_yaw_rate);

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
	
// Gear changing.
/*	if (myc->tr_mode == 0) {
		if (car->_gear <= 0) {
			car->_gearCmd =  1;
		} else {
			float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
			float omega = car->_enginerpmRedLine/gr_up;
			float wr = car->_wheelRadius(2);

			if (omega*wr*myc->SHIFT < car->_speed_x) {
				car->_gearCmd++;
			} else {
				float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
				omega = car->_enginerpmRedLine/gr_down;
				if (car->_gear > 1 && omega*wr*myc->SHIFT > car->_speed_x + myc->SHIFT_MARGIN) {
					car->_gearCmd--;
				}
			}
		}
	}*/




}




