//---------------------------------------------------------------------
//            0 8   -   F O R C E   M E A S U R E M E N T
//
// This example demonstrates how to query the HapticMASTER sensors
// and display the data in an OpenGL window.
//---------------------------------------------------------------------

#include "HapticAPI2.h"
#include "HapticMASTER.h"

#define IPADDRESS "192.168.0.25"

#define PosX 0
#define PosY 1
#define PosZ 2

long dev = 0;
char response[100];

double CurrentPosition[3];

double ViewportWidth;
double ViewportHeight;

double dampingCoef[3] = { 30.0, 30.0, 30.0 };

Vector3d force( 0.0, -1.0, 0.0 );

//---------------------------------------------------------------------
// M E A S U R E D   P A R A M E T E R S
//---------------------------------------------------------------------
const int MaxParams = 10;
const int MaxSamples = 50;
const int MaxSampleNr = MaxSamples-1;
int SampleNr;
double ParamSamples[MaxParams][MaxSamples];

double ParamMax[MaxParams] = {0.19, 0.25, 0.2, 1.0, 1.0, 1.0, 35.0, 35.0, 35.0, 10.0};

double ParamScale[MaxParams] = {1.0, 1.0, 1.0, 0.25, 0.25, 0.25, 0.0075, 0.0075, 0.0075, 1.0};

char ParamNameStrings[MaxParams][10] = {"X-Pos", "Y-Pos", "Z-Pos", 
                                        "X-Vel", "Y-Vel", "Z-Vel",
                                        "X-Force", "Y-Force", "Z-Force", "Inertia"};

char ParamUnitStrings[MaxParams][8] = {"[m]", "[m]", "[m]",
                                       "[m/s]", "[m/s]", "[m/s]",
                                       "[N]", "[N]", "[N]", "[kg]"};

char ParamValueStrings[MaxParams][11];

//---------------------------------------------------------------------
// O P E N G L   M A T E R I A L S
//---------------------------------------------------------------------
// EndEffector OpenGL Material Parameters.
GLfloat EndEffectorAmbient[] = {0.91, 0.44, 0.00, 1.00};
GLfloat EndEffectorDiffuse[] = {0.90, 0.38, 0.00, 1.00};

// Spring OpenGL Material Parameters.
GLfloat SpringAmbient[] = {1.00, 0.00, 1.00, 1.00};
GLfloat SpringDiffuse[] = {0.97, 0.0, 0.97, 1.00};

GLfloat DarkGrayLineAmbient[] = {0.25, 0.25, 0.25, 1.00};
GLfloat DarkGrayLineDiffuse[] = {0.25, 0.25, 0.25, 1.00};

GLfloat GrayLineAmbient[] = {0.650, 0.650, 0.650, 1.00};
GLfloat GrayLineDiffuse[] = {0.650, 0.650, 0.650, 1.00};

GLfloat BlueLineAmbient[] = {0.00, 0.76, 0.70, 1.00};
GLfloat BlueLineDiffuse[] = {0.00, 0.90, 0.77, 1.00};

// Block OpenGL Material Parameters.
GLfloat BlockAmbient[] = {0.00, 0.66, 0.60, 1.00};
GLfloat BlockDiffuse[] = {0.00, 0.80, 0.67, 0.28};

// General OpenGL Material Parameters
GLfloat Specular[] = {1.00, 1.00, 1.00, 1.00};
GLfloat Emissive[] = {0.00, 0.00, 0.00, 1.00};
GLfloat Shininess = {128.00};

//---------------------------------------------------------------------
// O B J E C T   P A R A M E T E R S
//---------------------------------------------------------------------
double springStiffness = 100;
double springMaxForce = 2.0;
double springPos[] = {0.15, 0.05, -0.05};
double springDampFactor = 0.7;

//---------------------------------------------------------------------
//              E N D   E F F E C T O R   M A T E R I A L
//
// EndEffectorMaterial() Sets The Current OpenGl Material Paremeters. 
// Call This Function Prior To Drawing The EndEffector.
//---------------------------------------------------------------------
void EndEffectorMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, EndEffectorAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, EndEffectorDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//                    S P R I N G   M A T E R I A L
//
// SpringMaterial() Sets The Current OpenGl Material Paremeters. 
// Call This Function Prior To Drawing The spring.
//---------------------------------------------------------------------
void SpringMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, SpringAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, SpringDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//                  B L O C K   M A T E R I A L
//
// BlockMaterial() Sets The Current OpenGl Material Parameters. 
// Call This Function Prior To Drawing The Block.
//---------------------------------------------------------------------
void BlockMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, BlockAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, BlockDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//                B L U E   L I N E   M A T E R I A L
//
// BlueLineMaterial() Sets The Current OpenGl Material Parameters. 
// Call This Function Prior To Drawing A Line Segment.
//---------------------------------------------------------------------
void BlueLineMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, BlueLineAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, BlueLineDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//                G R A Y   L I N E   M A T E R I A L
//
// GrayLineMaterial() Sets The Current OpenGl Material Parameters. 
// Call This Function Prior To Drawing A Line Segment.
//---------------------------------------------------------------------
void GrayLineMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, GrayLineAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, GrayLineDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//          D A R K   G R A Y   L I N E   M A T E R I A L
//
// DarkGrayLineMaterial() Sets The Current OpenGl Material Parameters. 
// Call This Function Prior To Drawing A Line Segment.
//---------------------------------------------------------------------
void DarkGrayLineMaterial()
{
   glMaterialfv(GL_FRONT, GL_AMBIENT, DarkGrayLineAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, DarkGrayLineDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emissive);
   glMaterialf(GL_FRONT, GL_SHININESS, Shininess);
}

//---------------------------------------------------------------------
//                  D R A W   E N D   E F F E C T O R
//
// This Function Is Called To Draw The Graphic Equivalent Of 
// The EndEffector In OpenGl.
// The EndEffector Is Drawn At The Current Position
//---------------------------------------------------------------------
void DrawEndEffector(void)
{
   EndEffectorMaterial();
   glPushMatrix();
   glTranslatef(CurrentPosition[PosX], CurrentPosition[PosY], CurrentPosition[PosZ]);
   glutSolidSphere(0.005, 20, 20);
   glPopMatrix();
}

//---------------------------------------------------------------------
//                  D R A W   S P R I N G   P O S
//
// This Function Is Called To Draw The origin position of the spring
//---------------------------------------------------------------------
void DrawSpringPos(void)
{
   SpringMaterial();
   glPushMatrix();
   glTranslatef(springPos[PosX], springPos[PosY], springPos[PosZ]);
   glutSolidSphere(0.005, 20, 20);
   glPopMatrix();
}

//---------------------------------------------------------------------
//                         D R A W   S P R I N G
//
// This Function Is Called To Draw The spring itself
//---------------------------------------------------------------------
void DrawSpring(void)
{
   SpringMaterial();
   glBegin(GL_LINES);
      glVertex3f(CurrentPosition[PosX], CurrentPosition[PosY], CurrentPosition[PosZ]);
      glVertex3f(springPos[PosX], springPos[PosY], springPos[PosZ]);
   glEnd();

}

//---------------------------------------------------------------------
//              D R A W   P A R A M   G R A P H
//
// This Function Plots A Graph Of Some HapticMASTER Parameter
// On The Screen
//---------------------------------------------------------------------
void DrawParamGraph(int Param)
{
   int i;
   double ScaleX = 0.7 / MaxSamples;
   double OffsetX = -0.35;
   
   BlueLineMaterial();
   
   if(SampleNr == MaxSampleNr)
   {
      glBegin(GL_LINE_STRIP);
         for(i=0; i<=MaxSampleNr; i++)
            glVertex3f(0.0, OffsetX+(i*ScaleX), (ParamSamples[Param][i]/ParamMax[Param])*0.20);
      glEnd();
   }
   else
   {
      glBegin(GL_LINE_STRIP);\
         for(i=SampleNr+1; i<MaxSamples; i++)
            glVertex3f(0.0, OffsetX+((i-(SampleNr+1))*ScaleX), (ParamSamples[Param][i]/ParamMax[Param])*0.25);

         for(i=0; i<=SampleNr; i++)
            glVertex3f(0.0, OffsetX+((i+MaxSampleNr-SampleNr)*ScaleX), (ParamSamples[Param][i]/ParamMax[Param])*0.25);

      glEnd();
   }

   GrayLineMaterial();
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, -0.35, -0.25);
      glVertex3f(0.0, -0.35, 0.25);
      glVertex3f(0.0, 0.35, 0.25);
      glVertex3f(0.0, 0.35, -0.25);
      glVertex3f(0.0, -0.35, -0.25);
   glEnd();

   glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, 0.0, -0.25);
      glVertex3f(0.0, 0.0, 0.25);
   glEnd();

   DarkGrayLineMaterial();
   for(i=0; i<20; i++)
   {
      glBegin(GL_LINE_STRIP);
         glVertex3f(0.0, -0.35+(i*0.035), -0.25);
         glVertex3f(0.0, -0.35+(i*0.035), 0.25);
      glEnd();
   }

   for(i=0; i<3; i++)
   {
      glBegin(GL_LINE_STRIP);
         glVertex3f(0.0, -0.35, -0.125+(i*0.125));
         glVertex3f(0.0, 0.35, -0.125+(i*0.125));
      glEnd();
   }
   
   BlockMaterial();
   glBegin(GL_POLYGON);
      glVertex3f(0.0, -0.35, -0.25);
      glVertex3f(0.0, -0.35, 0.25);
       glVertex3f(0.0, 0.35, 0.25);
      glVertex3f(0.0, 0.35, -0.25);
   glEnd();
}

//---------------------------------------------------------------------
//                  D R A W   P A R A M   I N F O
//
// This Function Plots A Vu-Meter Like Graph Of Some HapticMASTER Parameter
// On The Screen
//---------------------------------------------------------------------
void DrawParamInfo(int Param)
{
   int i=0;

   BlueLineMaterial();
   glRasterPos3f(0.0, -0.32, 0.15);
   
   while( (i<sizeof(ParamNameStrings[Param])) && (ParamNameStrings[Param][i] != '\0'))
   {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, ParamNameStrings[Param][i]);
      i++;
   }

   for(i=0; i<sizeof(ParamUnitStrings[Param]); i++)
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, ParamUnitStrings[Param][i]);
   
   glRasterPos3f(0.0, -0.25, 0.05);
   for(i=0; i<sizeof(ParamValueStrings[Param]); i++)
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, ParamValueStrings[Param][i]);

   EndEffectorMaterial();
   glBegin(GL_POLYGON);
      glVertex3f(0.0, 0.0, -0.2);
      glVertex3f(0.0, 0.0, -0.125);
       glVertex3f(0.0, (ParamSamples[Param][SampleNr]/ParamMax[Param])*0.3, -0.125);
      glVertex3f(0.0, (ParamSamples[Param][SampleNr]/ParamMax[Param])*0.3, -0.2);
   glEnd();

   GrayLineMaterial();
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, -0.35, -0.25);
      glVertex3f(0.0, -0.35, 0.25);
      glVertex3f(0.0, 0.35, 0.25);
      glVertex3f(0.0, 0.35, -0.25);
      glVertex3f(0.0, -0.35, -0.25);
   glEnd();

   glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, -0.3, -0.2);
      glVertex3f(0.0, -0.3, -0.125);
      glVertex3f(0.0, 0.3, -0.125);
      glVertex3f(0.0, 0.3, -0.2);
      glVertex3f(0.0, -0.3, -0.2);
   glEnd();
   
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, 0.0, -0.22);
      glVertex3f(0.0, 0.0, -0.105);
   glEnd();

   BlockMaterial();
   glBegin(GL_POLYGON);
      glVertex3f(0.0, -0.35, -0.25);
      glVertex3f(0.0, -0.35, 0.25);
      glVertex3f(0.0, 0.35, 0.25);
      glVertex3f(0.0, 0.35, -0.25);
   glEnd();
}

//---------------------------------------------------------------------
//                         I N I T   O P E N   G L
//
// This Function Initializes the OpenGl Graphics Engine
//---------------------------------------------------------------------
void InitOpenGl (void)
{
   glShadeModel(GL_SMOOTH);

   glLoadIdentity();
   
   GLfloat GrayLight[] = {0.75, 0.75, 0.75, 1.0};
   GLfloat LightPosition[] = {1.0, 2.0, 1.0, 0.0};
   GLfloat LightDirection[] = {0.0, 0.0, -1.0, 0.0};

   glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
   glLightfv(GL_LIGHT0, GL_AMBIENT, GrayLight);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, GrayLight);
   glLightfv(GL_LIGHT0, GL_SPECULAR, GrayLight);
   
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   
   glEnable (GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   glClearColor(0.0, 0.0, 0.3, 0.0);
}

//---------------------------------------------------------------------
//                           D I S P L A Y
//
// This Function Is Called By OpenGL To Redraw The Scene
// Here's Where You Put The EndEffector And Block Drawing FuntionCalls
//---------------------------------------------------------------------
void Display (void)
{
   int i;
   
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glPushMatrix ();
    
   // define eyepoint in such a way that
   // drawing can be done as in lab-frame rather than sgi-frame
   // (so X towards user, Z is up)
   
   glViewport (0, ViewportHeight*((MaxParams)/2), ViewportWidth*5, ViewportHeight*((MaxParams)/2));

   gluLookAt (1.0, 0.5, 0.35, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
   glutPostRedisplay();
   
   DrawAxes();
   DrawWorkspace(dev, 3);

   // Get The Current EndEffector Position From THe HapticMASTER
   haSendCommand( dev, "get modelpos", response );
   if ( strstr ( response, "--- ERROR:" ) ) {
      printf("get modelpos ==> %s", response);
      getchar();
      exit(-1);
   }
   else {
      ParseFloatVec( response, CurrentPosition[PosX], CurrentPosition[PosY], CurrentPosition[PosZ] ); 
   }

   DrawEndEffector();
   DrawSpring();
   DrawSpringPos();

   glPopMatrix();
   
   glPushMatrix();
   gluLookAt (1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

   SampleNr++;
   if (SampleNr >= MaxSamples)
      SampleNr = 0;

   haSendCommand(dev, "get modelpos", response);
   if ( strstr ( response, "--- ERROR:" ) ) {
      printf("get modelpos ==> %s\n", response);
      getchar();
      exit(-1);
   }
   else {
      ParseFloatVec(response, ParamSamples[0][SampleNr], ParamSamples[1][SampleNr], ParamSamples[2][SampleNr]);
   }

   haSendCommand(dev, "get modelvel", response);
   if ( strstr ( response, "--- ERROR:" ) ) {
      printf("get modelvel ==> %s\n", response);
      getchar();
      exit(-1);
   }
   else {
      ParseFloatVec(response, ParamSamples[3][SampleNr], ParamSamples[4][SampleNr], ParamSamples[5][SampleNr]);
   }

   haSendCommand(dev, "get measforce", response);
   if ( strstr ( response, "--- ERROR:" ) ) {
      printf("get measforce ==> %s\n", response);
      getchar();
      exit(-1);
   }
   else {
      ParseFloatVec(response, ParamSamples[6][SampleNr], ParamSamples[7][SampleNr], ParamSamples[8][SampleNr]);
   }

   haSendCommand(dev, "get inertia", response);
   if ( strstr ( response, "--- ERROR:" ) ) {
      printf("get inertia ==> %s\n", response);
      getchar();
      exit(-1);
   }
   else {
      ParamSamples[9][SampleNr] = atof(response);
   }
   
   for(i=0; i<MaxParams; i++)
      sprintf(ParamValueStrings[i], "%+08.5f", ParamSamples[i][SampleNr]);

   // For Each Parameter Draw The Graph
   for(i=0; i<MaxParams; i++)
   {
      glViewport(6*ViewportWidth, (MaxParams-i-1)*ViewportHeight, 4*ViewportWidth, ViewportHeight);
      DrawParamGraph(i);
   }

   // For Each Parameter Draw The Meter
   for(i=0; i<MaxParams; i++)
   {
      glViewport(5*ViewportWidth, (MaxParams-i-1)*ViewportHeight, ViewportWidth, ViewportHeight);
      DrawParamInfo(i);
   }

   glPopMatrix ();
   
   glutSwapBuffers();
}

//---------------------------------------------------------------------
//                            R E S H A P E 
//
// The Function Is Called By OpenGL Whenever The Window Is Resized
//---------------------------------------------------------------------
void Reshape(int iWidth, int iHeight)
{
   glViewport (0, 0, (GLsizei)iWidth, (GLsizei)iHeight);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();

   float fAspect = (float)iWidth/iHeight;
   gluPerspective (30.0, fAspect, 0.05, 20.0);            
 
   ViewportWidth = ((GLsizei)glutGet(GLUT_WINDOW_WIDTH)/10);
   ViewportHeight = ((GLsizei)glutGet(GLUT_WINDOW_HEIGHT)/MaxParams);

   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();
}

//---------------------------------------------------------------------
//                            K E Y B O A R D
//
// This Function Is Called By OpenGl WhenEver A Key Was Hit
//---------------------------------------------------------------------
void Keyboard(unsigned char ucKey, int iX, int iY)
{
   switch (ucKey) 
   {
      case 27: // Esc
         haSendCommand(dev, "remove all", response);
         printf("remove all ==> %s\n", response);
         
         haSendCommand(dev, "set state stop", response);
         printf("set state stop ==> %s\n", response);
         
         exit(0);
         break;

     case 101: // "e"
         haSendCommand(dev, "set myDrivingForce force", force.x, force.y, force.z, response);
         printf("set myDrivingForce force [%g,%g,%g] ==> %s\n", force.x, force.y, force.z, response);
         if (strstr(response, "--- ERROR:")) {
             printf("set myDrivingForce force ==> %s", response);
             getchar();
             exit(-1);
         }
         break;

     case 114: // "r"
         haSendCommand(dev, "set myDrivingForce force", 0, 0, 0, response);
         printf("set myDrivingForce force [0,0,0] ==> %s\n", response);
         break;
   }
}

//---------------------------------------------------------------------
//                              M A I N
//
// 08-Force-Measurement Main Function
//---------------------------------------------------------------------
int main(int argc, char** argv)
{
   // Call The Initialize HapticMASTER Function
   dev = haDeviceOpen( IPADDRESS );

   if( dev == HARET_ERROR ) {
      printf( "--- ERROR: Unable to connect to device: %s\n", IPADDRESS );
      return HARET_ERROR;
   }
   else {
      InitializeDevice( dev );

      // Create a damper effect 
      if ( haSendCommand (dev, "create damper myDamper", response) ) {
          printf("--- ERROR: Could not send command create damper myDapmer\n");
          getchar();
          exit(-1);
      }

      printf("create damper myDamper ==> %s\n", response);

      if (strstr(response, "--- ERROR:")) {
          getchar();
          exit(-1);
      }
      else {
          haSendCommand (dev, "set myDamper dampcoef", dampingCoef[0], dampingCoef[1], dampingCoef[2], response);
          printf("set myDamper dampcoef [%g,%g,%g] ==> %s\n", dampingCoef[0], dampingCoef[1], dampingCoef[2], response);

          haSendCommand(dev, "set myDamper enable", response);
          printf("set myDamper enable ==> %s\n", response);
      }

      // Create a spring effect
      if ( haSendCommand (dev, "create spring mySpring", response) ) {
         printf ( "--- ERROR: Could not send command create spring mySpring\n" );
      }

      printf( "create spring mySpring ==> %s\n", response);

      if ( strstr ( response, "--- ERROR:" ) ) {
         getchar();
         exit(-1);
      }
      else {
         haSendCommand (dev, "set mySpring stiffness", springStiffness, response);
         printf( "set mySpring stiffness %g ==> %s\n", springStiffness, response);
         
         haSendCommand (dev, "set mySpring dampfactor", springDampFactor, response);
         printf( "set mySpring dampfactor %g ==> %s\n", springDampFactor, response);
         
         haSendCommand (dev, "set mySpring pos", springPos[PosX], springPos[PosY], springPos[PosZ], response);
         printf( "set mySpring pos [%g,%g,%g] ==> %s\n", springPos[PosX], springPos[PosY], springPos[PosZ], response);
         
         haSendCommand (dev, "set mySpring maxforce", springMaxForce, response);
         printf( "set mySpring maxforce %g ==> %s\n", springMaxForce, response);
         
         haSendCommand (dev, "set mySpring enable", response);
         printf( "set mySpring enable ==> %s\n", response);
      }

      // Create a bias force Effect and supply it with parameters
      if ( haSendCommand (dev, "create biasforce myDrivingForce", response) ) {
          printf("--- ERROR: Could not send command create biasforce myDrivingForce\n");
          getchar();
          exit(-1);
      }

      printf("create biasforce myDrivingForce ==> %s\n", response);

      if (strstr(response, "--- ERROR:")) {
          getchar();
          exit(-1);
      }

      haSendCommand(dev, "set myDrivingForce enable", response);
      printf("set myDrivingForce enable ==> %s\n", response);

      // OpenGL Initialization Calls
      glutInit(&argc, argv);
      glutInitDisplayMode (GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH);
      glutInitWindowSize (1024, 768);

      // Create The OpenGlWindow
	   glutCreateWindow ("HapticAPI Programming Manual : Example08: Force Measurement");

      InitOpenGl();

      // More OpenGL Initialization Calls
      glutReshapeFunc (Reshape);
      glutDisplayFunc(Display);
      glutKeyboardFunc (Keyboard);
      glutMainLoop();
   }
   return 0; 
}