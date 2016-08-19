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
const float SimpleDriver::degree5 = 0.08726;
const float SimpleDriver::degree15 = 0.26180;

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
SimpleDriver::normalSteer(CarState &cs)	
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

	if(rxSensor>sxSensor && cSensor < 65 && (cs.getAngle()<0.001 || cs.getTrackPos()<0) ) 
		// approaching a turn on right
	{      	
	if( sxSensor < 5){
			calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
			targetAngle *= 0.05;
		}
	else if( sxSensor < 10 ){
			calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
			targetAngle *= 0.2;
		}
		calRAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
		targetAngle *= 0.5;
	}
	else if(rxSensor<sxSensor && cSensor < 70 && (cs.getAngle()>-0.001 || cs.getTrackPos()>0) )
		// approaching a turn on left
	{
		if( rxSensor < 5){
			calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle);
			targetAngle *= 0.05;
		}
		else if( rxSensor < 10 ){
			calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
			targetAngle *= 0.2;
		}
		calLAngle(rxSensor, cSensor, sxSensor, cs, targetAngle) ;
		targetAngle *= 0.5;
	}
 	else if (cs.getSpeedX() > steerSensitivityOffset){ 

		cout << "****************************steer:" << 
		targetAngle/(steerLock*(cs.getSpeedX()-steerSensitivityOffset)*wheelSensitivityCoeff) << endl;;
		// at high speed reduce the steering command to avoid loosing the control
        return targetAngle/(steerLock*(cs.getSpeedX()-steerSensitivityOffset)*wheelSensitivityCoeff);
	}
    cout << "****************************steer:" <<  (targetAngle)/steerLock<<endl;
    return (targetAngle)/steerLock;
}

float 
SimpleDriver::overtakingSteer(CarState &cs)
{
 	float cSensor=cs.getTrack(9);

	// 超车
	float disSensor32=cs.getOpponents(32);
	float disSensor18=cs.getOpponents(18);
	float disSensor17=cs.getOpponents(17);
	float disSensor35=cs.getOpponents(35);
	float disSensor34=cs.getOpponents(34);
	float disSensor33=cs.getOpponents(33);

	if(isOvertaking)
		return normalSteer(cs);

	if( ( (disSensor18 > 20 && disSensor18 < 40) || (disSensor17 > 20 && disSensor17 < 40))
			&& !isTurning){
		if(followTime >= 10)
			isFollow = true;
		else
			isFollow = false;
		followTime = 0;
		doOvertaking = true;
		isOvertaking = false;
		isBehind = false;
		
	}

	if( (disSensor32 < 200)||  ( disSensor35 < 200) || (disSensor34 < 200) || (disSensor33 < 200))
		isBehind = true;

	if(cSensor < 70 && !isTurning ){
		if(isFollow)
			isFollow = false;
		if(doOvertaking)
			doOvertaking = false;
		return followSteer(cs);
	}
	//   float targetAngle=(cs.getAngle()-cs.getTrackPos()*0.5)

	if(!isFollow){
		cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%跟车"<<endl;
		followTime++;
		return followSteer(cs);
	}
	//已经跟过了,开始超车
	if(cs.getTrackPos() <= 0.6 && !isBehind)
	{
		cout<<"********************************************************左拐"<<endl;	
		//先靠左走一段
		isTurning = true;
	return (cs.getAngle()-(cs.getTrackPos()-0.6)*0.5);
	}
	if(!isBehind)
	{
		cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$正常"<<endl;
		//此时在左车道中央附近
		isTurning = false;
		return (cs.getAngle()-(cs.getTrackPos()-0.6)*0.5);
	}

	/*cout<<"********************************disSensor35:"<<disSensor35<<endl;
	cout<<"********************************disSensor34:"<<disSensor34<<endl;
	cout<<"********************************disSensor33:"<<disSensor33<<endl;
	cout<<"********************************disSensor32:"<<disSensor32<<endl;*/

	if(isBehind){
		if(cs.getTrackPos() > -0.3)
		{
			cout<<"((((((((((()))))))))))))))))))))))))))))右拐"<<endl;
			isTurning = true;
			return (cs.getAngle()-(cs.getTrackPos()+0.3)*0.5);
		}
		else
		{
			isTurning = false;
			isOvertaking = true;
			return normalSteer(cs);
		}
	}

}

float 
SimpleDriver::overtakingAccel(CarState &cs)
{
	if(isOvertaking)
		return normalAccel(cs);

	if(!isFollow)
		return followAccel(cs);

	float disSensor17=cs.getOpponents(17);
	float disSensor18=cs.getOpponents(18);


	if(doOvertaking)
	{
		if(disSensor18 < 35 || disSensor17 < 35)
			return followAccel(cs);
		else
			return  normalAccel(cs);
	}
	return normalAccel(cs);
}

float
SimpleDriver::getSteer(CarState &cs)
{
	return normalSteer(cs);
	return overtakingSteer(cs);
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
SimpleDriver::normalAccel(CarState &cs)
{
	float targetSpeed = 0;
	 if (cs.getTrackPos() < 1 && cs.getTrackPos() > -1)
    {
        // reading of sensor at +5 degree w.r.t. car axis
        float rxSensor=cs.getTrack(10);
        // reading of sensor parallel to car axis
        float cSensor=cs.getTrack(9);
        // reading of sensor at -5 degree w.r.t. car axis
        float sxSensor=cs.getTrack(8);

		// track is straight and enough far from a turn so goes to max speed
        if (cSensor>maxSpeedDist || (cSensor>=rxSensor && cSensor >= sxSensor))
            targetSpeed = maxSpeed;
		else if(fabs(cs.getAngle()) < sin5 ){
			float h = cSensor*sin5;
            float b = max(rxSensor,sxSensor) - cSensor*cos5;
            float sinAngle = b*b/(h*h+b*b);
			targetSpeed = 2.0*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
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

				targetSpeed = 1.5 * maxSpeed*(cSensor * sqrt(sinAngle)/maxSpeedDist);
            }
            // approaching a turn on left
            else
            {

                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = sxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);

				targetSpeed = 1.5*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
            }
		}
 				return 2/(1+exp(cs.getSpeedX() - targetSpeed)) - 1;
    }
    else
        return 0.5; // when out of track returns a moderate acceleration command
}


float 
SimpleDriver::followSteer(CarState &cs){

	float targetAngle=(cs.getAngle()-cs.getTrackPos()*0.5);


	// 跟车
	float disSensor16=cs.getOpponents(16);
	float disSensor17=cs.getOpponents(17);
	float disSensor18=cs.getOpponents(18);
	float disSensor19=cs.getOpponents(19);
	if((disSensor16 < 200 && cs.getTrackPos() < 0) || keepTime != 0){
		if(turnTime == 0)
			turnTime = cs.getCurLapTime();
		keepTime = turnTime - cs.getCurLapTime();
		if(keepTime >=1)
			keepTime = 0;
		if(disSensor16<200){
			targetAngle+=degree15*2;
		}
		else if(disSensor17<200){
			targetAngle+=degree5*2;
		}
		return targetAngle/steerLock;
	}
	
	if((disSensor19 <200 && cs.getTrackPos() > 0) || keepTime != 0){
		if(turnTime == 0)
			turnTime = cs.getCurLapTime();
		keepTime = turnTime - cs.getCurLapTime();
		if(keepTime >=1)
			keepTime = 0;
		if(disSensor19<200){
			targetAngle-=degree15*2;
		}
		else if(disSensor18<200){
			targetAngle-=degree5*2;
		}
		return targetAngle/steerLock;
	}
	return normalSteer(cs);
}

float 
SimpleDriver::followAccel(CarState &cs)
{
	float targetSpeed = 0;
	 if (cs.getTrackPos() < 1 && cs.getTrackPos() > -1)
    {
        // reading of sensor at +5 degree w.r.t. car axis
        float rxSensor=cs.getTrack(10);
        // reading of sensor parallel to car axis
        float cSensor=cs.getTrack(9);
        // reading of sensor at -5 degree w.r.t. car axis
        float sxSensor=cs.getTrack(8);

		// track is straight and enough far from a turn so goes to max speed
        if (cSensor>maxSpeedDist || (cSensor>=rxSensor && cSensor >= sxSensor))
            targetSpeed = maxSpeed;
		else if(fabs(cs.getAngle()) < sin5 ){
			float h = cSensor*sin5;
            float b = max(rxSensor,sxSensor) - cSensor*cos5;
            float sinAngle = b*b/(h*h+b*b);
			targetSpeed = 2.0*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
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

				targetSpeed = 1.8 * maxSpeed*(cSensor * sqrt(sinAngle)/maxSpeedDist);
            }
            // approaching a turn on left
            else
            {

                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = sxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);
				targetSpeed = 1.8*maxSpeed*(cSensor*sqrt(sinAngle)/maxSpeedDist);
            }
		}

		// 跟车的加速与减速
		float disSensor15=cs.getOpponents(15);
		float disSensor16=cs.getOpponents(16);
		float disSensor17=cs.getOpponents(17);
		float disSensor18=cs.getOpponents(18);
		float disSensor19=cs.getOpponents(19);
		float disSensor20=cs.getOpponents(20);
		float disSensor9=cs.getOpponents(9);
		float disSensor27=cs.getOpponents(27);

		// 跟车超过目标车辆时的后方传感器
		float disSensor35=cs.getOpponents(35);
		float disSensor0=cs.getOpponents(0);
		float disSensor1=cs.getOpponents(1);
	
		if(disSensor35!=200 || disSensor0!=200 || disSensor1!=200||
			disSensor9!=200 || disSensor27!=200)
			return -1; 
		
		
		float tempDis;

		float disSensor = min(disSensor17,disSensor18);
		disSensor = min(disSensor,disSensor19);
		disSensor = min(disSensor,disSensor16);
		disSensor = min(disSensor,disSensor15);
		disSensor = min(disSensor,disSensor20);

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
        return 0.1; // when out of track returns a moderate acceleration command		
}

float
SimpleDriver::getAccel(CarState &cs)
{
	return normalAccel(cs);
	return overtakingAccel(cs);
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
