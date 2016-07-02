/***************************************************************************

    file        : grtrafficlight.h
    created     : Sun May 22 15:15:11 CET 2016
    copyright   : (C) 2016 by Binson

 ***************************************************************************/


#ifndef _GRTRAFFICLIGHT_H_
#define _GRTRAFFICLIGHT_H_
#include <track.h>
class ssgVtxTableTrafficlight : public ssgVtxTable
{
protected:
  int on ;
  float size;
  sgVec3 pos;
  double factor;

  virtual void copy_from ( ssgVtxTableTrafficlight *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgVtxTableTrafficlight () ;
  ssgVtxTableTrafficlight ( ssgVertexArray   *vtx,
			double s, sgVec3 p);
  int setSize (double s) { size=s ; return 0; }
  void setOnOff (int s) { on=s ; }
  int isOnOff () { return(on) ; }
  sgVec3 * getPos() { return(&pos);}
  void setFactor(double f){factor=f;}

  void draw_geometry();


  virtual void drawHighlight ( sgVec4 colour ){ssgVtxTable::drawHighlight(colour);}
  virtual void drawHighlight ( sgVec4 colour, int i ){ssgVtxTable::drawHighlight(colour,i);}

  virtual void pick ( int baseName )  { ssgVtxTable::pick(baseName);}
  virtual void transform ( const sgMat4 m )  { ssgVtxTable::transform(m);}

  virtual void setVertices  ( ssgVertexArray   *vl ) {  ssgVtxTable::setVertices(vl);}
  virtual void setNormals   ( ssgNormalArray   *nl ) {  ssgVtxTable::setNormals(nl);}
  virtual void setTexCoords ( ssgTexCoordArray *tl ) {  ssgVtxTable::setTexCoords(tl);}
  virtual void setColours   ( ssgColourArray   *cl ) {  ssgVtxTable::setColours(cl);}

  int getNumVertices  () { return vertices  -> getNum () ; }
  int getNumNormals   () { return normals   -> getNum () ; }
  int getNumColours   () { return colours   -> getNum () ; }
  int getNumTexCoords () { return texcoords -> getNum () ; }
  int getNumTriangles ()  { return ssgVtxTable::getNumTriangles();}
  void getTriangle ( int n, short *v1, short *v2, short *v3 )  { ssgVtxTable::getTriangle(n,v1,v2,v3);}
  int  getNumLines ()  {return ssgVtxTable::getNumLines();}
  void getLine ( int n, short *v1, short *v2 )  { ssgVtxTable::getLine(n,v1,v2);}


  virtual ~ssgVtxTableTrafficlight (void);

  virtual const char *getTypeName(void)  { return ssgVtxTable::getTypeName();}

  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2) { ssgVtxTable::print(fd,indent,how_much);}
  virtual int load ( FILE *fd )  {return  ssgVtxTable::load(fd);}
  virtual int save ( FILE *fd )  {return  ssgVtxTable::save(fd);}


};

#define MAX_NUMBER_TRAFFIC_LIGHT 2
#define LIGHT_TYPE_GREEN 0
#define LIGHT_TYPE_RED   1
//#define TRAFFICLIGHT_SWITCH_TIME_DELTA 8.0

typedef struct tgrTrafficlight_t
{
  ssgVtxTableTrafficlight * lightArray[MAX_NUMBER_TRAFFIC_LIGHT];
  ssgVtxTableTrafficlight * lightCurr[MAX_NUMBER_TRAFFIC_LIGHT];
  //ssgEntity *trafficlightEntity;
  int lightType[MAX_NUMBER_TRAFFIC_LIGHT];
  int numberTrafficlight;
  double delta_switch_time;
  ssgBranch *lightAnchor;
  ssgTransform *trafficlightTransform;
}tgrTrafficlight;

extern void grInitTrafficlight(tTrack*);
extern void grAddTrafficlight(int index, int type, sgVec3 pos, double size);
extern void grLinkTrafficlights(int index);
extern void grDrawTrafficlight(int index, int dispFlag,  double curTime, class cGrPerspCamera *curCam);
extern void grUpdateTrafficlight(int index, class cGrPerspCamera *curCam, int dispflag, double curTime);
extern void grShutdownTrafficlight(void);

extern int numberOfTrafficlight;
extern tRoadTrafficlight *theTrafficlightList;

#endif /* _GRTRAFFICLIGHT_H_ */
