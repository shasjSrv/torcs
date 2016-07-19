/***************************************************************************
 
    file                 : client.cpp
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
/* Uncomment the following lines under windows */
//#define WIN32 // maybe not necessary because already define
#define __DRIVER_CLASS__ SimpleDriver     // put here the name of your driver class
//#define __DRIVER_INCLUDE__ "SimpleDriver.h" // put here the filename of your driver header

#ifdef WIN32
#include <WinSock.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include __DRIVER_INCLUDE__

/*** defines for UDP *****/
#define UDP_MSGLEN 200*100*3 + 4 //1000
#define UDP_CLIENT_TIMEUOT 1000000
//#define __UDP_CLIENT_VERBOSE__
/************************/

#ifdef WIN32
typedef sockaddr_in tSockAddrIn;
#define CLOSE(x) closesocket(x)
#define INVALID(x) x == INVALID_SOCKET
#else
typedef int SOCKET;
typedef struct sockaddr_in tSockAddrIn;
#define CLOSE(x) close(x)
#define INVALID(x) x < 0
#endif

/************************* add by lz ************************/
#include <sys/shm.h> 
#define image_width 640
#define image_height 480
/*END********************* add by lz ************************/

class __DRIVER_CLASS__;
typedef __DRIVER_CLASS__ tDriver;

using namespace std;


//image functions
extern void createImage(int width, int height);
extern void releaseImage();
extern void processImage(const char camID, const unsigned char *imgBuf, int width, int height);


//void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,unsigned int &maxSteps,
//		bool &noise, double &noiseAVG, double &noiseSTD, long &seed, char *trackName, BaseDriver::tstage &stage);
void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
		  unsigned int &maxSteps, char *trackName, BaseDriver::tstage &stage, double &arg_p, double &arg_i, double &arg_d);


/************************add by lz****************************/
/****************data structure of share memory***************/
struct shared_use_st  
{  
    int written;
    uint8_t data[image_width*image_height*3]; 
    int control;
    int pause;
    double fast;

    double dist_L;
    double dist_R;

    double toMarking_L;
    double toMarking_M;
    double toMarking_R;

    double dist_LL;
    double dist_MM;
    double dist_RR;

    double toMarking_LL;
    double toMarking_ML;
    double toMarking_MR;
    double toMarking_RR;

    double toMiddle;
    double angle;
    double speed;

    double steerCmd;
    double accelCmd;
    double brakeCmd;
}; 

int* pwritten = NULL;
uint8_t* pdata = NULL;
int* pcontrol = NULL;
int* ppause = NULL;

double* psteerCmd_ghost = NULL;
double* paccelCmd_ghost = NULL;
double* pbrakeCmd_ghost = NULL;

double* pspeed_ghost = NULL;
double* ptoMiddle_ghost = NULL;
double* pangle_ghost = NULL;

double* pfast_ghost = NULL;

double* pdist_L_ghost = NULL;
double* pdist_R_ghost = NULL;

double* ptoMarking_L_ghost = NULL;
double* ptoMarking_M_ghost = NULL;
double* ptoMarking_R_ghost = NULL;

double* pdist_LL_ghost = NULL;
double* pdist_MM_ghost = NULL;
double* pdist_RR_ghost = NULL;

double* ptoMarking_LL_ghost = NULL;
double* ptoMarking_ML_ghost = NULL;
double* ptoMarking_MR_ghost = NULL;
double* ptoMarking_RR_ghost = NULL;

void *shm = NULL;
/****************data structure of share memory***************/
/*END********************add by lz****************************/

int main(int argc, char *argv[])
{
    SOCKET socketDescriptor;
    int numRead;

    char hostName[1000];
    unsigned int serverPort;
    char id[1000];
    unsigned int maxEpisodes;
    unsigned int maxSteps;
//    bool noise;
//    double noiseAVG;
//    double noiseSTD;
//    long seed;
    char trackName[1000];
    BaseDriver::tstage stage;

    tSockAddrIn serverAddress;
    struct hostent *hostInfo;
    struct timeval timeVal;
    fd_set readSet;
    char buf[UDP_MSGLEN];


#ifdef WIN32 
     /* WinSock Startup */

     WSADATA wsaData={0};
     WORD wVer = MAKEWORD(2,2);
     int nRet = WSAStartup(wVer,&wsaData);

     if(nRet == SOCKET_ERROR)
     {
 	std::cout << "Failed to init WinSock library" << std::endl;
	exit(1);
     }
#endif

//    parse_args(argc,argv,hostName,serverPort,id,maxEpisodes,maxSteps,noise,noiseAVG,noiseSTD,seed,trackName,stage);
    double arg_p=0;
    double arg_i=0;
    double arg_d=0;
    parse_args(argc,argv,hostName,serverPort,id,maxEpisodes,maxSteps,trackName,stage,arg_p,arg_i,arg_d);

//    if (seed>0)
//    	srand(seed);
//    else
//    	srand(time(NULL));

    hostInfo = gethostbyname(hostName);
    if (hostInfo == NULL)
    {
        cout << "Error: problem interpreting host: " << hostName << "\n";
        exit(1);
    }

    // Print command line option used
    cout << "***********************************" << endl;

    cout << "HOST: "   << hostName    << endl;

    cout << "PORT: " << serverPort  << endl;

    cout << "ID: "   << id     << endl;

    cout << "MAX_STEPS: " << maxSteps << endl; 

    cout << "MAX_EPISODES: " << maxEpisodes << endl;

//    if (seed>0)
//    	cout << "SEED: " << seed << endl;

    cout << "TRACKNAME: " << trackName << endl;

    if (stage == BaseDriver::WARMUP)
		cout << "STAGE: WARMUP" << endl;
	else if (stage == BaseDriver::QUALIFYING)
		cout << "STAGE:QUALIFYING" << endl;
	else if (stage == BaseDriver::RACE)
		cout << "STAGE: RACE" << endl;
	else
		cout << "STAGE: UNKNOWN" << endl;

	cout << "***********************************" << endl;
    // Create a socket (UDP on IPv4 protocol)
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID(socketDescriptor))
    {
        cerr << "cannot create socket\n";
        exit(1);
    }

    // Set some fields in the serverAddress structure.
    serverAddress.sin_family = hostInfo->h_addrtype;
    memcpy((char *) &serverAddress.sin_addr.s_addr,
           hostInfo->h_addr_list[0], hostInfo->h_length);
    serverAddress.sin_port = htons(serverPort);

    tDriver d(arg_p,arg_i,arg_d);
    strcpy(d.trackName,trackName);
    d.stage = stage;

    bool shutdownClient=false;
    unsigned long curEpisode=0;
    do
    {
        /***********************************************************************************
        ************************* UDP client identification ********************************
        ***********************************************************************************/
        do
        {
        	// Initialize the angles of rangefinders
        	float angles[19];
        	d.init(angles);
        	string initString = SimpleParser::stringify(string("init"),angles,19);
            cout << "Sending id to server: " << id << endl;
            initString.insert(0,id);
            cout << "Sending init string to the server: " << initString << endl;
            if (sendto(socketDescriptor, initString.c_str(), initString.length(), 0,
                       (struct sockaddr *) &serverAddress,
                       sizeof(serverAddress)) < 0)
            {
                cerr << "cannot send data ";
                CLOSE(socketDescriptor);
                exit(1);
            }

            // wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec
            FD_ZERO(&readSet);
            FD_SET(socketDescriptor, &readSet);
            timeVal.tv_sec = 0;
            timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

            if (select(socketDescriptor+1, &readSet, NULL, NULL, &timeVal))
            {
                // Read data sent by the solorace server
                memset(buf, 0x0, UDP_MSGLEN);  // Zero out the buffer.
                numRead = recv(socketDescriptor, buf, UDP_MSGLEN, 0);
                if (numRead < 0)
                {
                    cerr << "didn't get response from server...";
                }
		else
		{
                	cout << "Received: " << buf << endl;

                	if (strcmp(buf,"***identified***")==0)
                    		break;
            	}
	      }

        }  while(1);

        //create image
    	createImage(200,100);

        /***********************************add by lz ************************************/
        /*******************************init share memory*********************************/
        struct shared_use_st *shared = NULL;
        int shmid;  
        // establish memory sharing 
        shmid = shmget((key_t)4567, sizeof(struct shared_use_st), 0666|IPC_CREAT);  
        if(shmid == -1)  
        {  
            fprintf(stderr, "shmget failed\n");  
            exit(EXIT_FAILURE);  
        }  
  
        shm = shmat(shmid, 0, 0);  
        if(shm == (void*)-1)  
        {  
            fprintf(stderr, "shmat failed\n");  
            exit(EXIT_FAILURE);  
        }  
        printf("\n********** Memory sharing started, attached at %X **********\n \n", shm);  
        // set up shared memory 
        shared = (struct shared_use_st*)shm;  
        shared->written = 0;
        shared->control = 0;
        shared->pause = 1;    //0
        shared->fast = 0.0;
        
        shared->dist_L = 0.0;
        shared->dist_R = 0.0;

        shared->toMarking_L = 0.0;
        shared->toMarking_M = 0.0;
        shared->toMarking_R = 0.0;

        shared->dist_LL = 0.0;
        shared->dist_MM = 0.0;
        shared->dist_RR = 0.0;

        shared->toMarking_LL = 0.0;
        shared->toMarking_ML = 0.0;
        shared->toMarking_MR = 0.0;
        shared->toMarking_RR = 0.0;

        shared->toMiddle = 0.0;
        shared->angle = 0.0;
        shared->speed = 0.0;

        shared->steerCmd = 0.0;
        shared->accelCmd = 0.0;
        shared->brakeCmd = 0.0;

        pwritten=&shared->written;
        pdata=shared->data;
        pcontrol=&shared->control;
        ppause=&shared->pause;

        psteerCmd_ghost=&shared->steerCmd;
        paccelCmd_ghost=&shared->accelCmd;
        pbrakeCmd_ghost=&shared->brakeCmd;

        pspeed_ghost=&shared->speed;
        ptoMiddle_ghost=&shared->toMiddle;
        pangle_ghost=&shared->angle;

        pfast_ghost=&shared->fast;

        pdist_L_ghost=&shared->dist_L;
        pdist_R_ghost=&shared->dist_R;

        ptoMarking_L_ghost=&shared->toMarking_L;
        ptoMarking_M_ghost=&shared->toMarking_M;
        ptoMarking_R_ghost=&shared->toMarking_R;

        pdist_LL_ghost=&shared->dist_LL;
        pdist_MM_ghost=&shared->dist_MM;
        pdist_RR_ghost=&shared->dist_RR;

        ptoMarking_LL_ghost=&shared->toMarking_LL;
        ptoMarking_ML_ghost=&shared->toMarking_ML;
        ptoMarking_MR_ghost=&shared->toMarking_MR;
        ptoMarking_RR_ghost=&shared->toMarking_RR;

	    unsigned long currentStep=0; 

        int count=0;    //产生图计数（每0.1秒一个）
        /*******************************init share memory*********************************/
        /*END*******************************add by lz ************************************/

        while(1)
        {
            // wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec
            FD_ZERO(&readSet);
            FD_SET(socketDescriptor, &readSet);
            timeVal.tv_sec = 0;
            timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

            if (select(socketDescriptor+1, &readSet, NULL, NULL, &timeVal))
            {
                // Read data sent by the solorace server
                memset(buf, 0x0, UDP_MSGLEN);  // Zero out the buffer.
                numRead = recv(socketDescriptor, buf, UDP_MSGLEN, 0);
                if (numRead < 0)
                {
                    cerr << "didn't get response from server?";
                    CLOSE(socketDescriptor);
                    exit(1);
                }

#ifdef __UDP_CLIENT_VERBOSE__
                cout << "Received: " << buf << endl;
#endif

                if (strcmp(buf,"***shutdown***")==0)
                {
                    d.onShutdown();
                    shutdownClient = true;
                    cout << "Client Shutdown" << endl;
                    break;
                }

                if (strcmp(buf,"***restart***")==0)
                {
                    d.onRestart();
                    cout << "Client Restart" << endl;
                    break;
                }


		// receive image
                if(strncmp(buf,"cam",3)==0) {

                    //sscanf(buf+3,"%c",&camID);
                    //printf("Receive image of Camera %c, size %d\n",buf[3],numRead);
                    //process image.
                    // ...
                    //processImage(buf[3],(unsigned char *)(buf+4),200,100);
                    if(buf[3]==0){
                        if (*ppause == 1) 
                        { 
                            count++;
                            if (count>5)
                            {
                                count=1;

                                processImage(buf[3],(unsigned char *)(buf+4),200,100);  //产生pdata
                                *pwritten=1;

                                //while (*pwritten == 1)
                                    //usleep(1);
                            }
                        }
                    }
                    continue;
                }
                /**************************************************
                 * Compute The Action to send to the solorace sever
                 **************************************************/

		        if ( (++currentStep) != maxSteps)
		        {
                	    string action = d.drive(string(buf));
                	    memset(buf, 0x0, UDP_MSGLEN);
			            sprintf(buf,"%s",action.c_str());
		        }
		        else
			    sprintf (buf, "(meta 1)");
                
                    if (sendto(socketDescriptor, buf, strlen(buf)+1, 0,
                           (struct sockaddr *) &serverAddress,
                           sizeof(serverAddress)) < 0)
                    {
                        cerr << "cannot send data ";
                        CLOSE(socketDescriptor);
                        exit(1);
                    }
                    #ifdef __UDP_CLIENT_VERBOSE__
                    else
                        cout << "Sending " << buf << endl;
                    #endif
            }
            else
            {
                cout << "** Server did not respond in 1 second.\n";
            }
        }
    } while(shutdownClient==false && ( (++curEpisode) != maxEpisodes) );


    //release image
    releaseImage();

    /****************************** add by lz ******************************/
    if(shmdt(shm) == -1)  
    {  
        fprintf(stderr, "shmdt failed\n");  
        exit(EXIT_FAILURE);  
    }  
    printf("\n********** Memory sharing stopped. Good Bye! **********\n");  
    /****************************** add by lz ******************************/

    if (shutdownClient==false)
	d.onShutdown();
    CLOSE(socketDescriptor);
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}

//void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
//		  unsigned int &maxSteps,bool &noise, double &noiseAVG, double &noiseSTD, long &seed, char *trackName, BaseDriver::tstage &stage)
void parse_args(int argc, char *argv[], char *hostName, unsigned int &serverPort, char *id, unsigned int &maxEpisodes,
		  unsigned int &maxSteps, char *trackName, BaseDriver::tstage &stage, double &arg_p,double &arg_i, double &arg_d)
{
    int		i;

    // Set default values
    maxEpisodes=0;
    maxSteps=0;
    serverPort=3001;
    strcpy(hostName,"localhost");
    strcpy(id,"SCR");
//    noise=false;
//    noiseAVG=0;
//    noiseSTD=0.05;
//    seed=0;
    strcpy(trackName,"unknown");
    stage=BaseDriver::UNKNOWN;


    i = 1;
    while (i < argc)
    {
    	if (strncmp(argv[i], "host:", 5) == 0)
    	{
    		sprintf(hostName, "%s", argv[i]+5);
    		i++;
    	}
    	else if (strncmp(argv[i], "port:", 5) == 0)
    	{
    		sscanf(argv[i],"port:%d",&serverPort);
    		i++;
    	}
    	else if (strncmp(argv[i], "id:", 3) == 0)
    	{
    		sprintf(id, "%s", argv[i]+3);
    		i++;
	    }
    	else if (strncmp(argv[i], "maxEpisodes:", 12) == 0)
    	{
    		sscanf(argv[i],"maxEpisodes:%ud",&maxEpisodes);
    	    i++;
    	}
    	else if (strncmp(argv[i], "maxSteps:", 9) == 0)
    	{
    		sscanf(argv[i],"maxSteps:%ud",&maxSteps);
    		i++;
    	}
//    	else if (strncmp(argv[i], "seed:", 5) == 0)
//    	{
//    	    	sscanf(argv[i],"seed:%ld",&seed);
//    	    	i++;
//    	}
    	else if (strncmp(argv[i], "track:", 6) == 0)
    	{
    	    	sscanf(argv[i],"track:%s",trackName);
    	    	i++;
    	}
    	else if (strncmp(argv[i], "stage:", 6) == 0)
    	{
				int temp;
    		   	sscanf(argv[i],"stage:%d",&temp);
    		   	stage = (BaseDriver::tstage) temp;
    	    	i++;
    	    	if (stage<BaseDriver::WARMUP || stage > BaseDriver::RACE)
					stage = BaseDriver::UNKNOWN;
    	}
        else if (strncmp(argv[i], "p:", 2) == 0)
        {
            sscanf(argv[i],"p:%lf",&arg_p);
            i++;
        }
        else if (strncmp(argv[i], "i:", 2) == 0)
        {
            sscanf(argv[i],"i:%lf",&arg_i);
            i++;
        }
        else if (strncmp(argv[i], "d:", 2) == 0)
        {
            sscanf(argv[i],"d:%lf",&arg_d);
            i++;
        }
    	else {
    		i++;		/* ignore bad args */
    	}
    }
}
 
