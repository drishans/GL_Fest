/*
 * Drishan Sarkar: CSCI 4229 Final Project 
 * Draws a scene with some textured houses and a rotating light.
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  x/X        Decrease/increase ambient light
 *  c/C        Decrease/increase diffuse light
 *  v/V        Decrease/increase specular light
 *  b/B        Decrease/increase emitted light
 *  n/N        Decrease/increase shininess
 *  F1         Toggle smooth/flat shading
 *  F2         Toggle local viewer mode
 *  F3         Toggle light distance (1/5)
 *  F5         Day/Night
 *  F8         Change ball increment
 *  F9         Invert bottom normal
 *  m          Toggles light movement
 *  []         Lower/rise light
 *  f          Toggle first person
 *  <>         Move the light
 *  -/=        Change field of view of perspective
 *  z          Toggle axes
 *  g          Toggle ground
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  r          Reset view angle
 *  ESC        Exit
 *  p          Dev mode
 */
#include "CSCIx229.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

int th=35;         //  Azimuth of view angle
int ph=15;         //  Elevation of view angle
int axes=1;        //  Display axes
int ground=1;      //  Display ground
int mode=0;        //  Dev mode
int day=0;         //  Day / Night
int move=1;        //  Move light
int fov=55;        //  Field of view (for perspective)
double dt=0.05;    //  Fraction for updating eye position
double asp=1;      //  Aspect ratio
double dim=10.0;   //  Size of world

//  First person Point-Of-View 
int fppov=0;
int rot=0;
//  Eye position
double Ex=0;
double Ey=0;
double Ez=2;
//  Look position
double Cx=0;
double Cy=0;
double Cz=0;

// Light values
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =  15;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  70;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =  50;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   7;  // Elevation of light

// Textures
unsigned int texture[16];
unsigned int sky[4];
// Shaders
int shader[] = {0,0,0};

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glColor3f(Cos(th)*Cos(th), Sin(ph)*Sin(ph), Sin(th)*Sin(th));
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

/* 
 *  Draw sky box
 */
static void Sky(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,day?sky[0]:sky[2]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.00,0); glVertex3f(-D,-D*.25,-D);
   glTexCoord2f(0.25,0); glVertex3f(+D,-D*.25,-D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(0.25,0); glVertex3f(+D,-D*.25,-D);
   glTexCoord2f(0.50,0); glVertex3f(+D,-D*.25,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

   glTexCoord2f(0.50,0); glVertex3f(+D,-D*.25,+D);
   glTexCoord2f(0.75,0); glVertex3f(-D,-D*.25,+D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

   glTexCoord2f(0.75,0); glVertex3f(-D,-D*.25,+D);
   glTexCoord2f(1.00,0); glVertex3f(-D,-D*.25,-D);
   glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glEnd();

   //  Top and bottom
   glBindTexture(GL_TEXTURE_2D,day?sky[1]:sky[3]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(1.0,1); glVertex3f(-D,-D*.25,+D);
   glTexCoord2f(0.5,1); glVertex3f(+D,-D*.25,+D);
   glTexCoord2f(0.5,0); glVertex3f(+D,-D*.25,-D);
   glTexCoord2f(1.0,0); glVertex3f(-D,-D*.25,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

/*
 * Draw the ground
 */
static void Ground()
{
   // Set material
   float green[] = {0,1,0,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,green);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   // Save transformation
   glPushMatrix();
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[0]);
   // Draw
   glColor3ub(0,69,0);
   glBegin(GL_QUADS);
   glNormal3f(0,+1,0);
   glTexCoord2f(0.0, 0.0); glVertex3f(-100,-1,-100);
   glTexCoord2f(0.0, 100); glVertex3f(-100,-1,100);
   glTexCoord2f(100, 100); glVertex3f(100,-1,100);
   glTexCoord2f(100, 0.0); glVertex3f(100,-1,-100);
   glEnd();
   // Swith off textures
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/*
 *  Draw a cylinder
 */
static void Cylinder(float x,float y,float z , float th,float ph , float R,float H)
{
   int i,j;   // Counters
   int N=32; // Number of slices

   //  Transform
   glPushMatrix();
   glTranslated(x,y,z);
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);
   glScaled(R,R,H);

   //  Two end caps (fan of triangles)
   for (j=-1;j<=1;j+=2)
   {
      glNormal3d(0,0,j); 
      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2d(0,0); glVertex3d(0,0,j);
      for (i=0;i<=N;i++)
      {
         float th = j*i*360.0/N;
         glTexCoord2d(Cos(th),Sin(th)); glVertex3d(Cos(th),Sin(th),j);
      }
      glEnd();
   }

   //  Cylinder Body (strip of quads)
   glBegin(GL_QUADS);
   for (i=0;i<N;i++)
   {
      float th0 =  i   *360.0/N;
      float th1 = (i+1)*360.0/N;
      glNormal3d(Cos(th0),Sin(th0),0); glTexCoord2d(0,th0/90.0); glVertex3d(Cos(th0),Sin(th0),+1);
      glNormal3d(Cos(th0),Sin(th0),0); glTexCoord2d(2,th0/90.0); glVertex3d(Cos(th0),Sin(th0),-1);
      glNormal3d(Cos(th1),Sin(th1),0); glTexCoord2d(2,th1/90.0); glVertex3d(Cos(th1),Sin(th1),-1);
      glNormal3d(Cos(th1),Sin(th1),0); glTexCoord2d(0,th1/90.0); glVertex3d(Cos(th1),Sin(th1),+1);
   }
   glEnd();

   //  Restore
   glPopMatrix();
}

/*
 *  Draw the stage and screen
 */
static void Screen()
{
   glUseProgram(shader[1]);

   glPushMatrix();
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[6]);
   //  Stand
   Cylinder( 7, 1 , -16, 0, 90, 0.0686, 3);
   Cylinder(-3, 1 , -16, 0, 90, 0.0686, 3);
   Cylinder( 6, -0.25 , -12, 0, 90, 0.0686, 3);
   Cylinder(-2, -0.25 , -12, 0, 90, 0.0686, 3);
   glBegin(GL_QUADS);
   //  Screen 1
   glNormal3f( 0, 0, +1);
   glTexCoord2f(0.0, 0.0);   glVertex3f(-3,  1, -16);
   glTexCoord2f(1.0, 0.0);   glVertex3f(+7,  1, -16);
   glTexCoord2f(1.0, 1.0);   glVertex3f(+7, +5, -16);
   glTexCoord2f(0.0, 1.0);   glVertex3f(-3, +5, -16);
   //  Screen 2
   glNormal3f( 0, 0, +1);
   glTexCoord2f(0.0, 0.0);   glVertex3f(-2, -1, -12);
   glTexCoord2f(1.0, 0.0);   glVertex3f(+6, -1, -12);
   glTexCoord2f(1.0, 1.0);   glVertex3f(+6, +2.1, -12);
   glTexCoord2f(0.0, 1.0);   glVertex3f(-2, +2.1, -12);
   glEnd();
   glDisable(GL_TEXTURE_2D);  
   glPopMatrix();
   
   glUseProgram(0);
}
/*
 *  Draw the lake
 */
static void Lake()
{
   // Set material
   float blue[] = {0,0,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,blue);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[7]);
   // Draw a lake
   Cylinder(17, -1 , 17, 0, 90, 13, 0.05);
   Cylinder(19, -1 , 22, 0, 90, 13, 0.05);
   Cylinder(11, -1 , 22, 0, 90, 14, 0.05);
   Cylinder(9, -1 , 22, 0, 90, 13, 0.05);
   Cylinder(3, -1 , 25, 0, 90, 13, 0.05);
   Cylinder(0, -1 , 24, 0, 90, 13, 0.05);
   Cylinder(-3, -1 , 24, 0, 90, 13, 0.05);
   Cylinder(-7, -1 , 24, 0, 90, 13, 0.05);
   Cylinder(-17, -1 , 17, 0, 90, 13, 0.05);
   Cylinder(-17, -1 , 0, 0, 90, 6, 0.05);
   Cylinder(-17, -1 , 0, 0, 90, 6, 0.05);
   Cylinder(-22, -1 , 4, 0, 90, 6, 0.05);
   Cylinder(-17, -1 , 4, 0, 90, 6, 0.05);
   Cylinder(-22, -1 , -4, 0, 90, 6, 0.05);
   Cylinder(-15, -1 , -4, 0, 90, 6, 0.05);
   Cylinder(-17, -1 , -17, 0, 90, 13, 0.05);
   glDisable(GL_TEXTURE_2D);  
}

/*
 *  Draw a house
 */
static void House(double x, double y, double z,
		      double dx, double dy, double dz,
		      double th)
{
   const double rTop = 2;
   const double rOffset = 1.5;
   const double rWid = 1.25;
   const double rBot = .75;

   // Set material
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   //  Save transformation
   glPushMatrix();
   //  Translations
   glTranslated(x, y, z);
   glRotated(th, 0, 1, 0);
   glScaled(dx, dy, dz);
   
   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

   //  Body
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   //  Front
   glColor3ub(123,10,10);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0.0, 0.0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1.0, 0.0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1.0, 1.0); glVertex3f(+1,+1, 1);
   glTexCoord2f(0.0, 1.0); glVertex3f(-1,+1, 1);
   //  Back
   glColor3ub(123,10,10);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1.0, 0.0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1.0, 1.0); glVertex3f(-1,+1,-1);
   glTexCoord2f(0.0, 1.0); glVertex3f(+1,+1,-1);
   //  Right
   glColor3ub(123,10,10);
   glNormal3f(+1, 0, 0); 
   glTexCoord2f(0.0, 0.0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1.0, 0.0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1.0, 1.0); glVertex3f(+1,+1,-1);
   glTexCoord2f(0.0, 1.0); glVertex3f(+1,+1,+1);
   //  Left
   glColor3ub(123,10,10);
   glNormal3f(-1, 0, 0); 
   glTexCoord2f(0.0, 0.0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1.0, 0.0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1.0, 1.0); glVertex3f(-1,+1,+1);
   glTexCoord2f(0.0, 1.0); glVertex3f(-1,+1,-1);
   
   //  Top
   glColor3ub(1,1,1);
   glNormal3f( 0,+1, 0); 
   glVertex3f(-1,+1,+1);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   glVertex3f(-1,+1,-1);
   //  Bottom
   glColor3ub(77,66,0);
   glNormal3f( 0,-1, 0); 
   glVertex3f(-1,-1,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,-1,+1);
   glVertex3f(-1,-1,+1);
   glEnd();
   
   // Chimney
   glBegin(GL_QUADS);
   //  Left
   glColor3ub(100,0,0);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.5,+1,+.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+.5,+1,-.25);
   glTexCoord2f(1.0, 1.0); glVertex3f(+.5,+2,-.25);
   glTexCoord2f(0.0, 1.0); glVertex3f(+.5,+2,+.25);
   //  Right
   glColor3ub(100,0,0);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(+1, +1,+.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+1, +1,-.25);
   glTexCoord2f(1.0, 1.0); glVertex3f(+1, +2,-.25);
   glTexCoord2f(0.0, 1.0); glVertex3f(+1, +2,+.25);
   //  Back
   glColor3ub(100,0,0);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.5,+1,-.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+.5,+2,-.25);
   glTexCoord2f(1.0, 1.0); glVertex3f(+1, +2,-.25);
   glTexCoord2f(0.0, 1.0); glVertex3f(+1, +1,-.25);
   //  Front
   glColor3ub(100,0,0);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.5,+1,+.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+.5,+2,+.25);
   glTexCoord2f(1.0, 1.0); glVertex3f(+1, +2,+.25);
   glTexCoord2f(0.0, 1.0); glVertex3f(+1, +1,+.25);
   //  Top
   glColor3ub(101,101,101);
   glNormal3f( 0,+1, 0);
   glVertex3f(+.5,+2,+.25);
   glVertex3f(+.5,+2,-.25);
   glVertex3f(+1 ,+2,-.25);
   glVertex3f(+1 ,+2,+.25);
   glEnd();
    
   // Roof
   glBindTexture(GL_TEXTURE_2D,texture[3]);
   glBegin(GL_TRIANGLES);
   //  Front
   glColor3ub(35,35,35);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+rOffset,+rBot,+rWid);
   glTexCoord2f(1.0, 0.0); glVertex3f(-rOffset,+rBot,+rWid);
   glTexCoord2f(0.5, 1.0); glVertex3f(   0    ,+rTop,+rWid);
   //  Back
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+rOffset,+rBot,-rWid);
   glTexCoord2f(1.0, 0.0); glVertex3f(-rOffset,+rBot,-rWid);
   glTexCoord2f(0.5, 1.0); glVertex3f(   0    ,+rTop,-rWid);
   glEnd();
   glBegin(GL_QUADS);
   // Right side
   glColor3ub(35,35,35);
   glNormal3f(+1,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(+rOffset,+rBot,-rWid);
   glTexCoord2f(1.0, 0.0); glVertex3f(+rOffset,+rBot,+rWid);
   glTexCoord2f(1.0, 1.0); glVertex3f(   0    ,+rTop,+rWid);
   glTexCoord2f(0.0, 1.0); glVertex3f(   0    ,+rTop,-rWid);
   // Left Side
   glColor3ub(35,35,35);
   glNormal3f(-1,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(-rOffset,+rBot,-rWid);
   glTexCoord2f(1.0, 0.0); glVertex3f(-rOffset,+rBot,+rWid);
   glTexCoord2f(1.0, 1.0); glVertex3f(   0    ,+rTop,+rWid);
   glTexCoord2f(0.0, 1.0); glVertex3f(   0    ,+rTop,-rWid);
   glEnd();

   // Door
   glBindTexture(GL_TEXTURE_2D,texture[4]);
   glBegin(GL_QUADS);
   glColor3ub(120, 125, 70);
   //  Front
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.2,-1,-1.1);
   glTexCoord2f(1.0, 0.0); glVertex3f(-.2,-1,-1.1);
   glTexCoord2f(1.0, 1.0); glVertex3f(-.2, 0,-1.1);
   glTexCoord2f(0.0, 1.0); glVertex3f(+.2, 0,-1.1);
   //  Top
   glVertex3f(+.2, 0,-1.1);
   glVertex3f(+.2, 0,-1.0);
   glVertex3f(-.2, 0,-1.0);
   glVertex3f(-.2, 0,-1.1);
   //  Right
   glVertex3f(+.2, 0,-1.1);
   glVertex3f(+.2,-1,-1.1);
   glVertex3f(+.2,-1,-1.0);
   glVertex3f(+.2, 0,-1.0);
   //  Left
   glVertex3f(-.2, 0,-1.1);
   glVertex3f(-.2,-1,-1.1);
   glVertex3f(-.2,-1,-1.0);
   glVertex3f(-.2, 0,-1.0);
   glEnd();
   
   // Swith off textures
   glDisable(GL_TEXTURE_2D);

   glPopMatrix();
}

/*
 *  Draw a tree
 */
static void Tree(double x, double y, double z,
                  double dx, double dy, double dz,
                  double th)
{  
   // Set material
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

   // Save transformation
   glPushMatrix();
   
   // Offset and scale
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);


   // Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   // Body of tree
   glBindTexture(GL_TEXTURE_2D,texture[1]);
   glBegin(GL_QUADS);
   //Front
   glColor3ub(102,51,0);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0.0, 0.0); glVertex3f(-.25,-.25, .25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+.25,-.25, .25);
   glTexCoord2f(1.0, 2.0); glVertex3f(+.25,+5  , .25);
   glTexCoord2f(0.0, 2.0); glVertex3f(-.25,+5  , .25);
   //  Back
   glColor3ub(102,51,0);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.25,-.25,-.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(-.25,-.25,-.25);
   glTexCoord2f(1.0, 2.0); glVertex3f(-.25,+5  ,-.25);
   glTexCoord2f(0.0, 2.0); glVertex3f(+.25,+5  ,-.25);
   //  Right
   glColor3ub(102,51,0);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(+.25,-.25,+.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(+.25,-.25,-.25);
   glTexCoord2f(1.0, 2.0); glVertex3f(+.25,+5  ,-.25);
   glTexCoord2f(0.0, 2.0); glVertex3f(+.25,+5  ,+.25);
   //  Left
   glColor3ub(102,51,0);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f(-.25,-.25,-.25);
   glTexCoord2f(1.0, 0.0); glVertex3f(-.25,-.25,+.25);
   glTexCoord2f(1.0, 2.0); glVertex3f(-.25,+5  ,+.25);
   glTexCoord2f(0.0, 2.0); glVertex3f(-.25,+5  ,-.25);
   /*  Top */
   glColor3ub(51,0,0);
   glNormal3f( 0,+1, 0);
   glVertex3f(-.25,+5  ,+.25);
   glVertex3f(+.25,+5  ,+.25);
   glVertex3f(+.25,+5  ,-.25);
   glVertex3f(-.25,+5  ,-.25);
   //  Bottom
   glColor3ub(51,0,0);
   glNormal3f( 0,-1, 0);
   glVertex3f(-.25,-.25,-.25);
   glVertex3f(+.25,-.25,-.25);
   glVertex3f(+.25,-.25,+.25);
   glVertex3f(-.25,-.25,+.25);
   
   /* Top of Tree ** ERROR: invalid operation [display]**
   glBindTexture(GL_TEXTURE_2D,texture[5]);
   glBegin(GL_TRIANGLES);
   glColor3ub(0,20,0);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f( 0.0f, 7.f, 0.0f );
   glTexCoord2f(1.0, 0.0); glVertex3f( -1.0f, 4.0f, 1.0f );
   glTexCoord2f(1.0, 1.0); glVertex3f( 1.0f, 4.0f, 1.0f);

   glColor3ub(0,20,0);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f( 0.0f, 7.0f, 0.0f);
   glTexCoord2f(1.0, 0.0); glVertex3f( -1.0f, 4.0f, 1.0f);
   glTexCoord2f(1.0, 1.0); glVertex3f( 0.0f, 4.0f, -1.0f);

   glColor3ub(0,20,0);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f( 0.0f, 7.0f, 0.0f);
   glTexCoord2f(1.0, 0.0); glVertex3f( 0.0f, 4.0f, -1.0f);
   glTexCoord2f(1.0, 1.0); glVertex3f( 1.0f, 4.0f, 1.0f);

   glColor3ub(0,20,0);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0.0, 0.0); glVertex3f( 0.0f, 7.0f, 0.0f);
   glTexCoord2f(1.0, 0.0); glVertex3f( 0.0f, 4.0f, -1.0f);
   glTexCoord2f(1.0, 1.0); glVertex3f( 1.0f, 4.0f, 1.0f);*/
   
   glEnd();
   // Swith off textures
   glDisable(GL_TEXTURE_2D);   
   
   glPopMatrix();
}

/*
 *  Draw the scene
 */
void Scene()
{
   //  Draw Sky
   Sky(3.5*dim);
   //  Draw the ground
   if (ground) {
      Ground();
   }
   //  Draw houses
   House( 0  , 0  , 2  , 1  , 1  , 1  , 0  );
   House( 6.5, 0  , 1  , 1.5, 1  , 2  , 35 );
   House(-6.5, 0  , 1  , 2.5, 1  , 1  ,-35 );
   //  Draw trees
   Tree( 0  , -1 , 6  , 1  , .9 , 1.3, 0  );
   Tree( 3  , -1 , 5  , 1  , 1  , 0.8, .1 );
   Tree(-2.2, -1 , 5  , 1  , 1  , 1.1, .2 );
   Tree( 2.8, -1 , 8  , 1  , .7 , 1.2, .3 );
   Tree(-2.8, -1 , 8.6, 1  , 1  , 0.9, .4 );
   Tree( 1.7, -1 , 9  , 1  , .9 , 1.3, 0  );
   Tree( 9  , -1 , 5  , 1  , .9 , 1.3, 0  );
   Tree(-10 , -1 , 4  , 1  , .9 , 1.3, 0  );
   Tree(-7  , -1 , 5.5, 1  , .9 , 1.3, 0  );
   
   //  Draw the lake
   Lake();
   
   //  Draw the screen
   Screen();

}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=3.5;  //  Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();

   //  Perspective - set look position
   if (fppov)
   {
      Cx = +2*dim*Sin(rot);
      Cz = -2*dim*Cos(rot);
      gluLookAt(Ex,Ey,Ez,Cx+Ex,/*Cy+*/Ey,Cz-Ez,0,1,0);
   }
   //  Perspective - set eye position
   else
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt( Ex , Ey , Ez , 0 , 0 , 0 , 0,Cos(ph),0);
   }
   
   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);
   
   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.2);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else {
      glDisable(GL_LIGHTING);
   }
   
   Scene(1); 

   //  Draw axes
   glColor3f(1,1,1);
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,-len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,-len);
      Print("Z");
   }
   
   if (mode) {
      //  Five pixels from the lower left corner of the window
      glWindowPos2i(5,5);
      //  Print the text string
      Print("Angle=%d,%d  Dim=%.1f  FOV=%d  Projection=%s  POV=%d,%d \n",
            th,ph,dim,fov,mode?"Perpective":"Orthogonal",fppov,rot);
      if (light)
      {
         glWindowPos2i(5,45);
         Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
         glWindowPos2i(5,25);
         Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
      } 
   }
   //  Render the scene
   ErrCheck("display");
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_UP)
      dim -= 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  Smooth color model
   else if (key == GLUT_KEY_F1)
      smooth = 1-smooth;
   //  Local Viewer
   else if (key == GLUT_KEY_F2)
      local = 1-local;
   else if (key == GLUT_KEY_F3)
      distance = (distance==1) ? 5 : 1;
   //  Night mode
   else if (key == GLUT_KEY_F5)
      day = 1-day;   
   //  Toggle ball increment
   else if (key == GLUT_KEY_F8)
      inc = (inc==10)?3:10;
   //  Flip sign
   else if (key == GLUT_KEY_F9)
      one = -one;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(fov,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27) {
      exit(0);
   }
   //  Reset View Angle
   else if (ch == 'r') {
      th = 180;
      ph = 7;
   }
   //  Toggle Axes
   else if (ch == 'z' || ch == 'Z') {
      axes = 1-axes;
   }
   //  Toggle Ground
   else if (ch == 'g' || ch == 'G') {
      ground = 1-ground;
   }
   //  Light elevation
   else if (ch=='[') {
      ylight -= 0.1;
   }
   else if (ch==']') {
      ylight += 0.1;
   }
   //  Ambient level
   else if (ch=='x' && ambient>0) {
      ambient -= 5;
   }
   else if (ch=='X' && ambient<100) {
      ambient += 5;
   }
   //  Diffuse level
   else if (ch=='c' && diffuse>0) {
      diffuse -= 5;
   }
   else if (ch=='C' && diffuse<100) {
      diffuse += 5;
   }
   //  Specular level
   else if (ch=='v' && specular>0) {
      specular -= 5;
   }
   else if (ch=='V' && specular<100) {
      specular += 5;
   }
   //  Emission level
   else if (ch=='b' && emission>0) {
      emission -= 5;
   }
   else if (ch=='B' && emission<100) {
      emission += 5;
   }
   //  Shininess level
   else if (ch=='n' && shininess>-1) {
      shininess -= 1;
   }
   else if (ch=='N' && shininess<7) {
      shininess += 1;
   }
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L') {
      light = 1-light;
   }
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P') {
      mode = 1-mode;
      //fppov = 0;
   }
   //  Toggle light movement
   else if (ch == 'm' || ch == 'M') {
      move = 1-move;
   }
   //  Move light
   else if (ch == '<') {
      zh += 1;
   }
   else if (ch == '>') {
      zh -= 1;
   }
   //  Switch to First Person POV
   else if (mode && (ch == 'f' || ch == 'F')) {
      fppov = 1-fppov;
   }
   //  Change Field of View Angle
   else if (ch == '-' && ch>1) {
      fov--;
   }
   else if (ch == '=' && ch<179) {
      fov++;
   }
   //  Movement and View
   else if (fppov && (ch == 'q' || ch == 'Q')) {
      if (Ey >= 0) Ey -= dt;
   }
   else if (fppov && (ch == 'e' || ch == 'E')) {
      Ey += dt;
   }
   else if (fppov && (ch == 'w' || ch == 'W')) {
      Ex += Cx*dt;
      Ez += Cz*dt;
   }
   else if (fppov && (ch == 's' || ch == 'S')) {
      Ex -= Cx*dt;
      Ez -= Cz*dt;
   }
   else if (fppov && (ch == 'a' || ch == 'A')) {
      rot -= 1;
   }
   else if (fppov && (ch == 'd' || ch == 'D')) {
      rot += 1;
   }
   rot %= 360;
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(fov,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp,dim);
}

/*
 *  Read text file
 */
char* ReadText(char *file)
{
   int   n;
   char* buffer;
   //  Open file
   FILE* f = fopen(file,"rt");
   if (!f) Fatal("Cannot open text file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char*)malloc(n+1);
   if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n",n+1,file);
   //  Snarf the file
   if (fread(buffer,n,1,f)!=1) Fatal("Cannot read %d bytes for text file %s\n",n,file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

/*
 *  Print Shader Log
 */
void PrintShaderLog(int obj,char* file)
{
   int len=0;
   glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n",len);
      glGetShaderInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s:\n%s\n",file,buffer);
      free(buffer);
   }
   glGetShaderiv(obj,GL_COMPILE_STATUS,&len);
   if (!len) Fatal("Error compiling %s\n",file);
}

/*
 *  Print Program Log
 */
void PrintProgramLog(int obj)
{
   int len=0;
   glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n",len);
      glGetProgramInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s\n",buffer);
   }
   glGetProgramiv(obj,GL_LINK_STATUS,&len);
   if (!len) Fatal("Error linking program\n");
}

/*
 *  Create Shader
 */
int CreateShader(GLenum type,char* file)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char* source = ReadText(file);
   glShaderSource(shader,1,(const char**)&source,NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr,"Compile %s\n",file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader,file);
   //  Return name
   return shader;
}

/*
 *  Create Shader Program
 */
int CreateShaderProg(char* VertFile,char* FragFile)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER  ,VertFile);
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER,FragFile);
   //  Attach vertex shader
   glAttachShader(prog,vert);
   //  Attach fragment shader
   glAttachShader(prog,frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(900,600);
   //  Create the window
   glutCreateWindow("Final: Drishan Sarkar");
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Load the textures
   texture[0] = LoadTexBMP("textures/TallGreenGrass.bmp");
   texture[1] = LoadTexBMP("textures/bark.bmp");
   texture[2] = LoadTexBMP("textures/Carbon_02.bmp");
   texture[3] = LoadTexBMP("textures/Carbon_03.bmp");
   texture[4] = LoadTexBMP("textures/crate.bmp");
   texture[5] = LoadTexBMP("textures/leaf.bmp");
   texture[6] = LoadTexBMP("textures/smoke.bmp");
   texture[7] = LoadTexBMP("textures/water.bmp");
   sky[0] = LoadTexBMP("textures/sky0.bmp");
   sky[1] = LoadTexBMP("textures/sky1.bmp");
   sky[2] = LoadTexBMP("textures/sky0n.bmp");
   sky[3] = LoadTexBMP("textures/sky1n.bmp");
 
   //  Load shader programs
   shader[1] = CreateShaderProg("shader/toon.vert","shader/toon.frag");
   shader[2] = CreateShaderProg("shader/texture.vert","shader/texture.frag");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}

