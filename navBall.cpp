#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/freeglut.h> // Include the GLUT header file

// Sources: TODO add sources.

// Compile and link with gcc t1.c -o t1 -lGL -lGLU -lglut

// Display FCN
void display(void) {
	// TODO move stuff to a initGL FCN. 
	//	For example a cleanStart FCN.
	
	// Logic for rotating sphere in here.
	
	// This specifies "cleared" values for the buffers.
	// Note this only allows values of 0 to 1.
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Red, Green, Blue, alpha.
	
	// Actually clear the buffer.	
	glClear(GL_COLOR_BUFFER_BIT); 
	
	// www.opengl-tutorial.org/beginners-tutorial/tutorial-3-matrix
	// 	Everything is matrix-cis man.
	// 	Basically creates a blank template to work on.
	// 	Loads the identity matrix.
	glLoadIdentity(); 
 
	// Push stuff back so we can see it 200 units.
	glTranslatef(0.0f, 0.0f, -5.0f); 
 
	// We love spheres. So round. So classy.
	glutSolidSphere(1.0, 20, 50);
 
	// SHOW ME WHAT YOU GOT.
	// Reference: 
	glFlush(); // Flush the OpenGL buffers to the window
	
	// Logic for rotating sphere goes here.
	
}

// Some magic happens here.
void reshape(int width, int height) {
	
	// Defines a area that we will draw too.
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); // Set our viewport to the size of our window
	
	
	glMatrixMode(GL_PROJECTION); // Switch to the projection matrix so that we can manipulate how our scene is viewed
	glLoadIdentity(); // Reset the projection matrix to the identity matrix so that we don't get any artifacts (cleaning up)
 
	gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 100.0); // Set the Field of view angle (in degrees), the aspect ratio of our window, and the new and far planes
 
	glMatrixMode(GL_MODELVIEW); // Switch back to the model view matrix, so that we can start drawing shapes correctly
	
}
 

// Main program
int main(int argc, char **argv) {
	
	printf("Hello, world!\n");
	
	// Define our window size:
	glutInitWindowSize(500, 500);
	
		
	// Set the initial display position in pixels.
	glutInitWindowPosition(100, 100);	// X and Y Location
	
	// Initiatize the GLUT library, and negotiate a session,
	//	with the window system.
	glutInit(&argc, argv);
	
	// Initial Display Mode: determines the display mode,
	//	for the to-be-created window.
	glutInitDisplayMode(GLUT_SINGLE);	// singe buffered window. We can add more later.

	
	// TODO: For glutCreateWindow. remove Title Bar or go full Screen.
	// Check: glutFULLScreen.
	// Creates a top-level window. Name: Navball
	glutCreateWindow("NavBall");
	
	// Our Display Function
	glutDisplayFunc(display);	// FCN: display up above.
	
	// Our idle function
	glutIdleFunc(display);		// FCN: display up above.
	
	// Our window makes a call back to the program. 
	// 	Nothing happens unless we describe how we create our shape.
	glutReshapeFunc(reshape); 
	
	// Glut main loop for event processing, like changing rotation etc.
	glutMainLoop();
	
	
	
	return 0;
}
