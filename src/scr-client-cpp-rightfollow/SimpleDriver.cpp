/***************************************************************************
 
    file                 : SimpleDriver.cpp
    copyright            : (C) 2007 Daniele Loiacono
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "SimpleDriver.h"


/* Gear Changing Constants*/
const int SimpleDriver::gearUp[6]=
    {
        5000,6000,6000,6500,7000,0
    };
const int SimpleDriver::gearDown[6]=
    {
        0,2500,3000,3000,3500,3500
    };

/* Stuck constants*/
const int SimpleDriver::stuckTime = 2;
const float SimpleDriver::stuckAngle = .523598775; //PI/6
const int SimpleDriver::stuckBrakeTime = 2;

/* Accel and Brake Constants*/
const float SimpleDriver::maxSpeedDist=150;
const float SimpleDriver::maxSpeed=220;
const float SimpleDriver::sin5 = 0.08716;
const float SimpleDriver::cos5 = 0.99619;

/* Steering constants*/
const float SimpleDriver::steerLock=0.366519 ;
const float SimpleDriver::steerSensitivityOffset=80.0;
const float SimpleDriver::wheelSensitivityCoeff=0.8;

/* ABS Filter Constants */
const float SimpleDriver::wheelRadius[4]={0.3306,0.3306,0.3276,0.3276};
const float SimpleDriver::absSlip=2.0;
const float SimpleDriver::absRange=3.0;
const float SimpleDriver::absMinSpeed=3.0;

/* Clutch constants */
const float SimpleDriver::clutchMax=0.5;
const float SimpleDriver::clutchDelta=0.05;
const float SimpleDriver::clutchRange=0.82;
const float SimpleDriver::clutchDeltaTime=0.02;
const float SimpleDriver::clutchDeltaRaced=10;
const float SimpleDriver::clutchDec=0.01;
const float SimpleDriver::clutchMaxModifier=1.3;
const float SimpleDriver::clutchMaxTime=1.5;


int
SimpleDriver::getGear(CarState &cs)
{

    int gear = cs.getGear();
    int rpm  = cs.getRpm();

    // if gear is 0 (N) or -1 (R) just return 1 
    if (gear<1)
        return 1;
    // check if the RPM value of car is greater than the one suggested 
    // to shift up the gear from the current one     
    if (gear <6 && rpm >= gearUp[gear-1])
        return gear + 1;
    else
    	// check if the RPM value of car is lower than the one suggested 
    	// to shift down the gear from the current one
        if (gear > 1 && rpm <= gearDown[gear-1])
            return gear - 1;
        else // otherwhise keep current gear
            return gear;
}


float
SimpleDriver::getSteer(CarState &cs)
{
	// steering angle is compute by correcting the actual car angle w.r.t. to track 
	// axis [cs.getAngle()] and to adjust car position w.r.t to middle of track [cs.getTrackPos()*0.5]
    float targetAngle=(cs.getAngle()-cs.getTrackPos()*0.5);

	// reading of sensor at +5 degree w.r.t. car axis
    float rxSensor=cs.getTrack(10);
    // reading of sensor parallel to car axis
    float cSensor=cs.getTrack(9);
    // reading of sensor at -5 degree w.r.t. car axis
    float sxSensor=cs.getTrack(8);
	
	
	// 跟车
	float disSensor16=cs.getOpponents(16);
	float disSensor17=cs.getOpponents(17);
	float disSensor18=cs.getOpponents(18);
	float disSensor19=cs.getOpponents(19);
	float disSensor20=cs.getOpponents(20);

	if(disSensor18==200 && (disSensor16 < 200 || disSensor17 < 200 ) && cs.getTrackPos() < 0){
		calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
		cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&left"<<endl;
		return targetAngle/steerLock*0.5;
	}
	
	if(disSensor18==200 && (disSensor19 <200 || disSensor20 <200 ) && cs.getTrackPos() > 0){
		calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
		cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&right"<<endl;
		return targetAngle/steerLock*0.5;
	}

	// check if the car is going to be stucken slowly

	if(rxSensor>sxSensor && cSensor < 65 && (cs.getAngle()<0.001 || cs.getTrackPos()<0) ) 
		// approaching a turn on right
	{      	
		if( sxSensor < 3){
			calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
			return (targetAngle)/steerLock * 0.07;
		}
		else if( sxSensor < 10 ){
			calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
			return (targetAngle)/steerLock * 0.2;
		}
		calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
		return (targetAngle)/steerLock * 0.5;
	}
	else if(rxSensor<sxSensor && cSensor < 100 && (cs.getAngle()>-0.001 || cs.getTrackPos()>0) )
		// approaching a turn on left
	{
		if( rxSensor < 3){
			calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
			return (targetAngle)/steerLock * 0.07;
		}
		else if( rxSensor < 10 ){
			calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
			return (targetAngle)/steerLock * 0.2;
		}
		calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
		return (targetAngle)/steerLock * 0.5;
	}
 	else if (cs.getSpeedX() > steerSensitivityOffset) 
		// at high speed reduce the steering command to avoid loosing the control
        return targetAngle/(steerLock*(cs.getSpeedX()-steerSensitivityOffset)*wheelSensitivityCoeff);
    else
        return (targetAngle)/steerLock ;

}

void SimpleDriver::calLAngle(float rxSensor, float cSensor, float sxSensor, CarState &cs, float &targetAngle)
{
	float h = cSensor*sin5;
    float b = sxSensor - cSensor*cos5;
    float sinAngle = b*b/(h*h+b*b);
	targetAngle = targetAngle * 0.5 +  sinAngle*0.3;	
}

void SimpleDriver::calRAngle(float rxSensor, float cSensor, float sxSensor, CarState &cs, float &targetAngle)
{
 	float h = cSensor*sin5;
    float b = rxSensor - cSensor*cos5;
    float sinAngle = b*b/(h*h+b*b);//in fact, sinAngle is sina^2
	targetAngle = targetAngle * 0.5 - sinAngle*0.3;
}


float
SimpleDriver::getAccel(CarState &cs)
{
    // checks if car is out of track
    if (cs.getTrackPos() < 1 && cs.getTrackPos() > -1)
    {
        // reading of sensor at +5 degree w.r.t. car axis
        float rxSensor=cs.getTrack(10);
        // reading of sensor parallel to car axis
        float cSensor=cs.getTrack(9);
        // reading of sensor at -5 degree w.r.t. car axis
        float sxSensor=cs.getTrack(8);

        float targetSpeed;

        // track is straight and enough far from a turn so goes to max speed
        if (cSensor>maxSpeedDist || (cSensor>=rxSensor && cSensor >= sxSensor))
            targetSpeed = maxSpeed;
		else if(fabs(cs.getAngle()) < sin5 ){
			float h = cSensor*sin5;
            float b = max(rxSensor,sxSensor) - cSensor*cos5;
            float sinAngle = b*b/(h*h+b*b);
			targetSpeed = 2.2*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
		}
			else
        {
            // approaching a turn on right
            if(rxSensor>sxSensor)
            {

                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = rxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);//in fact, sinAngle is sina^2

				if(cSensor<40 && rxSensor-sxSensor<5)
					return maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);


                // estimate the target speed depending on turn and on how close it is
                targetSpeed = 1.8*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist); 
            }
            // approaching a turn on left
            else
            {

                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = sxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);

				if(cSensor<40 && sxSensor-rxSensor<5)
					return maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);

                // estimate the target speed depending on turn and on how close it is
                targetSpeed = 1.8*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
            }

        }

		if(cs.getCurLapTime() < 5)
			return  2/(1+exp(cs.getSpeedX() - targetSpeed)) - 1;

		
		// 跟车的加速与减速
		float disSensor16=cs.getOpponents(16);
		float disSensor17=cs.getOpponents(17);
		float disSensor18=cs.getOpponents(18);
		float disSensor19=cs.getOpponents(19);
		float disSensor20=cs.getOpponents(20);


		// 跟车超过目标车辆时的后方传感器
		float disSensor35=cs.getOpponents(35);
		float disSensor0=cs.getOpponents(0);
		float disSensor1=cs.getOpponents(1);
	
		if(disSensor35!=200 || disSensor0!=200 || disSensor1!=200)
			return -1; 
		
		
		float tempDis;

		float disSensor = min(disSensor17,disSensor18);
		disSensor = min(disSensor,disSensor19);
		disSensor = min(disSensor,disSensor20);
		disSensor = min(disSensor,disSensor16);

		cout << "*************************************************" << disSensor << endl;
		
		if(disSensor<30 || disSensor>50){
			if(miniDistance == 0)
				miniDistance=disSensor;
			if(miniDistance != 0)
				tempDis=disSensor;
		}

		if(disSensor<30 || disSensor>50)
			stuckBrake++;
		else
			stuckBrake=0;

		if(stuckBrake>stuckBrakeTime){				
			stuckBrake=0;
			tempDis-=miniDistance;
			float relaVel=tempDis/0.002;
				
			targetSpeed +=relaVel;
			
			if(disSensor<20)
				return -0.8;
			if(disSensor>50)
				targetSpeed = fabs(targetSpeed)*1.2;
			if(disSensor>80)
				return 1;
			miniDistance = 0;
			return  2/(1+exp(cs.getSpeedX() - targetSpeed)) - 1;
			
		}

        // accel/brake command is expontially scaled w.r.t. the difference between target speed and current one
        return 2/(1+exp(cs.getSpeedX() - targetSpeed)) - 1;
    }
    else
        return 0.5; // when out of track returns a moderate acceleration command

}

CarControl
SimpleDriver::wDrive(CarState cs)
{
	// check if car is currently stuck
	if ( fabs(cs.getAngle()) > stuckAngle )
    {
		// update stuck counter
        stuck++;
    }
    else
    {
    	// if not stuck reset stuck counter
        stuck = 0;
    }


	// after car is stuck for a while apply recovering policy
    if (stuck > stuckTime)
    {
    	/* set gear and sterring command assuming car is 
    	 * pointing in a direction out of track */
    	
    	// to bring car parallel to track axis
        float steer = - cs.getAngle() / steerLock; 
        int gear=-1; // gear R
        
        // if car is pointing in the correct direction revert gear and steer  
        if (cs.getAngle()*cs.getTrackPos()>0)
        {
            gear = 1;
            steer = -steer;
        }

        // Calculate clutching
        clutching(cs,clutch);

        // build a CarControl variable and return it
        CarControl cc (1.0,0.0,gear,steer,clutch);
        return cc;
    }

    else // car is not stuck
    {
    	// compute accel/brake command
        float accel_and_brake = getAccel(cs);
        // compute gear 
        int gear = getGear(cs);
        // compute steering
        float steer = getSteer(cs);

        // normalize steering
        if (steer < -1)
            steer = -1;
        if (steer > 1)
            steer = 1;
        
        // set accel and brake from the joint accel/brake command 
        float accel,brake;
        if (accel_and_brake>0)
        {
            accel = accel_and_brake;
            brake = 0;
        }
        else
        {
            accel = 0;
            // apply ABS to brake
            brake = filterABS(cs,-accel_and_brake);
        }

        // Calculate clutching
        clutching(cs,clutch);

        // build a CarControl variable and return it
        CarControl cc(accel,brake,gear,steer,clutch);
        return cc;
    }
}

float
SimpleDriver::filterABS(CarState &cs,float brake)
{

	// convert speed to m/s
	float speed = cs.getSpeedX() / 3.6;
	// when spedd lower than min speed for abs do nothing
    if (speed < absMinSpeed)
        return brake;
    
    // compute the speed of wheels in m/s
    float slip = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        slip += cs.getWheelSpinVel(i) * wheelRadius[i];
    }

    // slip is the difference between actual speed of car and average speed of wheels
    slip = speed - slip/4.0f;
    // when slip too high apply ABS
    if (slip > absSlip)
    {
        brake = brake - (slip - absSlip)/absRange;
    }
    
    // check brake is not negative, otherwise set it to zero
    if (brake<0)
    	return 0;
    else
    	return brake;
}

void
SimpleDriver::onShutdown()
{
    cout << "Bye bye!" << endl;
}

void
SimpleDriver::onRestart()
{
    cout << "Restarting the race!" << endl;
}

void
SimpleDriver::clutching(CarState &cs, float &clutch)
{
  double maxClutch = clutchMax;

  // Check if the current situation is the race start
  if (cs.getCurLapTime()<clutchDeltaTime  && stage==RACE && cs.getDistRaced()<clutchDeltaRaced)
    clutch = maxClutch;

  // Adjust the current value of the clutch
  if(clutch > 0)
  {
    double delta = clutchDelta;
    if (cs.getGear() < 2)
	{
      // Apply a stronger clutch output when the gear is one and the race is just started
	  delta /= 2;
      maxClutch *= clutchMaxModifier;
      if (cs.getCurLapTime() < clutchMaxTime)
        clutch = maxClutch;
	}

    // check clutch is not bigger than maximum values
	clutch = min(maxClutch,double(clutch));

	// if clutch is not at max value decrease it quite quickly
	if (clutch!=maxClutch)
	{
	  clutch -= delta;
	  clutch = max(0.0,double(clutch));
	}
	// if clutch is at max value decrease it very slowly
	else
		clutch -= clutchDec;
  }
}

void
SimpleDriver::init(float *angles)
{

	// set angles as {-90,-75,-60,-45,-30,20,15,10,5,0,5,10,15,20,30,45,60,75,90}

	for (int i=0; i<5; i++)
	{
		angles[i]=-90+i*15;
		angles[18-i]=90-i*15;
	}

	for (int i=5; i<9; i++)
	{
			angles[i]=-20+(i-5)*5;
			angles[18-i]=20-(i-5)*5;
	}
	angles[9]=0;
}
