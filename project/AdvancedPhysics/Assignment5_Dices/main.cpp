/*
* Assignment 5 - Dices
* by Mathis Florian (StdNo: 500702331) and Heimb�ck Philip (StdNo: 500702328)
*/

#include <cyclone\core.h>
#include <cyclone\pworld.h>
#include <cyclone\pfgen.h>
#include <cyclone\collide_fine.h>
#include <gl\glut.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdlib.h>
#include <time.h>

#include "glutBasic.h"
#include "Dice.h"

// Define Window Settings
#define WIDTH	1028
#define HEIGHT	640
#define MAXCONTACTS 1024
#define NUMBEROFDICES 3

// Callback function that draws everything on the screen
void display();
// Callback function for physic calculations
void update();
// Callback function that will handle the inputs
void keyPress(unsigned char key, int x, int y);
// Called when a mouse button is pressed.
void mouse(int button, int state, int x, int y);
// Called when the mouse is dragged.
void motion(int x, int y);
// Initializing the boxes (will also be called by Pressing N for New)
void initialize();
// Reset the simulation with the same masses
void reset();

// Create the Contact Resolve and contacts array
cyclone::ContactResolver resolver = cyclone::ContactResolver(MAXCONTACTS * 8);
cyclone::Contact contacts[MAXCONTACTS];
cyclone::CollisionData cData;

// Create Dice Objects
Dice* dices[NUMBEROFDICES];

// Mouse variables for camera movement
float theta = 0.0f;
float phi = 15.0f;
int last_x, last_y;

/*
* Application entry point
* This method will initialize the basic settings 
* of our physic application and start the cyclone engine.
*/
int main(int argc, char** argv) {
	// Init glut
	glutInit(&argc, argv);

	// Initialize random
	std::srand(time(NULL));

	// Init application physics
	initialize();

	// Create a window
	createWindow("Dices", WIDTH, HEIGHT);

	// Initialize graphics and set ambient lightning
	GLfloat lightAmbient[] = {0.8f,0.8f,0.8f,1.0f};
	GLfloat lightDiffuse[] = {0.9f,0.95f,1.0f,1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

	glEnable(GL_LIGHT0);
	glClearColor(0.9f, 0.95f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (double)WIDTH/(double)HEIGHT, 1.0, 500.0);
	glMatrixMode(GL_MODELVIEW);

	// Set up the handler functions for glut
	glutDisplayFunc(display);	// The display callback is executed whenever GLUT
	// determines that the window should be refreshed
	glutIdleFunc(update);		// No event
	glutKeyboardFunc(keyPress);
	glutMouseFunc(mouse);		// Mouse Handler
    glutMotionFunc(motion);		// Mouse Motion handler

	// Run the application
	// The main loop does the following things:
	// * looks at the events in the queue
	// * for each event in the queue, GLUT executes the 
	//   appropriate callback function if one is defined
	// * if no callback is defined for the event, the event is ignored
	glutMainLoop();

	// Close the application
	return 0;
}

/**
 * Display - Render Floor, Axes and Dices
 */
void display() {
	// Clear the scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt( -30.0, 15.0, -5.0, 0.0, 15.0, 0.0, 0.0, 10.0, 0.0 );
	glRotatef(-phi, 0, 0, 1);
    glRotatef(theta, 0, 1, 0);

	// Print the Floor
	glColor3f(0.7, 0.8, 0.8);
	glBegin(GL_QUADS);
	glVertex3f(0,0,0); glTexCoord2f(0,0); glNormal3f(0,1,0);
    glVertex3f(300,0,0); glTexCoord2f(1,0); glNormal3f(0,1,0);
    glVertex3f(300,0,300); glTexCoord2f(1.0,1.0); glNormal3f(0,1,0);
    glVertex3f(0,0,300); glTexCoord2f(0.0,1.0); glNormal3f(0,1,0);
	glEnd();

	// Render dices
	for ( int i = 0; i < NUMBEROFDICES; i++ ) 
	{
		dices[i]->render();
	}

	// Print all axes - X
	glColor3f(0,0,0);
	glRasterPos3f(2, 0, 0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(1000, 0, 0);
	glEnd();

	// Axis Y
	glRasterPos3f(0, 2, 0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Y');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1000, 0);
	glEnd();

	// Axis Z
	glRasterPos3f(0, 0, 2);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Z');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1000);
	glEnd();

	// Print Info for app usage
	glColor3f(0, 0, 0);
	drawHudText("Press 'R' to reset", WIDTH, HEIGHT, 10, 20);
	drawHudText("Press 'V' for verbose mode", WIDTH, HEIGHT, 10, 40);

	// Update the displayed content.
	glFlush();
	glutSwapBuffers();
}

/**
 * Update Physic
 */
void update()
{
	// integrate each dice in physic world
	float duration = 0.05f;
	for ( int i = 0; i < NUMBEROFDICES; i++ ) 
	{
		dices[i]->integrate(duration);
	}

	// Create the floor plane contact
    cyclone::CollisionPlane plane;
    plane.direction = cyclone::Vector3(0,1,0);
    plane.offset = 0;

	// Setting the collision data
    cData.reset(MAXCONTACTS);
    cData.friction = (cyclone::real)0.9;
    cData.restitution = (cyclone::real)0.1;
    cData.tolerance = (cyclone::real)0.1;

	// Check Collision for each Dice
	for ( int i = 0; i < NUMBEROFDICES; i++ ) 
	{
		// Create Contact for Dice to Plane
		dices[i]->createContactsPlane(&plane, &cData);

		// Create Contact for Dice to Dice
		for ( int j = i; j < NUMBEROFDICES; j++)
		{
			dices[i]->createContactsDice(dices[j], &cData);
		}
	}

	// Resolve detected contacts
    resolver.resolveContacts(
        cData.contactArray,
        cData.contactCount,
        duration
    );

	// Set a flag, so that the display function will be called again
	glutPostRedisplay();
}

/**
 * KeyPress Method
 * Interact to the user input 
 */
void keyPress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r': case 'R':
		// Reset
		reset();
		break;
	case 'v': case 'V':
		// Verbose: Display also the spheres of the dices
		for ( int i = 0; i < NUMBEROFDICES; i++ )
		{
			dices[i]->verbose = !dices[i]->verbose;
		}
		break;
	default:
		std::cout<<(int)key<<std::endl;
	}
}

/*
 * Initialize the Dices
 */
void initialize()
{
	// Initialize dices with
	// different positons, accelerations and rotations
	for ( int i = 0; i < NUMBEROFDICES; i++ ) {

		cyclone::real posx = rand() % 31;		// 0 to 30
		cyclone::real posy = rand() % 21 + 15;	// 15 to 35
		cyclone::real posz = rand() % 31;		// 0 to 30

		cyclone::real accelx = rand() % 4;		// 0 to 3
		cyclone::real accely = cyclone::Vector3::GRAVITY.y; // Always the gravity
		cyclone::real accelz = rand() % 4;		// 0 to 3

		cyclone::real rotatx = rand() % 7 - 3; // -3 to 3
		cyclone::real rotaty = rand() % 7 - 3;
		cyclone::real rotatz = rand() % 7 - 3;

		dices[i] = new Dice(2, posx, posy, posz);
		dices[i]->setAcceleration(accelx, accely, accelz);
		dices[i]->setRotation(rotatx, rotaty, rotatz);
	}
	
	// Set the contact array to store our box/floor contacts
	cData.contactArray = contacts;
}

/*
 * Reset the world to start values and delete dice objects
 */
void reset()
{
	for ( int i = 0; i < NUMBEROFDICES; i++ )
	{
		delete dices[i];
	}
	initialize();
}

/**
 * Mouse
 * Save last mouse position for camera motion
 */
void mouse(int button, int state, int x, int y)
{
    // Set the position
    last_x = x;
    last_y = y;
}

/**
 * Motion
 * Update camera position based on mouse motion
 */
void motion(int x, int y)
{
    // Update the camera
    theta += (x - last_x)*0.25f;
    phi += (y - last_y)*0.25f;

    // Keep it in bounds
    if (phi < -20.0f) phi = -20.0f;
    else if (phi > 80.0f) phi = 80.0f;

    // Remember the position
    last_x = x;
    last_y = y;
}