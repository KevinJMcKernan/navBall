// Main Sources:
// Dr. Christensen's TCP Client/Server Example
// www.khronos.org/registry/OpenGL-Refpages
// F.S.Hill Jr. "Computer Graphics Using OpenGL"
// www.glprogramming.com/red/about.html

// Compile and link with gcc navDisplay.c -o t1 -lGL -lGLU -lglut
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <GL/freeglut.h>
#include "RGBpixmap.h"
#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <unistd.h>
#include "SOIL.h"

// Sockets example from Christensen's tool page.
#define  BSD                // WIN for Winsock and BSD for BSD sockets
//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#ifdef WIN
  #include <windows.h>      // Needed for all Winsock stuff
#endif
#ifdef BSD
  #include <unistd.h>
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
#endif

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM         1052   // Port number used at the server
// Remote was 127.0.0.141
#define  IP_ADDR    "127.0.0.1"  // IP address of server (*** HARDWIRED ***)
#define  radius  110
#define PI 3.1459265

// Graphics Library Unsigned Binary Integers
GLuint navBallTexture = 1;
GLuint airSpeedTexture = 2;
GLuint altitudeTexture = 3;
GLuint compassTexture = 4;
// struct for pos and destination
struct flightData{
	float pitch, roll, airspeed, heading, slipSkid, localizerScale, glideSlope;
	int altitude;
};

struct flightData position;
struct flightData destination;
// PNG. You can make one in MS paint.
const char navBallFileName[] = "imageTextures/newBall.png";
const char airSFileName[] = "imageTextures/t8.png";
const char altitudeFileName[] = "imageTextures/t6a.png";
const char compassFileName[] =  "imageTextures/compassFinal.png";

// Temporary values for moving objects.
GLfloat moveAirStripUpTest = 3;
GLfloat moveAltitudeUpTest = 0;

// Constants for our lighting and image. Edit these to change
// Lighting.
// Resource: www.glprogramming.com/red/chapter05.html
// See example 5.2

// This specifies light position.
const GLfloat LIGHT_0_POSITION[] = {1.0, 0.0, 1.0, 0.0}; 	// Light Position.
// Ambient light is the light already present in the scene.
const GLfloat LIGHT_AMBIENT[] = {1.0, 1.0, 1.0, 1.0};		// RGBA ambient lighting. was .8 x 3
// Direct light hitting the object and reflecting, angle proportional to intensity.
const GLfloat LIGHT_DIFFUSE[] = {1.0, 1.0, 1.0, 1.0};
// This is the direct ray from the reflection.
const GLfloat LIGHT_SPECULAR[] = {1.0, 1.0, 1.0, 1.0};		// Specular settings.
// ratio of width to heigh of an image.
const GLfloat ASPECT_RATIO = 1.0;				// We could probably bump this down to 1.0
// TODO: edit LIGHT_MODEL_AMBIETN VALUES.
const GLfloat LIGHT_MODEL_AMBIENT[] = {1.0, 1.0, 1.0, 1.0}; // was .2, .2, .2, 1.0

// Time measurement Variables
struct timeval tv1;
struct timeval tv2;

// General Variable for manipulating our Ball.
GLfloat theta[] = {0.0, 0.0, 0.0};	// Used in our display function.

// Setup light source named GL_LIGHT0
void setupLighting() {
	glLightfv(GL_LIGHT0, GL_AMBIENT, LIGHT_AMBIENT);	// Set ambient lighting.
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LIGHT_DIFFUSE);	// Set diffuse lighting.
	glLightfv(GL_LIGHT0, GL_SPECULAR, LIGHT_SPECULAR);	// Set specular lighting.
	glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_0_POSITION);	// Set the light source position.
}

int                  client_s;        // Client socket descriptor
struct sockaddr_in   server_addr;     // Server Internet address
char                 out_buf[4096];   // Output buffer for data
char                 in_buf[4096] ={'\0'};    // Input buffer for data
int                  retcode;         // Return code
char *in_buf_ptr = in_buf;

// Update our lighting.
void updateLighting() {
	// Push any transformation onto the model view matrix stack.
	glPushMatrix();
	glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_0_POSITION);
	// Pop a matrix off the stack.
	glPopMatrix();
	// Enable lighting.
	glEnable(GL_LIGHT0);
}

int flightDataPtr; // pointer to flight data (destination)

void receiveFlightData (void){
	retcode = recv(client_s, in_buf, sizeof(in_buf), 0);
	if(retcode < 0)	{
		printf("*** ERROR - recv() failed \n");
		exit(-1);
	}

	float i = 0;
	std::vector<float> vect;
	std::stringstream ss(in_buf_ptr);

	while (ss >> i) {
       		vect.push_back(i);
		if (ss.peek() == ','){
        		ss.ignore();
	        }
    	}

	destination.pitch 		= vect[0];
        destination.roll		= vect[1];
        destination.airspeed		= vect[2];
        destination.heading		= vect[3];
        destination.slipSkid		= vect[4];
        destination.localizerScale	= vect[5];
        destination.glideSlope		= vect[6];
        destination.altitude		= int(vect[7]);

	printf("Pitch = %f Roll = %f \n",destination.pitch, destination.roll);
    	retcode = send(client_s, in_buf, (strlen(in_buf) + 1), 0);
	if(retcode < 0) {
		printf("*** ERROR - send() failed \n");
		exit(-1);
	}
    	printf("After Print\n");
}

// get the struct data from server and populate struct

void getDataFillStruct( void  ){// set globaly: struct flightData position
	// call client to fill struct with destination // destination = destination // then rotates ball 1 degree for each
	//printf("Received from server:");
	// call the client struct filler
	receiveFlightData();
	//check pos of pitch
	if( position.pitch < destination.pitch ){
		glRotatef(1,1,0, 0);

	position.pitch += 1;
	}
	if(position.pitch > destination.pitch ){
		glRotatef(-1,1,0, 0);
		position.pitch -= 1;
	}
	if(position.roll < destination.roll){
		glRotatef(1,0,1, 0);
		position.roll += 1;
	}
	if(position.roll  > destination.roll ){
		glRotatef(-1,0,1, 0);
		position.roll -= 1;
	}
	return;
}

// Draw the navball
void drawNavBall(GLuint navBallTexture, GLfloat navBallRadius){
	// use GLU to create a quadric surface reference
        GLUquadricObj* ball = gluNewQuadric();
	// Specificy the surface normals.
        gluQuadricNormals(ball, GLU_SMOOTH);
	// For our object reference turn texture on.
	gluQuadricTexture(ball, GL_TRUE);
	// Enable 2D textures.
        glEnable(GL_TEXTURE_2D);
	// Push
        glPushMatrix();
	// Setup the texture environment
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// Push
        glPushMatrix();
	// This controls translation: 1st arg +right, 2nd arg +up, 3rd arg +closer
	glTranslatef(0.0, 0.5, -2.0);
	// This controls rotation.
	//glRotatef(-90.0, 1.0, 0.0, 0.0);
	//glRotatef(180.0, 0.0, 1.0, 0.0);
	//glRotatef(90.0, 0.0, 0.0, 1.0);// this rotates to hz
	//glRotatef(theta[0], 0.0, 1.0, 0.0);
	glRotatef(theta[0], 1.0, 0.0, 0.0);
	printf("\n%f\n", destination.roll);
	glRotatef(destination.roll, 0.0, 1.0, 0.0);
	glRotatef(destination.heading, 0.0, 0.0, 1.0);

	// Setup texture binding between the references.
        glBindTexture(GL_TEXTURE_2D, navBallTexture);
	// Create the sphere with the texture and longitude & lat. divisions,

        gluSphere(ball, navBallRadius, 48, 48);
	// Pop x2
        glPopMatrix();
	glPopMatrix();
	// Cleanup
        glDisable(GL_TEXTURE_2D);
        gluDeleteQuadric(ball);
}

// Draw the Altitude Strip
void drawAltitudeStrip(GLuint altitudeTexture){
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
	// 3rd arg previously MODULATE
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glPushMatrix();
        // Enable 2D textures.
        glBindTexture(GL_TEXTURE_2D, altitudeTexture);
	// Positioning.
	glTranslatef(.95, -0.21 - (0.00068 * destination.altitude), 2.0);
	// Create polygon object that has bound texture.
	glBegin(GL_POLYGON);
	    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0, 16.0, 0.0);
	    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0, 0.0, 0.0);
	    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, 0.0, 0.0);
	    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, 16.0, 0.0);
	glEnd();
	glPopMatrix();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Draw the Altitude Strip
void drawAirSpeedStrip(GLuint airSpeedTexture){
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        // 3rd arg previously MODULATE
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glPushMatrix();
        // Enable 2D textures.
        glBindTexture(GL_TEXTURE_2D, airSpeedTexture);
        // Second arguement is up
        // first arguement is left/right
        // 2, -5, 4
	// Finding zero position.
        glTranslatef(-2.0, -2.95 - (.0704 * destination.airspeed), 2.0);
        glBegin(GL_POLYGON);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0, 32.0, 0.0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0, 0.0, 0.0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, 0.0, 0.0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, 32.0, 0.0);
        glEnd();
        glPopMatrix();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
}

// Draw the compass strip
void drawCompass(GLuint compassTexture) {
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        // 3rd arg previously MODULATE
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glPushMatrix();
        // Enable 2D textures.
        glBindTexture(GL_TEXTURE_2D, compassTexture);
        // Second arguement is up
        // first arguement is left/right
        // 2, -5, 4
        glTranslatef(-1.5 +(-.0335 * destination.heading), -2.15, 1.9);
        glBegin(GL_POLYGON);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0, 1.0, 0.0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0, 0.0, 0.0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(16.0, 0.0, 0.0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(16.0, 1.0, 0.0);
        glEnd();
        glPopMatrix();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
}

// Draw a compass cover for bottom right corner.
void drawCompassCover(void){
	glDisable(GL_LIGHTING);
	glTranslatef(1.03, -2.14, 1.95);
	glBegin(GL_POLYGON);
		glColor3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(1.0, 2.0, 0.0);
		glVertex3f(0.0, 2.0, 0.0);
	glEnd();
	glEnable(GL_LIGHTING);

}

// Draw airplane  Left Wing
void drawAirplaneLeftWing(void){
	glPushMatrix();
        glTranslatef(-0.55, 0.275, 2.0); // 1: left/right, 2: upDown
	glDisable(GL_LIGHTING);
        glBegin(GL_POLYGON);
		glColor3f(0.0, 0.0, 0.0);
                glVertex3f(0.0, 0.05, 0.0); // #1
		glVertex3f(0.0, 0.0, 0.0); // #2
		glVertex3f(0.4, 0.0, 0.0); // #3
		glVertex3f(0.4, 0.05, 0.0);
        glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

// Draw airplane right wing
void drawAirplaneRightWing(void){
        glPushMatrix();
        glTranslatef(.15, 0.275, 2.0);
	glDisable(GL_LIGHTING);
		glColor3f(0.0, 0.0, 0.0);

        glBegin(GL_POLYGON);
                glVertex3f(0.0, 0.05, 0.0); // #1
                glVertex3f(0.0, 0.0, 0.0); // #2
                glVertex3f(0.4, 0.0, 0.0); // #3
                glVertex3f(0.4, 0.05, 0.0);
        glEnd();
	glEnable(GL_LIGHTING);
        glPopMatrix();
}

void drawAirplaneTinyBall(void){
        // use GLU to create a quadric surface reference
        GLUquadricObj* planeBall = gluNewQuadric();
        // Specificy the surface normals.
        gluQuadricNormals(planeBall, GLU_SMOOTH);
        // For our object reference turn texture on.
        // Push
        glPushMatrix();
        // Push
        glPushMatrix();
        // This controls translation: 1st arg +right, 2nd arg +up, 3rd arg +closer
        glTranslatef(0.0, .5, 2.0);
        // Setup texture binding between the references.
        // Create the sphere with the texture and longitude & lat. divisions,
        gluSphere(planeBall, .05, 10, 10);
        // Pop x2
        glPopMatrix();
        glPopMatrix();
        // Cleanup
        gluDeleteQuadric(planeBall);

}

void drawAltitudeText(int altitude){
	std:stringstream ss;
	ss << altitude;
	std::string str = ss.str();
	glDisable(GL_LIGHTING);
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos3f(0.398, 0.07, 4.0);
	for(int i = 0; i < str.length(); i++){
		// GLUT_BITMAP_TIMES_ROMAN_24
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
	}
	glEnable(GL_LIGHTING);

}

void drawAltitudeBox(void){
        glPushMatrix();
        glTranslatef(.85, 0.05, 2.5);
        glDisable(GL_LIGHTING);
        glBegin(GL_POLYGON);
		glColor3f(0.0, 0.400, 0.010);
                glVertex3f(0.0, 0.29, 0.0); // #1
                glVertex3f(0.0, 0.0, 0.0); // #2
                glVertex3f(0.6, 0.0, 0.0); // #3
                glVertex3f(0.6, 0.29, 0.0);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
}

void drawAirspeedBox(void){
	glPushMatrix();
	glTranslatef(-1.49, 0.05, 2.5);
	glDisable(GL_LIGHTING);
        glBegin(GL_POLYGON);
                glColor3f(0, 0.400, 0.010);
                glVertex3f(0.0, 0.29, 0.0); // #1
                glVertex3f(0.0, 0.0, 0.0); // #2
                glVertex3f(0.6, 0.0, 0.0); // #3
                glVertex3f(0.6, 0.29, 0.0);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
}

void drawAirspeedText(float airspeed){
        std:stringstream ss;
        ss << (int)airspeed;
        std::string str = ss.str();
        glDisable(GL_LIGHTING);
        glColor3f(0.0, 0.0, 0.0);
        glRasterPos3f(-.525, 0.07, 4.0);
        for(int i = 0; i < str.length(); i++){
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
        }
        glEnable(GL_LIGHTING);
}

void drawSlipBox(void){
	glLineWidth(3.0);
	glDisable(GL_LIGHTING);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
		glVertex3f(-0.5, 2.5, 0.0);
		glVertex3f(0.5, 2.5, 0.0);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(-0.5, 2.8, 0.0);
		glVertex3f(0.5, 2.8, 0.0);
        glEnd();
        glBegin(GL_LINES);
                 glVertex3f(-0.5, 2.8, 0.0);
                 glVertex3f(-0.5, 2.5, 0.0);
        glEnd();
        glBegin(GL_LINES);
                glVertex3f(0.5, 2.8, 0.0);
                glVertex3f(0.5, 2.5, 0.0);
        glEnd();
        glBegin(GL_LINES);
                glVertex3f(-0.17, 2.8, 0.0);
                glVertex3f(-0.17, 2.5, 0.0);
        glEnd();
        glBegin(GL_LINES);
                glVertex3f(0.17, 2.8, 0.0);
	        glVertex3f(0.17, 2.5, 0.0);
        glEnd();

	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
}

void drawGlideSlopeBox(void){
        glLineWidth(2.0);
	glDisable(GL_LIGHTING);
        glColor3f(0.0, 1.0, 0.0);
	// tiny box 1
	glBegin(GL_LINES);
               glVertex3f(0.125, 0.12, 4.5);
               glVertex3f(0.135, 0.12, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.12, 4.5);
               glVertex3f(0.125, 0.11, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.11, 4.5);
               glVertex3f(0.135, 0.11, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.135, 0.11, 4.5);
               glVertex3f(0.135, 0.12, 4.5);
        glEnd();

        // tiny box 2
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.08, 4.5);
               glVertex3f(0.135, 0.08, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.08, 4.5);
               glVertex3f(0.125, 0.07, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.07, 4.5);
               glVertex3f(0.135, 0.07, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.135, 0.07, 4.5);
               glVertex3f(0.135, 0.08, 4.5);
        glEnd();

        // tiny box 3
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.045, 4.5);
               glVertex3f(0.135, 0.045, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.045, 4.5);
               glVertex3f(0.125, 0.035, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.035, 4.5);
               glVertex3f(0.135, 0.035, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.135, 0.035, 4.5);
               glVertex3f(0.135, 0.045, 4.5);
        glEnd();

        // tiny box 1
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.01, 4.5);
               glVertex3f(0.135, 0.01, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.01, 4.5);
               glVertex3f(0.125, 0.0, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, 0.0, 4.5);
               glVertex3f(0.135, 0.0, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.135, 0.0, 4.5);
               glVertex3f(0.135, 0.01, 4.5);
        glEnd();


	// last tiny box
        glBegin(GL_LINES);
               glVertex3f(0.125, -0.04, 4.5);
               glVertex3f(0.135, -0.04, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, -0.04, 4.5);
               glVertex3f(0.125, -0.03, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.125, -0.03, 4.5);
               glVertex3f(0.135, -0.03, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.135, -0.03, 4.5);
               glVertex3f(0.135, -0.04, 4.5);
        glEnd();

}

void drawGlideSlopeBall(float position){
        // use GLU to create a quadric surface reference
        GLUquadricObj* planeBall = gluNewQuadric();
        // Specificy the surface normals.
        gluQuadricNormals(planeBall, GLU_SMOOTH);
        // For our object reference turn texture on.
        // Push
        glPushMatrix();
        glColor3f(127.0, 0.0, 255.0);
        // Push
        glPushMatrix();
        // This controls translation: 1st arg +right, 2nd arg +up, 3rd arg +closer
        glTranslatef(0.259, 0.08 + (0.067 * destination.glideSlope), 4.0);
        // Setup texture binding between the references.
        // Create the sphere with the texture and longitude & lat. divisions,
        gluSphere(planeBall, .01, 10, 10);
        // Pop x2
        glPopMatrix();
        glPopMatrix();
        // Cleanup
        gluDeleteQuadric(planeBall);
}

void drawLocalizerBox(float position){
        glPushMatrix();
        glTranslatef(0.0 + (destination.localizerScale * 0.0400), 0.0, 4.5);
        glDisable(GL_LIGHTING);
        glBegin(GL_POLYGON);
	float offset = .005;
        glColor3f(127.0, 0.0, 255.0);
		// grab coordinates of leftest
            	glVertex3f(-0.01 + offset, -0.14, 0.0); // #1
	        glVertex3f(0.00 + offset, -0.14, 0.0); // #2
	        glVertex3f(0.00 + offset, -0.13, 0.0); // #3
        	glVertex3f(-0.01 + offset, -0.13, 0.0);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();

}

void drawSlipBall(float position){
        // use GLU to create a quadric surface reference
        GLUquadricObj* planeBall = gluNewQuadric();
        // Specificy the surface normals.
        gluQuadricNormals(planeBall, GLU_SMOOTH);
        // For our object reference turn texture on.
        // Push
        glPushMatrix();
        // Push
        glPushMatrix();
        // This controls translation: 1st arg +right, 2nd arg +up, 3rd arg +closer
        glTranslatef(0.0 + (0.13333 * destination.slipSkid), 1.60, 2.0);
        // Setup texture binding between the references.
        // Create the sphere with the texture and longitude & lat. divisions,
        gluSphere(planeBall, .05, 10, 10);
        // Pop x2
        glPopMatrix();
        glPopMatrix();
        // Cleanup
        gluDeleteQuadric(planeBall);

}

void drawLocalizerScale(float position){
	glLineWidth(2.0);
        glDisable(GL_LIGHTING);
        glColor3f(0.0, 1.0, 0.0);
	float offset = -0.005;
        // far right
        glBegin(GL_LINES);
               glVertex3f(0.09 + offset, -0.14, 4.5);
               glVertex3f(0.08 + offset, -0.14, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.08 + offset, -0.14, 4.5);
               glVertex3f(0.08 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.08 + offset, -0.13, 4.5);
               glVertex3f(0.09 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.09 + offset, -0.13, 4.5);
               glVertex3f(0.09 + offset, -0.14, 4.5);
        glEnd();


        // 2nd from middle on right
        glBegin(GL_LINES);
               glVertex3f(0.05 + offset, -0.14, 4.5);
               glVertex3f(0.04 + offset, -0.14, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.04 + offset, -0.14, 4.5);
               glVertex3f(0.04 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.04 + offset, -0.13, 4.5);
               glVertex3f(0.05 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.05 + offset, -0.13, 4.5);
               glVertex3f(0.05 + offset, -0.14, 4.5);
        glEnd();

        // middle box
        glBegin(GL_LINES);
               glVertex3f(0.01 + offset, -0.14, 4.5);
               glVertex3f(0.00 + offset, -0.14, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.00 + offset, -0.14, 4.5);
               glVertex3f(0.00 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.00 + offset, -0.13, 4.5);
               glVertex3f(0.01 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(0.01 + offset, -0.13, 4.5);
               glVertex3f(0.01 + offset, -0.14, 4.5);
        glEnd();

        // 2nd from middle box on left
        glBegin(GL_LINES);
               glVertex3f(-0.04 + offset, -0.14, 4.5);
               glVertex3f(-0.03 + offset, -0.14, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.03 + offset, -0.14, 4.5);
               glVertex3f(-0.03 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.03 + offset, -0.13, 4.5);
               glVertex3f(-0.04 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.04 + offset, -0.13, 4.5);
               glVertex3f(-0.04 + offset, -0.14, 4.5);
        glEnd();

	// far left box
        // 2nd from middle box on left
        glBegin(GL_LINES);
               glVertex3f(-0.08 + offset, -0.14, 4.5);
               glVertex3f(-0.07 + offset, -0.14, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.07 + offset, -0.14, 4.5);
               glVertex3f(-0.07 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.07 + offset, -0.13, 4.5);
               glVertex3f(-0.08 + offset, -0.13, 4.5);
        glEnd();
        glBegin(GL_LINES);
               glVertex3f(-0.08 + offset, -0.13, 4.5);
               glVertex3f(-0.08 + offset, -0.14, 4.5);
        glEnd();
}

void drawCompassBox(float heading) {
        glPushMatrix();
        glTranslatef(-0.03, -0.2, 4.5);
        glDisable(GL_LIGHTING);
        glBegin(GL_POLYGON);
                glColor3f(0.0, 0.05, 1.0);
                glVertex3f(0.00, 0.05, 0.0); // #1
                glVertex3f(0.00, 0.0, 0.0); // #2
                glVertex3f(0.06, 0.0, 0.0); // #3
                glVertex3f(0.06, 0.05, 0.0);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
}


// Draw Roll.
void drawRoll(void){
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	// Various values.
	double cy = 0.5;	// Center of sphere x
	double cx = 0.0;	// Center of sphere y
	double x = 0.0;		// Value for XCoord of one vertex of the dash
	double y = 0.0;		// Value for YCoord of one vertex of the dash
	double x1 = 0.0;
	double y1 = 0.0;
	double x2 = 0.0;
	double y2 = 0.0;
	double xInner = 0.0;	// Inner XCoord
	double yInner = 0.0;	// Inner YCoord
	double r = 2.3;		// Radius of sphere.
	double a = 0.0;		// Angle in degrees.
	int i = 0;
	// This draws the fine grain markers every 11.25 degrees.
	for(i = 4; i < 14; i++){
		// Outer Point
		x = cx + r * cos(a);
		y = cy + r * sin(a);
		// Inner Point: smaller circle circumscribed.
		xInner = cx + (r - .1) * cos(a);
		yInner = cy + (r - .1) * sin(a);
		// Draw the dashes
		glLineWidth(1.0);
        	glBegin(GL_LINES);
			glVertex3f(x, y, -2.0); // outer point
        	        glVertex3f(xInner, yInner, -2.0); // inner point
	        glEnd();
		a = (PI/180) * (i*11.25);
	}
	a = 0;
	// This draws the the triangles every 45*
	for(i = 1; i < 5; i++){
		// Inner Point of Triangle
 		xInner = cx + (r - .1) * cos(a);
		yInner = cy + (r - .1) * sin(a);
		// Outer Point 1
                x1 = cx + (r + .1) * cos(a - 0.0314);
                y1 = cy + (r + .1) * sin(a - 0.0314);
		// Outer Point 2
                x2 = cx + (r + .1) * cos(a + 0.0314);
                y2 = cy + (r + .1)  * sin(a + 0.0314);
		glLineWidth(2.0);
		glBegin(GL_POLYGON);
			glVertex3f(xInner, yInner, -2.0);
			glVertex3f(x2, y2, -2.0);
			glVertex3f(x1, y1, -2.0);
		glEnd();
		a = (PI/180)*(i*45.0);
	}

}


void drawDecorators(void){
	glLineWidth(8.0);
	glDisable(GL_LIGHTING);
	glColor3f(0.6, 0.6, 0.6);
        // Left Line
	glBegin(GL_LINES);
               glVertex3f(-0.175, 2.0, 4.5);
               glVertex3f(-0.175, -0.5, 4.5);
	glEnd();
	// Right Line
	glBegin(GL_LINES);
               glVertex3f(0.165, 2.0, 4.5);
               glVertex3f(0.165, -0.5, 4.5);
	glEnd();
	// Compass Strip Dividing Line
	glBegin(GL_LINES);
		glVertex3f(-0.175, -0.185, 4.5);
		glVertex3f(0.165, -0.185, 4.5);
	glEnd();
	// Box Box
	glLineWidth(4.0);
        glBegin(GL_LINES); // Top Side
                glVertex3f(-0.175, 0.07, 4.5);
                glVertex3f(-0.3, 0.07, 4.5);
        glEnd();
        glBegin(GL_LINES); // Bottom
                glVertex3f(-0.175, 0.008, 4.5);
                glVertex3f(-0.3, 0.008, 4.5);
        glEnd();
	// Box Box
	glBegin(GL_LINES);
		glVertex3f(0.17-0.005, 0.07, 4.5);
		glVertex3f(0.3-0.005, 0.07, 4.5);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(0.17-0.005, 0.008, 4.5);
		glVertex3f(0.3-0.005, 0.008, 4.5);
	glEnd();

	glEnable(GL_LIGHTING);

}


void drawCompassNeedle(void){
      glLineWidth(8.0);
      glDisable(GL_LIGHTING);
      glColor3f(0.500, 0.216, 0.066);
        // middle line
      glBegin(GL_LINES);
               glVertex3f(0.0, -0.2 - 0.01, 4.5);
               glVertex3f(0.0, -0.3, 4.5);
      glEnd();

}

// MakeImage usage: makeImage(fileName, textureName, hasAlpha);
// Function from adam coffman solar-system-opengl
// Uses F.S. Hill's RGBPixmap files.
void makeImage(const char bitmapFilename[], GLuint &textureName, bool hasAlpha) {
                RGBpixmap pix;
                pix.readBMPFile(bitmapFilename, hasAlpha);
                pix.setTexture(textureName);
                return;
}

// Call previously defined function, allows for expansion with more objects.
// Make all our images: ex. NavBall texture, sliding rule textures.
void makeAllImages() {
	// Use bitmap to create texture for navBall
	// makeImage(navBallFileName, navBallTexture, false);
	navBallTexture = SOIL_load_OGL_texture(
		navBallFileName,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

	// Use png to create texture for strip
	altitudeTexture = SOIL_load_OGL_texture(
		altitudeFileName,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	// Error Check
	if(0 == altitudeTexture) {
		printf("Error loading texture!\n");
	}
	// Use png to create texture for strip.
	airSpeedTexture = SOIL_load_OGL_texture(
	        airSFileName,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
	// Error Check
	if(airSpeedTexture == 0) {
		printf("Error loading texture!\n");
	}
	// Use png to create texture for strip.
	compassTexture = SOIL_load_OGL_texture(
                compassFileName,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
	if(compassTexture == 0){
		printf("Error loading texture!\n");
	}

        return;
}

//GLfloat theta[] = {(-90.0+destination.pitch), 0.0, 0.0};	// Used in our display function.
// Display FCN
void display(void) {
  gettimeofday(&tv1, NULL);
  getDataFillStruct();
	// Initialize lighting model
	theta[0] =  destination.pitch-90;

	// Initialize lighting model
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LIGHT_MODEL_AMBIENT);
	// Enable the lighting model.
	glEnable(GL_LIGHTING);
	// Setup viewing camera
	glMatrixMode(GL_PROJECTION);
	// Clean the current matrixc loaded.
	glLoadIdentity();
	// Setup 60 degree viewing perspective.
	gluPerspective(60, 1, 0.2, 10.0);
	// Clear buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.05, 0.05, 0.05, 1.0);
	// Load matrix modes.
	glMatrixMode(GL_MODELVIEW);
	// Clean
	glLoadIdentity();
	// Push stuff back so we can see it 2 units.
	// 3rd value was 10
	glTranslatef(0.0f, 0.0f, -5.0f);

	// Setup lighting!
	updateLighting();

	// We love spheres. So round. So classy.
	drawNavBall(navBallTexture, 2);
	drawAirSpeedStrip(airSpeedTexture);
	drawAltitudeStrip(altitudeTexture);
	drawCompass(compassTexture);
	drawAirplaneRightWing();
	drawAirplaneLeftWing();
	drawSlipBall(0);
	drawAltitudeBox();
	drawAltitudeText(destination.altitude);
	drawAirspeedBox();
	drawAirspeedText(destination.airspeed);
	drawSlipBox();
	drawGlideSlopeBox();
	drawGlideSlopeBall(1.0);
	drawLocalizerScale(1.0);
	drawLocalizerBox(1.0);
	drawRoll();
	drawDecorators();
	drawCompassNeedle();
	drawCompassCover();
	//drawAltStrip(altitudeTexture);
	glDisable(GL_LIGHTING);
	glutSwapBuffers();

	// SHOW ME WHAT YOU GOT.
	// Reference:
	glFlush(); // Flush the OpenGL buffers to the window
	gettimeofday(&tv2, NULL);
	printf("Time to render image, once data recieved is: %ld microseconds\n", tv2.tv_usec-tv1.tv_usec);
}

// Reshape function. Not sure if we need this but tutorials have it.
void reshape(int width, int height) {

	// Defines a area that we will draw too.
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	// Switch to proj. Matrix so we can manipulate the scene.
	glMatrixMode(GL_PROJECTION);
	// Clean
	glLoadIdentity();
 	// Set field of view to 60 degrees
	gluPerspective(60, (GLfloat)width/(GLfloat)height, 1.0, 100.0);
 	// Switch to model view for drawing,
	glMatrixMode(GL_MODELVIEW);
}



// Main program
// We use GLUT to negotiate for a window to do stuff in.
// We use GLU library to make complex tasks easier.
// We use openGL aka "gl" to do 'stuff'.
int main(int argc, char **argv) {
	#ifdef WIN
		WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
	 	WSADATA wsaData;                              // Stuff for WSA functions
	#endif

	#ifdef WIN
	   	// This stuff initializes winsock
	  	WSAStartup(wVersionRequested, &wsaData);
	#endif
        // >>> Step #1 <<<
	// Christensen's tools page
	// Create a client socket
	//   - AF_INET is Address Family Internet and SOCK_STREAM is streams
	client_s = socket(AF_INET, SOCK_STREAM, 0);
	if(client_s < 0) {
	    printf("*** ERROR - socket() failed \n");
	    exit(-1);
	}


	// >>> Step #2 <<<
	// Fill-in the server's address information and do a connect with the
	// listening server using the client socket - the connect() will block.
	server_addr.sin_family = AF_INET;                 // Address family to use
	server_addr.sin_port = htons(PORT_NUM);           // Port num to use
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // IP address to use
	retcode = connect(client_s, (struct sockaddr *)&server_addr,
	sizeof(server_addr));
	if(retcode < 0) {
		printf("*** ERROR - connect() failed \n");
		exit(-1);
	}

	///////////// end start client////////////

	// >>> Step #3 <<<
	// Receive from the server using the client socket
	retcode = recv(client_s, in_buf, sizeof(in_buf), 0);
	if(retcode < 0) {
		printf("*** ERROR - recv() failed \n");
		exit(-1);
	}

	// Output the received message
	printf("Received from server: %s \n", in_buf);

        // >>> Step #4 <<<
	// Send to the server using the client socket
	strcpy(out_buf, "Client is connected to server... Waiting for packets");
	retcode = send(client_s, out_buf, (strlen(out_buf) + 1), 0);
	if (retcode < 0) {
		printf("*** ERROR - send() failed \n");
		exit(-1);
	}

	printf("Hello, world!\n");

	// Define our window size and the are to draw too.
	glutInitWindowSize(400, 400);
	glViewport(0,0, 400, 400);

	// Set the initial display position in pixels.
	glutInitWindowPosition(0, 0);	// X and Y Location

	// Initiatize the GLUT library, and negotiate a session,
	//	with the window system.
	glutInit(&argc, argv);


	// Initial Display Mode: determines the display mode,
	//	for the to-be-created window.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH);	// singe buffered window. We can add more later.


	// TODO: For glutCreateWindow. remove Title Bar or go full Screen.
	// Check: glutFULLScreen.
	// Creates a top-level window. Name: Navball
	glutCreateWindow("NavBall");
//	glutFullScreen();

	// Our Display Function
	glutDisplayFunc(display);	// FCN: display up above.;
	// Set our shade model to default.
	glShadeModel(GL_SMOOTH);
	// Setup some depth stuff.
	// Good example of depth test:
	// Youtube: openGL glEnable Depth Test by Jamie King.
	glEnable(GL_DEPTH_TEST);	// Expensive but worth it!
	glDepthFunc(GL_LEQUAL);
	// Hits are like the pirate code, "More like guidelines".
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_NORMALIZE);
	// This eliminates triangles.
	// Example: We remove triangles inside the sphere. or not facing us.
	// See: Jamie King's video Backface Culling OpenGL glCullFace.
	glCullFace(GL_BACK);
	glCullFace(GL_CULL_FACE);
	// Cleanup
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// Set pixel storage modes.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Setup our lighting
	setupLighting();
	// Make all our images.
	makeAllImages();
	// Our idle function
	glutIdleFunc(display);		// FCN: display up above.

	// Our window makes a call back to the program.
	// 	Nothing happens unless we describe how we create our shape.
	glutReshapeFunc(reshape);

	// Glut main loop for event processing, like changing rotation etc.
	glutMainLoop();



	////////////////////// Close connection ///////////////////////
	#ifdef WIN
	  retcode = closesocket(client_s);
	  if (retcode < 0)
	  {
	    printf("*** ERROR - closesocket() failed \n");
	    exit(-1);
	  }
	#endif
	#ifdef BSD
	  retcode = close(client_s);
	  if (retcode < 0)
	  {
	    printf("*** ERROR - close() failed \n");
	    exit(-1);
	  }
	#endif

	#ifdef WIN
	  // Clean-up winsock
	  WSACleanup();
	#endif

	return 0;
}
