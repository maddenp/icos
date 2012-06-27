// Copyright 2012 Paul Madden (maddenp@colorado.edu)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Based on CSCI 5229 (University of Colorado at Boulder) class project

#define EARTHS 3
#define FONT GLUT_BITMAP_8_BY_13
#define GL_GLEXT_PROTOTYPES
#define GRIDS 5
#define PI 3.14159265

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

struct T // triangles
{
  double v[3][3];              // vertices
  double n[3];                 // normal
  double c[3];                 // centroid
};

struct G // grid
{
  struct T* Tp;                // pointer to triangles
  int nTs;                     // number of triangles
};
  
// colors

double black[4]={0,0,0,1};
double grey[4]={0.5,0.5,0.5,1};
double magenta[4]={1,0,1,1};
double orange[4]={1,.27,0,1};
double red[4]={1,0,0,1};
double springgreen[4]={0,1,.5,1};
double white[4]={1,1,1,1};
double yellow[4]={1,1,0,1};

// global variables

double ar=1;                   // aspect ratio
double defearthalpha=.75;      // default transparency of globe overlay
double *deffc=grey;            // default triangle face color
double dim=2.5;                // size for ortho box
double earthalpha;             // current transparency of globe overlay
double facecolor[4];           // color for geodesic faces
double lasttime=0;             // keep track of time for animation
double p;                      // special icosahedron coordinate
double radius=0;               // distance from origin to vertex
double th=0,ph=0,la=0;         // display/light angles
double vertex[12][3];          // storage for initial icos vertices
int animatem=1;                // animation mode: 0 => instant, 1 => animated
int animaten=0;                // to remember this grid's number of triangles
int animatep=0;                // is an animation active?
int animates=0;                // animation stage: 1 => bisection done
int axesp=0;                   // whether to show axes
int centroidsp=0;              // display centroids?
int edgesp=1;                  // show triangle-face edges?
int fixedp=0;                  // do not rotate during refinement?
int fov=55;                    // field of view for perspective
int level=0;                   // current grid level
int levels=GRIDS;              // max grid level allowed
int normalsp=0;                // draw all normals? (0 => disable)
int play=1;                    // auto-play
int projmode=0;                // orthogonal (0) vs perspective (1)
int refinem=0;                 // refine mode: 0 => 2-step, 1 => 1-step
int spherep=1;                 // show translucent sphere?
int textp=1;                   // display text?
int texturen=1;                // which texture? 0 => none
struct G grid[GRIDS];          // storage for generated grids
unsigned int textures[EARTHS]; // opaque handle for texture

// function prototypes

double distance(double,double,double,double,double,double);
void bisect();
void centroid(double [3][3],struct T *);
void die(char *);
void display();
void downgrid();
void drawaxes();
void drawcentroids();
void drawchars();
void drawgrid(int,double [3],double [3]);
void drawnormals();
void drawtext();
void errorcheck();
void extend_vertex(struct T*);
void extend_vertices();
void icosahedron();
void idle();
void init();
void key(unsigned char,int,int);
void loadtextures();
void midpoints(struct T *,double [3][3]);
void normal(struct T *,double [3][3]);
void project();
void refine();
void reshape(int,int);
void rotate_la(double);
void rotate_ph(double);
void rotate_th(double);
void set_ns_and_cs();
void setfc(double *);
void shellsphere();
void special(int,int,int);
void spherev(double,double);
void upgrid();

// functions

void animate()
{
  // called by idle() - progressive draw new grid
  int speedparam=160;
  if (animatep==1)
  {
    animaten=grid[level].nTs;
    grid[level].nTs=1;
    facecolor[0]=1;
    facecolor[1]=0;
    facecolor[2]=0;
    earthalpha=.25;
    animatep=2;
  }
  if (animatep==2)
  {
    if (!fixedp) rotate_th((360.0/animaten)/3.0);
    grid[level].nTs+=(animaten/speedparam)<1?1:(animaten/speedparam);
    if (grid[level].nTs>animaten)
    {
      grid[level].nTs=animaten;
      animatep=0;
    }
  }
}

void bisect()
{
  // bisect the faces of triangles to produce new triangles
  double m[3][3];
  int i;
  int nTsold=grid[level-1].nTs;
  int nTsnew=4*nTsold;
  struct T *Tpold=grid[level-1].Tp;
  struct T *Tpnew=(struct T *)malloc(nTsnew*sizeof(struct T));
  if (!Tpnew) die("Cannot malloc space for bisection triangles.");
  grid[level].Tp=Tpnew;
  grid[level].nTs=nTsnew;  
  for (i=0;i<nTsold;i++,Tpnew+=4)
  {
    midpoints(&Tpold[i],m);
    // new triangle 1
    Tpnew[0].v[0][0]=Tpold[i].v[0][0];
    Tpnew[0].v[0][1]=Tpold[i].v[0][1];
    Tpnew[0].v[0][2]=Tpold[i].v[0][2];
    Tpnew[0].v[1][0]=m[0][0];
    Tpnew[0].v[1][1]=m[0][1];
    Tpnew[0].v[1][2]=m[0][2];
    Tpnew[0].v[2][0]=m[2][0];
    Tpnew[0].v[2][1]=m[2][1];
    Tpnew[0].v[2][2]=m[2][2];
    // new triangle 2    
    Tpnew[1].v[0][0]=m[0][0];
    Tpnew[1].v[0][1]=m[0][1];
    Tpnew[1].v[0][2]=m[0][2];
    Tpnew[1].v[1][0]=Tpold[i].v[1][0];
    Tpnew[1].v[1][1]=Tpold[i].v[1][1];
    Tpnew[1].v[1][2]=Tpold[i].v[1][2];
    Tpnew[1].v[2][0]=m[1][0];
    Tpnew[1].v[2][1]=m[1][1];
    Tpnew[1].v[2][2]=m[1][2];
    // new triangle 3        
    Tpnew[2].v[0][0]=m[2][0];
    Tpnew[2].v[0][1]=m[2][1];
    Tpnew[2].v[0][2]=m[2][2];
    Tpnew[2].v[1][0]=m[1][0];
    Tpnew[2].v[1][1]=m[1][1];
    Tpnew[2].v[1][2]=m[1][2];
    Tpnew[2].v[2][0]=Tpold[i].v[2][0];
    Tpnew[2].v[2][1]=Tpold[i].v[2][1];
    Tpnew[2].v[2][2]=Tpold[i].v[2][2];
    // new triangle 4        
    Tpnew[3].v[0][0]=m[0][0];
    Tpnew[3].v[0][1]=m[0][1];
    Tpnew[3].v[0][2]=m[0][2];
    Tpnew[3].v[1][0]=m[1][0];
    Tpnew[3].v[1][1]=m[1][1];
    Tpnew[3].v[1][2]=m[1][2];
    Tpnew[3].v[2][0]=m[2][0];
    Tpnew[3].v[2][1]=m[2][1];
    Tpnew[3].v[2][2]=m[2][2];
  }
  set_ns_and_cs();
  animates=1;
}

void centroid(double m[3][3],struct T* Tp)
{
  // find the centroid of a triangle
  Tp->c[0]=(m[0][0]+m[1][0]+m[2][0])/3;
  Tp->c[1]=(m[0][1]+m[1][1]+m[2][1])/3;
  Tp->c[2]=(m[0][2]+m[1][2]+m[2][2])/3;
}

void die(char *msg)
{
  // print informative message and exit with error code
  printf("%s\n",msg);
  exit(1);
}

void display()
{
  // process visual elements
  double ex=0,ey=0,ez=0,m=PI/180;
  float lightradius=6;
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST); // enable z-buffer
  glLoadIdentity();
  if (projmode)
  {
    // move eye to viewpoint for projection mode
    ex=-2*dim*sin(th*m)*cos(ph*m);
    ey=2*dim*sin(ph*m);
    ez=2*dim*cos(th*m)*cos(ph*m);
    gluLookAt(ex,ey,ez,0,0,0,0,cos(ph*m),0);
  }
  else
  {
    // rotate scene for orthogonal mode
    glRotated(th,0,1,0);
    glRotated(-ph,1,0,0);
  }
  // set up lighting
  glPushMatrix();
  float lightpos[]={lightradius*cos(la*m),0,lightradius*sin(la*m),1};
  glTranslated(lightpos[0],lightpos[1],lightpos[2]);
  glPopMatrix();
  glEnable(GL_LIGHTING);
  float ambient[]={0.25,0.25,0.25,1.0};
  float diffuse[]={0.5,0.5,0.5,1.0};
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
  glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
  glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
  glEnable(GL_LIGHT0);
  // draw geodesic grid
  if (animatep) // if animation is enabled...
  {
    if (animates) // bisection is done: draw extended grid
    {
      setfc(yellow);
      drawgrid(level-1,yellow,black);
      drawgrid(level,yellow,red);
    }
    else // draw bisected grid
    {
      drawgrid(level-1,yellow,black);
      drawgrid(level,red,black);
    }
  }
  else
    if (animates) // bisection done, no animation      
      drawgrid(level,facecolor,red);
    else // extension done, no animation
      drawgrid(level,facecolor,black);
  if (centroidsp) drawcentroids(); // draw centroids (maybe)
  if (normalsp) drawnormals();     // draw normals (maybe)
  glDepthMask(0);                  // make z-buffer read-only
// glDisable(GL_DEPTH_TEST); // w/o this, weird splotches at some vertices at g3+
  if (spherep) shellsphere();      // show translucent sphere (maybe)
  glDepthMask(1);                  // make z-buffer read/write
  glDisable(GL_LIGHTING);          // turn off lighting
  glDisable(GL_DEPTH_TEST);        // disable z-buffer
  if (textp) drawtext();           // draw text (maybe)
  if (axesp) drawaxes();           // draw axes (maybe)
  glFlush();                       // get stuff drawn now
  glutSwapBuffers();               // enable redrawn buffer
  errorcheck();                    // see if we encountered any errors
}

double distance(double x1,double y1,double z1,double x2,double y2,double z2)
{
  // distance between two 3D points
  double dx=x2-x1;
  double dy=y2-y1;
  double dz=z2-z1;
  return sqrt(dx*dx+dy*dy+dz*dz);
}

void drawaxes()
{
  // draw x/y/z axes with labels
  float m=2;
  glColor3dv(orange);
  glBegin(GL_LINES);
  {
    glVertex3i(0,0,0);
    glVertex3i(m,0,0);
    glVertex3i(0,0,0);
    glVertex3i(0,m,0);
    glVertex3i(0,0,0);
    glVertex3i(0,0,m);
  }
  glEnd();
  glRasterPos3i(m,0,0);
  glutBitmapCharacter(FONT,'X');
  glRasterPos3i(0,m,0);
  glutBitmapCharacter(FONT,'Y');
  glRasterPos3i(0,0,m);
  glutBitmapCharacter(FONT,'Z');
}

void drawcentroids()
{
  // show centroids of triangles
  int i;
  struct T *Tp=grid[level].Tp;
  glColor3dv(springgreen);
  for (i=0;i<grid[level].nTs;i++)
  {
    glPushMatrix();
    glTranslated(Tp[i].c[0],Tp[i].c[1],Tp[i].c[2]);
    glutSolidSphere(.05,10,10);
    glPopMatrix();
  }
}

void drawchars(char *str,int ypos)
{
  // draw characters to screen at given y position
  char *c;
  glWindowPos2i(5,ypos);
  for (c=str;*c;c++)
    glutBitmapCharacter(FONT,*c);
}

void drawgrid(int lvl,double facec[3],double edgec[3])
{
  // construct geodesic grid from triangles
  int i,j;
  struct T *Tp=grid[lvl].Tp;
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  for (i=0;i<grid[lvl].nTs;i++)
  {
    // draw triangle
    glColor3dv(facec);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    glBegin(GL_POLYGON);
    glNormal3dv(Tp[i].n);
    for (j=0;j<3;j++)
      glVertex3dv(Tp[i].v[j]);
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
    // draw edges
    if (edgesp)
    {
      glColor3dv(edgec);
      glBegin(GL_LINE_LOOP);
      for (j=0;j<3;j++)
        glVertex3dv(Tp[i].v[j]);
      glEnd();
    }
  }
}

void drawnormals()
{
  // show normals to polyhedron faces
  int i,j;
  double cn[3];
  struct T *Tp=grid[level].Tp;
  glColor3dv(magenta);
  for (i=0;i<grid[level].nTs;i++)
  {
    glBegin(GL_LINES);
    {
      glVertex3d(Tp[i].c[0],Tp[i].c[1],Tp[i].c[2]);
      for (j=0;j<3;j++)
        cn[j]=Tp[i].c[j]+(Tp[i].n[j]/2);
      glVertex3d(cn[0],cn[1],cn[2]);
    }
    glEnd();
  }
}

void drawtext()
{
  // show some text in lower-left corner
  char str[1000];
  glColor3dv(white);
  if (level==0)
    sprintf(str,"grid level 0 - initial icosahedron");
  else if (animates)
    sprintf(str,"grid level %d - %s",level,animatep?"bisecting...":"bisected");
  else
    sprintf(str,"grid level %d - %s",level,animatep?"extending...":"complete");
  drawchars(str,55);
  sprintf(str,"[a]xes %s | [c]entroids %s | [e]dges %s | [f]ixed %s | [g]o %s | ani[m]ate %s",
          axesp?"+":"-",centroidsp?"+":"-",edgesp?"+":"-",fixedp?"+":"-",
          play?"+":"-",animatem?"+":"-");
  drawchars(str,35);
  sprintf(str,"[n]ormals %s | [r]efine %s | [s]phere %s | [t]exture %d",
          normalsp?"+":"-",refinem?"1-step":"2-step",spherep?"+":"-",texturen); 
  drawchars(str,20);
  sprintf(str,"zoom: [+-] | grid [<>] | rotate: arrows | reset angles: [0] | quit: <esc>");
  drawchars(str,5);
}

void downgrid()
{
  // reduce grid refinement & deallocate memory
  animatep=0;
  animates=0;
  free(grid[level].Tp);
  grid[level].Tp=NULL;
  grid[level].nTs=-1;
  --level;
  setfc(deffc);
}

void errorcheck()
{
  // query opengl for errors: inform & exit if found
  GLenum error=glGetError();
  if (error!=GL_NO_ERROR)
  {
    printf("%s\n",gluErrorString(error));
    exit(1);
  }
}

void extend_vertex(struct T *Tp)
{
  // position new vertex at correct radius from origin
  int i,j;
  double d,e;
  for (i=0;i<3;i++)
  {
    d=distance(0,0,0,Tp->v[i][0],Tp->v[i][1],Tp->v[i][2]);
    if (d!=radius)
    {
      e=radius/d;
      for (j=0;j<3;j++)
        Tp->v[i][j]*=e;
    }
  }
}

void extend()
{
  // extend new vertices to correct radius
  int i;
  for (i=0;i<grid[level].nTs;i++)
    extend_vertex(&grid[level].Tp[i]);
  set_ns_and_cs();
  animates=0;
}

void icosahedron()
{
  // construct the initial icosahedron
  double m[3][3];
  p=(1+sqrt(5))/2; // special coordinate for icosahedron of side 2
  // the 12 vertices of the icosahedron:
  vertex[0][0]=-1;  vertex[0][1]=+p;  vertex[0][2]=+0;
  vertex[1][0]=-p;  vertex[1][1]=+0;  vertex[1][2]=+1;
  vertex[2][0]=+0;  vertex[2][1]=-1;  vertex[2][2]=+p;
  vertex[3][0]=+p;  vertex[3][1]=+0;  vertex[3][2]=+1;
  vertex[4][0]=+1;  vertex[4][1]=+p;  vertex[4][2]=+0;
  vertex[5][0]=+0;  vertex[5][1]=+1;  vertex[5][2]=+p;
  vertex[6][0]=-p;  vertex[6][1]=+0;  vertex[6][2]=-1;
  vertex[7][0]=-1;  vertex[7][1]=-p;  vertex[7][2]=+0;
  vertex[8][0]=+1;  vertex[8][1]=-p;  vertex[8][2]=+0;
  vertex[9][0]=+p;  vertex[9][1]=+0;  vertex[9][2]=-1;
  vertex[10][0]=+0; vertex[10][1]=+1; vertex[10][2]=-p;
  vertex[11][0]=+0; vertex[11][1]=-1; vertex[11][2]=-p;
  // the 20 faces of the icosahedron:
  int face[20][3]=
  {
    {0,1,5},{1,2,5},{2,3,5},{3,4,5},{4,0,5},
    {0,1,6},{1,2,7},{2,3,8},{3,4,9},{4,0,10},
    {7,6,1},{8,7,2},{9,8,3},{10,9,4},{6,10,0},
    {6,7,11},{7,8,11},{8,9,11},{9,10,11},{10,6,11}
  };
  int i,j,k;
  struct T *Tp=(struct T *)malloc(20*sizeof(struct T));
  if (!Tp) die("Cannot malloc space for triangles.");
  // create face triangles and calculate normals
  for (i=0;i<20;i++)
  {
    for (j=0;j<3;j++)
    {
      for (k=0;k<3;k++)
        Tp[i].v[j][k]=vertex[face[i][j]][k];
    }
    midpoints(&Tp[i],m);
    normal(&Tp[i],m);
  }
  radius=distance(0,0,0,Tp[0].v[0][0],Tp[0].v[0][1],Tp[0].v[0][2]);
  grid[level].nTs=20;
  grid[level].Tp=Tp;
}

void idle()
{
  // do this when no user activity is being handled
  double thistime;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  thistime=tv.tv_sec+(tv.tv_usec/1000000.0);

  if (thistime-lasttime>.03)
  {
    // update if enough time has passed
    if (!animatep)
    {
      facecolor[0]+=facecolor[0]<deffc[0]?0.005:-0.005;
      facecolor[1]+=facecolor[1]<deffc[1]?0.005:-0.005;
      facecolor[2]+=facecolor[2]<deffc[2]?0.005:-0.005;
      if (earthalpha<defearthalpha) earthalpha+=0.005;
    }
    rotate_la(1);
    if (play)
    {
      // rotate if autoplay is enabled...
      rotate_th(.5);
      rotate_ph(.5);
    }
    glutPostRedisplay();
    lasttime=thistime;       // record time of last update
    if (animatep) animate(); // update animation settings
  }
}

void init()
{
  // set things up
  int i;
  glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH|GLUT_DOUBLE);
  glutInitWindowSize(600,600);
  glutCreateWindow("Icosahedral Tiling");
  glClearColor(0,0,0,1);
  setfc(deffc);
  // register callback functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(key);
  glutSpecialFunc(special);
  glutIdleFunc(idle);
  loadtextures();
  for (i=0;i<levels;i++)
  {
    grid[i].Tp=NULL;
    grid[i].nTs=-1;
  }
  earthalpha=defearthalpha;
  icosahedron();
}

void key(unsigned char ch,int x,int y)
{
  // handle "normal" keypresses
  switch(ch)
  {
    case '+': if (dim>=radius+.1) { dim-=.1; --fov; } break;
    case '-': dim+=.1; ++fov; break;
    case '<': if (level>0) downgrid(); break;
    case '>': if ((!animatep)&&(level<levels)) upgrid(); return;
    case 'a': axesp=1-axesp; break;
    case 'c': centroidsp=1-centroidsp; break;
    case 'e': edgesp=1-edgesp; break;
    case 'f': fixedp=1-fixedp; break;
    case 'g': play=1-play; break;
    case 'm': if (!animatep) animatem=1-animatem; break;
    case 'n': normalsp=1-normalsp; break;
    case 'r': if (!animatep) refinem=1-refinem; break;
    case 's': spherep=1-spherep; break;
    case 't': ++texturen; texturen%=EARTHS+1; break;
    case '0': la=0; ph=0; th=0; break;
    case 27:  exit(0); break;
    // unadvertised control:
    case 'p': projmode=1-projmode; break;
  }
  project();
}

void loadtextures()
{
  // based on CSCI 5229 loadtexbmp.c
  char filename[11];
  FILE *f;
  unsigned int dx,dy,size,k;
  unsigned short magic,nbp,bpp;
  unsigned char *image,temp;
  int i;
  for (i=0;i<EARTHS;i++)
  {
    sprintf(filename,"earth%d.bmp",i);
    f=fopen(filename,"r");
    if (!f) die("Cannot open texture file.");
    if (fread(&magic,2,1,f)!=1) die("Cannot read magic from texture file.");
    if (magic!=0x4D42) die("Texture file not BMP or endian problem.");
    if (fseek(f,16,SEEK_CUR)||fread(&dx ,4,1,f)!=1||fread(&dy,4,1,f)!=1||
        fread(&nbp,2,1,f)!=1||fread(&bpp,2,1,f)!=1||fread(&k,4,1,f)!=1)
      die("Cannot read header from texture file.");
    if (k!=0) die("Cannot use compressed bmp.");
    size=3*dx*dy;
    image=(unsigned char *)malloc(size);
    if (!image) die("Cannot malloc space for texture image.");
    if (fseek(f,20,SEEK_CUR)||fread(image,size,1,f)!=1)
      die("Cannot read image.");
    fclose(f);
    for (k=0;k<size;k+=3)
    {
      temp=image[k];
      image[k]=image[k+2];
      image[k+2]=temp;
    }
    glGenTextures(1,&textures[i]);
    glBindTexture(GL_TEXTURE_2D,textures[i]);
    glTexImage2D(GL_TEXTURE_2D,0,3,dx,dy,0,GL_RGB,GL_UNSIGNED_BYTE,image);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    free(image);
    errorcheck();
  }
}

int main(int argc,char **argv)
{
  glutInit(&argc,argv);
  init();
  glutMainLoop();
  return(0);
}

void midpoints(struct T *Tp,double m[3][3])
{
  // find triangle side midpoints
  int j,k;
  for (j=0;j<3;j++)
    for (k=0;k<3;k++)
      m[j][k]=(Tp->v[j][k]+Tp->v[(j+1)%3][k])/2;
}

void normal(struct T *Tp,double m[3][3])
{
  // find the unit normal vector for a triangle
  int i;
  double cn[3],doc,don,l;
  // find triangle's unit normal vector
  double a1=Tp->v[0][0]-Tp->v[2][0];
  double a2=Tp->v[0][1]-Tp->v[2][1];
  double a3=Tp->v[0][2]-Tp->v[2][2];
  double b1=Tp->v[1][0]-Tp->v[2][0];
  double b2=Tp->v[1][1]-Tp->v[2][1];
  double b3=Tp->v[1][2]-Tp->v[2][2];
  Tp->n[0]=+(b2*a3-b3*a2);
  Tp->n[1]=-(b1*a3-b3*a1);
  Tp->n[2]=+(b1*a2-b2*a1);
  // check & possibly correct normal's direction
  centroid(m,Tp);
  l=distance(0,0,0,Tp->n[0],Tp->n[1],Tp->n[2]);
  for (i=0;i<3;i++)
  {
    Tp->n[i]/=l;
    cn[i]=Tp->c[i]+Tp->n[i];
  }
  doc=distance(0,0,0,Tp->c[0],Tp->c[1],Tp->c[2]);
  don=distance(0,0,0,cn[0],cn[1],cn[2]);
  if (don<doc)
    for (i=0;i<3;i++) Tp->n[i]*=-1;
}

void project()
{
  // set up the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (projmode) // perspective
    gluPerspective(fov,ar,dim/4,4*dim);
  else          // orthogonal
    glOrtho(-ar*dim,ar*dim,-dim,dim,-dim,dim);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glutPostRedisplay();
}

void refine()
{
  // create next grid level by bisecting and extending this grid's triangles
  if (refinem)
  {
    bisect();
    extend();
  }
  else
    animates==1?extend():bisect();
  if (animatem) animatep=1;
}

void reshape(int w,int h)
{
  // handle window resizing
  ar=(h>0)?(double)w/h:1;
  glViewport(0,0,w,h);
  project();
  errorcheck();
}

void rotate_la(double inc)
{
  // increment/decrement/modulate la
  la+=inc;
  if (la>360) la-=360;
}

void rotate_ph(double inc)
{
  // increment/decrement/modulate ph
  ph+=inc;
  if (ph>360) ph-=360;
}

void rotate_th(double inc)
{
  // increment/decrement/modulate th
  th+=inc;
  if (th>360) th-=360;
}

void set_ns_and_cs()
{
  // set normals & centroids
  double m[3][3];
  int i;
  for (i=0;i<grid[level].nTs;i++)
  {
    midpoints(&grid[level].Tp[i],m);
    normal(&grid[level].Tp[i],m);
  }
}

void setfc(double *c)
{
  // reset face color
  facecolor[0]=c[0];
  facecolor[1]=c[1];
  facecolor[2]=c[2];
  facecolor[3]=c[3];
}

void special(int key,int x,int y)
{
  // handle "special" keypresses
  switch(key)
  {
    case GLUT_KEY_RIGHT: rotate_th(+2); break;
    case GLUT_KEY_LEFT:  rotate_th(-2); break;
    case GLUT_KEY_UP:    rotate_ph(+2); break;
    case GLUT_KEY_DOWN:  rotate_ph(-2); break;
  }
  project();
}

void shellsphere()
{
  // draw a translucent shell around the geodesic
  int a,b;
  if (texturen!=0)
  {
    // set the texture on the sphere
    glBindTexture(GL_TEXTURE_2D,textures[texturen-1]);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glEnable(GL_TEXTURE_2D);
  }
  glColor4f(.5,.5,.5,earthalpha);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  glEnable(GL_BLEND);
  glPushMatrix();
  glRotated(-92,0,1,0);
  glRotated(-92,1,0,0);
  // this sphere-drawing routine & spherev() from CSCI 5229 ex17.c
  for (b=-90;b<90;b+=5)
  {
    glBegin(GL_QUAD_STRIP);
    for (a=0;a<=360;a+=5)
    {
      spherev(a,b);
      spherev(a,b+5);
    }
    glEnd();
  }
  glPopMatrix();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}

void spherev(double a,double b)
{
  // set a vertex for a sphere
  double scaling=1.01; // helps avoid z-fighting splotches @ g3+
  double m=PI/180;
  double x=radius*sin(a*m)*cos(b*m)*scaling;
  double y=radius*cos(a*m)*cos(b*m)*scaling;;
  double z=radius*sin(b*m)*scaling;
  glNormal3d(x,y,z);
  glTexCoord2d(a/360,b/180+.5);
  glVertex3d(x,y,z);
}

void upgrid()
{
  // increase the grid level
  //
  // if we're in the middle of a 2-step refinement and the user has switched to
  // 1-step mode, complete the refinement by extending the vertices before
  // proceeding
  if (refinem&&animates) refine();
  // not in the middle of a 2-part refinement? increase the grid level
  if (!animates) ++level;
  refine();
}
