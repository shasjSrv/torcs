
/***************************************************************************

    file                 : target.cpp
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
#include "driver.h"
#include "target.h"
#include <portability.h>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define maxBOTS 10
#define BOTS 3

/* function prototypes */
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation * situation);
static void drive(int index, tCarElt* car, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static int  InitFuncPt(int index, void *pt);
static int  pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);


static const char* botname[maxBOTS] = {
	"target 1", "target 2", "target 3", "berniw 4", "berniw 5",
	"berniw 6", "berniw 7", "berniw 8", "berniw 9", "berniw 10"
};

static const char* botdesc[maxBOTS] = {
	"target 1", "target 2", "target 3", "berniw 4", "berniw 5",
	"berniw 6", "berniw 7", "berniw 8", "berniw 9", "berniw 10"
};



static Target *driver[BOTS];

/* Module entry point */
extern "C" int target(tModInfo *modInfo)
{
	//char	buffer[BUFSIZE];

	for (int i = 0; i < BOTS; i++) {
		modInfo[i].name = strdup(botname[i]);	/* name of the module (short) */
		modInfo[i].desc = strdup(botdesc[i]);	/* description of the module (can be long) */
		modInfo[i].fctInit = InitFuncPt;		/* init function */
		modInfo[i].gfId    = ROB_IDENT;			/* supported framework version */
		modInfo[i].index   = i+1;
	}
	return 0;
}


/* initialize function (callback) pointers for torcs */
static int InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;

	// Create robot instance for index.
	driver[index] = new Target(index);

	itf->rbNewTrack = initTrack;	/* init new track */
	itf->rbNewRace  = newRace;		/* init new race */
	itf->rbDrive    = drive;		/* drive during race */
	itf->rbShutdown	= shutdown;		/* called for cleanup per driver */
	itf->rbPitCmd   = pitcmd;		/* pit command */
	itf->index      = index;
	return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	driver[index]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
	driver[index]->newRace(car, s);
}

// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
	driver[index]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	return driver[index]->pitcmd(s);
}

// Called before the module is unloaded.
static void shutdown(int index)
{
	delete driver[index];
}


/**** implement of Target ******/

Target::Target(int index)
{
	berniw = NULL;
	bt = NULL;

	if(index!=3) 	//target1和target2采用berniw模型
		berniw = new Driver_berniw(index);
	else 			//target3采用bt模型
		bt = new Driver(index);
}

Target::~Target()
{
	if (berniw != NULL) {
		delete berniw;
		berniw = NULL;
	}
	if (bt != NULL) {
		delete bt;
		bt = NULL;
	}
}


/* initialize track data, called for every selected driver */
void Target::initTrack(tTrack* track, void *carHandle, void **carParmHandle, tSituation * situation)
{
	if(berniw != NULL )
		berniw->initTrack(track,carHandle,carParmHandle,situation);
	else if (bt != NULL)
		bt->initTrack(track,carHandle,carParmHandle,situation);
}


/* initialize driver for the race, called for every selected driver */
void Target::newRace(tCarElt* car, tSituation *situation)
{
	if(berniw != NULL )
		berniw->newRace(car,situation);
	else if (bt != NULL)
		bt->newRace(car,situation);
}

/* pitstop callback */
int Target::pitcmd(tSituation *s)
{
	int rt=0;
	if(berniw != NULL )
	    rt = berniw->pitcmd(s);
	else if (bt != NULL)
		rt = bt->pitCommand(s);
	return rt;
}

void Target::drive(tSituation *s)
{
	if(berniw != NULL )
		berniw->drive(s);
	else if (bt != NULL)
		bt->drive(s);
}
