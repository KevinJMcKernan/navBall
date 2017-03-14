// Main Sources:
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
#include <GL/freeglut.h>  // Include the GLUT header file
#include "RGBpixmap.h"

// Graphics Library Unsigned Binary Integer
GLuint navBallTexture;

// BMP image file. You can make one in MS paint. 
const char navBallFileName[] = "Bitmaps/t2.bmp";

// Constants for our lighting and image. Edit these to change 
// Lighting.
// Resource: www.glprogramming.com/red/chapter05.html
// See example 5.2

// This specifies light position. 
// First value raises light.
// 
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

// General Variable for manipulating our Ball.
GLfloat theta[] = {0.0, 0.0, 0.0};	// Used in our display function.

// Setup light source named GL_LIGHT0
void setupLighting() {
	glLightfv(GL_LIGHT0, GL_AMBIENT, LIGHT_AMBIENT);	// Set ambient lighting.
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LIGHT_DIFFUSE);	// Set diffuse lighting.
	glLightfv(GL_LIGHT0, GL_SPECULAR, LIGHT_SPECULAR);	// Set specular lighting.
	glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_0_POSITION);	// Set the light source position.
}

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
	// This controls rotation.
	glRotatef(theta[0], 0.0, 1.0, 0.0);
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
        makeImage(navBallFileName, navBallTexture, false);
        // Add other images you want to make below.

        return;
}




// Display FCN
void display(void) {
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
	// Load matrix modes.
	glMatrixMode(GL_MODELVIEW);
	// Clean
	glLoadIdentity();	
	
	// TODO move stuff to a initGL FCN. 
	//	For example a cleanStart FCN.
	
	// Logic for rotating sphere in here.
	theta[0] += 1.0;
	if(theta[0] > 360.0) {
		theta[0] = 0.0;
	}
	
	// Push stuff back so we can see it 2 units.
	glTranslatef(0.0f, 0.0f, -5.0f); 

	// Setup lighting!
	updateLighting();
 
	// We love spheres. So round. So classy.
	drawNavBall(navBallTexture, 2);
	glDisable(GL_LIGHTING);
	glutSwapBuffers();
 
	// SHOW ME WHAT YOU GOT.
	// Reference: 
	glFlush(); // Flush the OpenGL buffers to the window
	
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
	
	printf("Hello, world!\n");
	
	// Define our window size and the are to draw too.
	glutInitWindowSize(500, 500);
	glViewport(0,0, 500, 500);	
			
	// Set the initial display position in pixels.
	glutInitWindowPosition(100, 100);	// X and Y Location
	
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
		
	return 0;
}
