#ifndef _TARGET_H_
#define _TARGET_H_

#include "berniw.h"
#include "driver.h"

class Target
{
	public:	
		Target(int index);
		~Target();

		// Callback functions called from TORCS.
		void initTrack(tTrack* track, void *carHandle, void **carParmHandle, tSituation * situation);
		void newRace(tCarElt* car, tSituation *situation);
		void drive(tSituation *s);
		int pitcmd(tSituation *s);
		void endRace(tSituation *s);	

	private:
		Driver_berniw *berniw;
		Driver *bt;
};


#endif
