/***************************************************************************

    file                 : berniw.h
    created              : Mon Mai 15 16:52:00 CET 2000
    copyright            : (C) 2000-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: berniw.h,v 1.16.2.1 2008/11/09 17:50:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _BERNIW_H_
#define _BERNIW_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>
#include <math.h>
#include "spline.h"
#include "trackdesc.h"
#include "mycar.h"
#include "pathfinder.h"

#define TRACKFILENAME		"/home/berni/track.dat"
#define TRACKDESCFILENAME   "/home/berni/track.txt"
#define RMAX				10000.0

#define BERNIW_SECT_PRIV	"berniw private"
#define BERNIW_ATT_PITENTRY	"pitentry"
#define BERNIW_ATT_PITEXIT	"pitexit"
#define BERNIW_ATT_AMAGIC	"caero"
#define BERNIW_ATT_FMAGIC	"cfriction"
#define BERNIW_ATT_FUELPERLAP "fuelperlap"

static const double g = 9.81;

class Driver_berniw {
	public:
		Driver_berniw(int index);
		~Driver_berniw();

		// Callback functions called from TORCS.
		void initTrack(tTrack* track, void *carHandle, void **carParmHandle, tSituation * situation);
		void newRace(tCarElt* car, tSituation *situation);
		void drive(tSituation *s);
		int pitcmd(tSituation *s);
		void endRace(tSituation *s);
	
	private:
		void drive_original(tSituation *s);
		void drive_normal(tSituation *s);
		void drive_follow(tSituation *s);		
		void update(tSituation *s);
		float Gear();
		float Steer();
		float Brake();
		float Accel(float brake);
		void Pit();
		int isStuck(tSituation *s);
		void getUnstuck();

		int index;
		tCarElt *car;			// Pointer to tCarElt struct.
		MyCar* myc;
		Pathfinder* mpf;
		OtherCar* ocar;
		TrackDesc* myTrackDesc;
		double currenttime;		
		int specialid[2];
		float limitspeed;

		tdble parallel;
		tdble cerror;
		static const tdble waitToTurn; /* how long should i wait till i try to turn backwards */
		static const float LIMITED_SPEED_NORMAL;
		static const float LIMITED_SPEED_FOLLOW;
		static const float ACCEL_NORMAL;

};


/* compute the angle between the track and the car */
inline double queryAngleToTrack(tCarElt * car)
{
	double angle = RtTrackSideTgAngleL(&(car->_trkPos)) - car->_yaw;
	NORM_PI_PI(angle);
	return angle;
}


inline double sign(double number)
{
	return (number >= 0.0) ? 1.0 : -1.0;
}


inline double sqr(double x)
{
	return x*x;
}


/* compute the radius given three points */
inline double radius(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double dx1 = x2 - x1;
	double dy1 = y2 - y1;
	double dx2 = x3 - x2;
	double dy2 = y3 - y2;

	//double z = (dy1*dx2 - dx1*dy2);
	double z = dx1*dy2 - dy1*dx2;

	if (z != 0.0) {
		double k = (dx2*(x3-x1) - dy2*(y1-y3))/z;
		return sign(z)*sqrt((1.0+k*k)*(dx1*dx1+dy1*dy1))/2.0;
	} else {
		return FLT_MAX;
	}
}

#endif // _BERNIW_H_

