/***************************************************************************

file                 : grtrafficlight.cpp
created              : Sun May 22 23:16:44 CET 2016
copyright            : (C) 2016 by Binson

 ***************************************************************************/



#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <plib/ssg.h>

#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <graphic.h>
#include <robottools.h>
#include <portability.h>
#include <math.h>
#include "grmain.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grsmoke.h"
#include "grcar.h"
#include "grcam.h"
#include "grscene.h"
#include "grboard.h"
#include "grssgext.h"
#include "grutil.h"
#include "grtrafficlight.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

extern ssgEntity *grssgLoadAC3D ( const char *fname, const ssgLoaderOptions* options );
double *lastTime;
int **switch_light;
ssgSimpleState	*greenlight= NULL;
ssgSimpleState	*redlight = NULL;
static ssgLoaderOptionsEx	options_1;

tgrTrafficlight * theTrafficlight = NULL;

int numberOfTrafficlight=0;
tRoadTrafficlight *theTrafficlightList=NULL;
void init_switch();

void ssgVtxTableTrafficlight::copy_from(ssgVtxTableTrafficlight *src, int clone_flags)
{
	ssgVtxTable::copy_from(src, clone_flags);
	size = src->size;
	on = src->on;
	pos[0] = src->pos[0];
	pos[1] = src->pos[1];
	pos[2] = src->pos[2];
}


ssgBase *ssgVtxTableTrafficlight::clone(int clone_flags)
{
	ssgVtxTableTrafficlight *b = new ssgVtxTableTrafficlight;
	b->copy_from(this, clone_flags);
	return b;
}


ssgVtxTableTrafficlight::ssgVtxTableTrafficlight()
{
}


ssgVtxTableTrafficlight::ssgVtxTableTrafficlight(ssgVertexArray *vtx, double s, sgVec3 p)
{
	gltype = GL_TRIANGLE_STRIP;
	type = ssgTypeVtxTable();
	size = s;
	on = 1;
	pos[0] = p[0];
	pos[1] = p[1];
	pos[2] = p[2];
	vertices = (vtx!=NULL) ? vtx : new ssgVertexArray();
	normals = new ssgNormalArray();
	texcoords = new ssgTexCoordArray();
	colours = new ssgColourArray();
	vertices->ref();
	normals->ref();
	texcoords->ref();
	colours->ref();

	recalcBSphere();
}

ssgVtxTableTrafficlight::~ssgVtxTableTrafficlight()
{
  /*  ssgDeRefDelete ( vertices  ) ;
      ssgDeRefDelete ( normals   ) ;
      ssgDeRefDelete ( texcoords ) ;
      ssgDeRefDelete ( colours   ) ; */
}



void ssgVtxTableTrafficlight::draw_geometry ()
{
	int num_normals = getNumNormals();
	float alpha;
	GLfloat modelView[16];
	sgVec3 A, B, C, D;
	sgVec3 right, up;
	sgVec3 axis;
	sgMat4 mat;
	sgMat4 mat3;
	sgVec3 *vx = (sgVec3 *) vertices->get(0);
	sgVec3 *nm = (sgVec3 *) normals->get(0);

	if (on == 0) {
		return;
	}

	alpha = 0.75f;
	glDepthMask(GL_FALSE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPolygonOffset(-15.0f, -20.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	// get the matrix.
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

	// get the up and right vector from the matrice view.
	up[0] = modelView[1];
	up[1] = modelView[5];
	up[2] = modelView[9];

	right[0] = modelView[0];
	right[1] = modelView[4];
	right[2] = modelView[8];

	// compute the coordinates of the four points of the quadri.

	// up and right points
	C[0] = right[0] + up[0];
	C[1] = right[1] + up[1];
	C[2] = right[2] + up[2];

	// left and up
	D[0] = -right[0] + up[0];
	D[1] = -right[1] + up[1];
	D[2] = -right[2] + up[2];

	// down and left
	A[0] = -right[0] - up[0];
	A[1] = -right[1] - up[1];
	A[2] = -right[2] - up[2];

	// right and down
	B[0] = right[0] - up[0];
	B[1] = right[1] - up[1];
	B[2] = right[2] - up[2];
	axis[0] = 0;
	axis[1] = 0;
	axis[2] = 1;

	if (maxTextureUnits > 1) {
		glActiveTextureARB (GL_TEXTURE0_ARB);
	}

	sgMakeRotMat4(mat, ((float)rand()/(float)RAND_MAX)*45, axis);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity ();
	sgMakeTransMat4(mat3, 0.5, 0.5, 0);
	glMultMatrixf((float *)mat3);
	glMultMatrixf((float *)mat);
	sgMakeTransMat4(mat3, -0.5, -0.5, 0);
	glMultMatrixf((float *)mat3);
	glMatrixMode(GL_MODELVIEW);

	glBegin(gltype) ;
	glColor4f(0.8, 0.8, 0.8, alpha);
	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}
	// the computed coordinates are translated from the smoke position with the x,y,z speed.
	glTexCoord2f(0, 0);
	glVertex3f(vx[0][0] + factor*size*A[0], vx[0][1] + factor*size*A[1], vx[0][2] + factor*size*A[2]);
	glTexCoord2f(0, 1);

	glVertex3f(vx[0][0] + factor*size*B[0], vx[0][1] + factor*size*B[1], vx[0][2] + factor*size*B[2]);
	glTexCoord2f(1, 0);

	glVertex3f(vx[0][0] + factor*size*D[0], vx[0][1] + factor*size*D[1], vx[0][2] + factor*size*D[2]);
	glTexCoord2f(1, 1);

	glVertex3f(vx[0][0]+factor*size*C[0],vx[0][1]+factor*size*C[1], vx[0][2]+factor*size*C[2]);

	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);

	if (maxTextureUnits > 1) {
		glActiveTextureARB (GL_TEXTURE0_ARB);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDepthMask(GL_TRUE);
}



void grInitTrafficlight(tTrack * theTrack)
{
    numberOfTrafficlight=theTrack->numberOfTrafficlight;
    theTrafficlightList=theTrack->theTrafficlightList;

    ssgEntity *trafficlightEntity=NULL;
	sgVec3 lightPos;
	int lightNum;
	const char *lightType;
	int lightTypeNum;
	const int BUFSIZE=256;
	char buf[BUFSIZE];
	int i,j = 0;
	theTrafficlight = (tgrTrafficlight *)malloc(sizeof(tgrTrafficlight)*numberOfTrafficlight);
    memset(theTrafficlight, 0, sizeof(tgrTrafficlight)*numberOfTrafficlight);
    tRoadTrafficlight *curTrafficlight= (tRoadTrafficlight*)calloc(1, sizeof(tRoadTrafficlight));

	if (!curTrafficlight) {
		GfFatal("grInitTraffilight: Memory allocation error");
	}
	if (!greenlight) {
		snprintf(buf, BUFSIZE, "data/textures;data/img;.");
		greenlight = (ssgSimpleState*)grSsgLoadTexStateEx("greenlight.rgb", buf, FALSE, FALSE);
		if (greenlight != NULL) {
			greenlight->disable(GL_LIGHTING);
			greenlight->enable(GL_BLEND);
			greenlight->disable(GL_CULL_FACE);
			greenlight->setTranslucent();
			greenlight->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
			greenlight->ref();
		}
	}

	if (!redlight) {
		snprintf(buf, BUFSIZE, "data/textures;data/img;.");
        redlight = (ssgSimpleState*)grSsgLoadTexStateEx("redlight.rgb", buf, FALSE, FALSE);
		if (redlight != NULL) {
			redlight->disable(GL_LIGHTING);
			redlight->enable(GL_BLEND);
			redlight->disable(GL_CULL_FACE);
			redlight->setTranslucent();
			redlight->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
			redlight->ref();
		}
	}

    init_switch();
	curTrafficlight= theTrafficlightList;
	for (i = 0; i < numberOfTrafficlight; i++) {
	    curTrafficlight= curTrafficlight->next;
		theTrafficlight[i].lightAnchor= new ssgBranch();

        lightNum = MAX_NUMBER_TRAFFIC_LIGHT;
    	for (j = 0; j < lightNum; j++) {
             lightPos[0] = -0.44;
             lightPos[1] = 0.22;
            if(j%2==0)
             lightPos[2] = 4.29;
            else
             lightPos[2] = 3.93;

            lightTypeNum=(j==0)?LIGHT_TYPE_GREEN:LIGHT_TYPE_RED;
            grAddTrafficlight(i, lightTypeNum, lightPos, 0.1);

    	}

        if(trafficlightEntity==NULL){
         	ssgSetCurrentOptions(&options_1);
         	ssgAddTextureFormat(".png", grLoadPngTexture);
         	grRegisterCustomSGILoader();
             /*Entity*/
         	snprintf(buf, BUFSIZE, "data/textures;data/img;.");
         	ssgTexturePath(buf);
            snprintf(buf, BUFSIZE, "data/objects;.");
         	ssgModelPath(buf);
            trafficlightEntity=grssgLoadAC3D("trafficlight.ac",NULL);
        }
        theTrafficlight[i].trafficlightTransform=new ssgTransform();
        theTrafficlight[i].trafficlightTransform->addKid(trafficlightEntity);
        sgMat4 m ;
        sgMakeIdentMat4 ( m ) ;

       /*position*/
        m[3][0]=curTrafficlight->pos.x;
        m[3][1]=curTrafficlight->pos.y;
        m[3][2]=curTrafficlight->pos.z;
       /*z-angle*/
        double angle=curTrafficlight->rot_angle_z*PI/180;
        m[0][0]=cos(angle);
        m[0][1]=sin(angle);
        m[1][0]=-sin(angle);
        m[1][1]=cos(angle);
        theTrafficlight[i].trafficlightTransform->setTransform(m);
        /*delta witch time*/
        theTrafficlight[i].delta_switch_time=curTrafficlight->delta_switch_time;

	    grLinkTrafficlights(i);

	}


}


void grShutdownTrafficlight(void)
{
	TrafficlightAnchor->removeAllKids();
	int i, j;
	for (i = 0; i < numberOfTrafficlight; i++) {
		for (j = 0; j < theTrafficlight[i].numberTrafficlight; j++) {
			ssgDeRefDelete(theTrafficlight[i].lightArray[j]);
		}
	}

	free(theTrafficlight);
	theTrafficlight=NULL;

	if (greenlight != NULL) {
		ssgDeRefDelete(greenlight);
		greenlight = NULL;
	}

	if (redlight != NULL) {
		ssgDeRefDelete(redlight);
		redlight = NULL;
	}
    theTrafficlightList=NULL;
    numberOfTrafficlight=0;
}


void grAddTrafficlight(int index, int type, sgVec3 pos, double size)
{
    const int BUFSIZE = 256;
	char buf[BUFSIZE];

	ssgVertexArray *light_vtx = new ssgVertexArray(1);

	light_vtx->add(pos);
	theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight] = new ssgVtxTableTrafficlight(light_vtx, size,pos);
	theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->ref();

	switch (type) {
        case LIGHT_TYPE_GREEN:
			theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setState(greenlight);
			theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setCullFace(0);
			break;
		case LIGHT_TYPE_RED:
			theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setState(redlight);
			theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setCullFace(0);
			break;
		default :
            theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setState(greenlight);
			theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->setCullFace(0);
			break;
	}

	theTrafficlight[index].lightType[theTrafficlight[index].numberTrafficlight] = type;
	theTrafficlight[index].lightCurr[theTrafficlight[index].numberTrafficlight] = (ssgVtxTableTrafficlight *)
		theTrafficlight[index].lightArray[theTrafficlight[index].numberTrafficlight]->clone(SSG_CLONE_GEOMETRY);

	theTrafficlight[index].lightAnchor->addKid(theTrafficlight[index].lightCurr[theTrafficlight[index].numberTrafficlight]);
	theTrafficlight[index].numberTrafficlight++;

        //clight->transform(m);
}


void grLinkTrafficlights(int index)
{
    theTrafficlight[index].trafficlightTransform->addKid(theTrafficlight[index].lightAnchor);
	TrafficlightAnchor->addKid(theTrafficlight[index].trafficlightTransform);
}


void grDrawTrafficlight(int index, int dispFlag, double curTime, class cGrPerspCamera *curCam)
{
	if (dispFlag != 1) {
		grUpdateTrafficlight(index, curCam, 0,curTime);
	} else {
		grUpdateTrafficlight(index, curCam, 1,curTime);
	}
}



void init_switch()
{
    lastTime=new double[numberOfTrafficlight];
    switch_light=new int *[numberOfTrafficlight];
    for (int i=0;i<numberOfTrafficlight;i++)
    {
        switch_light[i]=new int[MAX_NUMBER_TRAFFIC_LIGHT];
        switch_light[i][0]=1;
        switch_light[i][1]=0;
    }
}
void update_switch(int index, double curTime)
{
    if(fabs(curTime-lastTime[index])>theTrafficlight[index].delta_switch_time)
    {
        lastTime[index]=curTime;
        switch_light[index][LIGHT_TYPE_RED]=(switch_light[index][LIGHT_TYPE_RED]==0)?1:0;
        switch_light[index][LIGHT_TYPE_GREEN]=(switch_light[index][LIGHT_TYPE_GREEN]==0)?1:0;
    }
}
void grUpdateTrafficlight(int index,class cGrPerspCamera *curCam, int disp,double curTime)
{
    //printf("curTime is %f\n",curTime);
	int i = 0;
	ssgVtxTableTrafficlight	*clight;

	for (i = 0; i < theTrafficlight[index].numberTrafficlight; i++) {
		if (theTrafficlight[index].lightAnchor->getNumKids() != 0) {
			theTrafficlight[index].lightAnchor->removeKid(theTrafficlight[index].lightCurr[i]);
		}
	}


    update_switch(index,curTime);

    for (i = 0; i < theTrafficlight[index].numberTrafficlight; i++) {
		if (!disp) {
			continue;
		}
	clight = (ssgVtxTableTrafficlight *)theTrafficlight[index].lightArray[i]->clone(SSG_CLONE_GEOMETRY);
	clight->setCullFace(0);


		theTrafficlight[index].lightCurr[i]=clight;
		theTrafficlight[index].lightAnchor->addKid(clight);

        switch (theTrafficlight[index].lightType[i]) {
            case LIGHT_TYPE_RED:
                clight->setOnOff(switch_light[index][LIGHT_TYPE_RED]);
                break;
            case LIGHT_TYPE_GREEN:
                clight->setOnOff(switch_light[index][LIGHT_TYPE_GREEN]);
                break;
            default:
                break;
    }

		clight->setFactor(1);
	}
}

