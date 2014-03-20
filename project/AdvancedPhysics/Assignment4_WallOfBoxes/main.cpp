/*
* Assignment 4 - Wall of Boxes
* by Mathis Florian (StdNo: 500702331) and Heimb�ck Philip (StdNo: 500702328)
*
*/

#include <cyclone\core.h>
#include <gl\glut.h>
#include <cyclone\pworld.h>
#include <cyclone\pfgen.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdlib.h>
#include <time.h>

#include "glutBasic.h"
#include "Box.h"

#define WIDTH	1028
#define HEIGHT	640
#define NUMBEROFBOXES_WIDTH 4
#define NUMBEROFBOXES_HEIGHT 4
#define BOXSIZE 3
#define MAXCONTACTS 256
#define MINMASS 1
#define MAXMASS 8


void display();
void update();
void keyPress(unsigned char key, int x, int y);
void initialize();
void initializeBox(Box* box, cyclone::Vector3 position, cyclone::real mass);
void reset();
void launchBox(void);
void setMissileBox(bool sameMass);

Box boxes[NUMBEROFBOXES_WIDTH][NUMBEROFBOXES_HEIGHT];
Box *boxPointers[NUMBEROFBOXES_WIDTH*NUMBEROFBOXES_HEIGHT];
Box missileBox;

cyclone::ContactResolver resolver = cyclone::ContactResolver(MAXCONTACTS * 8);
cyclone::Contact contacts[MAXCONTACTS];
cyclone::CollisionData cData;

cyclone::Vector3 velocity;

time_t launchTime; // Used for reseting the missile after an short period of time

/*
* Application entry point
* This method will initialize the basic settings 
* of our physic application and start the cyclone engine.
*/
int main(int argc, char** argv) {
	// Init glut
	glutInit(&argc, argv);

	// Init application physics
	initialize();

	// Initialize random
	std::srand(time(NULL));

	// Create a window
	createWindow("Wall of Boxes", WIDTH, HEIGHT);

	// Initialize graphics
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

void display() {
	const static GLfloat lightPosition[] = {-1,1,0,0};

	// Clear the scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(NUMBEROFBOXES_WIDTH*BOXSIZE + 50, 64, -20,  NUMBEROFBOXES_HEIGHT*BOXSIZE, 5.0, 50.0,  0.0, 1.0, 0.0);

	// Floor
	glColor3f(0.7, 0.8, 0.8);
	glBegin(GL_QUADS);
	glVertex3f(0,0,0); glTexCoord2f(0,0); glNormal3f(0,1,0);
    glVertex3f(300,0,0); glTexCoord2f(1,0); glNormal3f(0,1,0);
    glVertex3f(300,0,300); glTexCoord2f(1.0,1.0); glNormal3f(0,1,0);
    glVertex3f(0,0,300); glTexCoord2f(0.0,1.0); glNormal3f(0,1,0);
	glEnd();

	// Print all axes
	glColor3f(0,0,0);
	glRasterPos3f(2, 0, 0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'X');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(1000, 0, 0);
	glEnd();

	glRasterPos3f(0, 2, 0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Y');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1000, 0);
	glEnd();

	glRasterPos3f(0, 0, 2);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, 'Z');
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1000);
	glEnd();

	// Print velocity
	cyclone::Vector3 pos1 = missileBox.body->getPosition();
	cyclone::Vector3 pos2 = missileBox.body->getPosition() + velocity;
	glBegin(GL_LINES);
	glVertex3f(pos1.x, pos1.y, pos1.z);
	glVertex3f(pos2.x, pos2.y, pos2.z);
	glEnd();

	// Render all Boxes
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(1,0,0);
	missileBox.render();
	for ( int i = 0; i < NUMBEROFBOXES_WIDTH; i++ )
	{
		for ( int j = 0; j < NUMBEROFBOXES_HEIGHT; j++ )
		{
			boxes[i][j].render();
		}
	}

	// Write controls
	glColor3f(0, 0, 0);
	drawHudText("[+-] Increase or decrease mass (" + 
		std::to_string((long long)(missileBox.body->getMass())) + " g)", WIDTH, HEIGHT, 10, 20);
	drawHudText("[WASD] Aim", WIDTH, HEIGHT, 10, 40);
	drawHudText("[R] Reset", WIDTH, HEIGHT, 10, 60);
	drawHudText("[N] New Map", WIDTH, HEIGHT, 10, 80);

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// Update the displayed content.
	glFlush();
	glutSwapBuffers();
}

void update()
{
	// Reset missile?
	if ( launchTime != 0 && time(NULL) - launchTime > 3 )
	{
		setMissileBox(true);
	}

	float duration = 0.05f;

	missileBox.integrate(duration);
	missileBox.calculateInternals();
	for ( int i = 0; i < NUMBEROFBOXES_WIDTH; i++ )
	{
		for ( int j = 0; j < NUMBEROFBOXES_HEIGHT; j++ )
		{
			boxes[i][j].integrate(duration);
			boxes[i][j].calculateInternals();
		}
	}


	/** GENERATE Contacts **/
	// Create the ground plane data
    cyclone::CollisionPlane plane;
    plane.direction = cyclone::Vector3(0,1,0);
    plane.offset = 0;

	// Set up the collision data structure
    cData.reset(MAXCONTACTS);
    cData.friction = (cyclone::real)0.9;
    cData.restitution = (cyclone::real)0.1;
    cData.tolerance = (cyclone::real)0.1;

	// Check ground plane collisions
	cyclone::CollisionDetector::boxAndHalfSpace(missileBox, plane, &cData);
	for ( int i = 0; i < NUMBEROFBOXES_WIDTH; i++ )
	{
		for ( int j = 0; j < NUMBEROFBOXES_HEIGHT; j++ )
		{
			if (!cData.hasMoreContacts()) return;
			Box *box = &boxes[i][j];
			cyclone::CollisionDetector::boxAndHalfSpace(*box, plane, &cData);
		}
	}
	
	// Check each box-to-box collision
	for ( int i=0; i < NUMBEROFBOXES_WIDTH*NUMBEROFBOXES_HEIGHT - 1; i++ ) 
	{
		for ( int j=i+1; j < NUMBEROFBOXES_WIDTH*NUMBEROFBOXES_HEIGHT; j++ ) 
		{
			if (!cData.hasMoreContacts()) return;
			Box *box = boxPointers[i];
			Box *box2 = boxPointers[j];
			cyclone::CollisionDetector::boxAndBox(*box, *box2, &cData);

			// Add collission for each box to missilebox
			cyclone::CollisionDetector::boxAndBox(*box, missileBox, &cData);
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

void keyPress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r': case 'R':
		// Reset
		std::cout<<"Reset"<<std::endl;
		reset();
		break;
	case 'n': case 'N':
		// New
		std::cout<<"Initialize"<<std::endl;
		initialize();
		break;
	case ' ':
		// Launch box
		launchBox();
		break;
	case '+':
		missileBox.setMass(min(missileBox.getMass() + 1, MAXMASS));
		break;
	case '-':
		missileBox.setMass(max(missileBox.getMass() - 1, MINMASS));
		break;
	case 'w':
		velocity.y++;
		break;
	case 's':
		velocity.y--;
		break;
	case 'a':
		velocity.x++;
		break;
	case 'd':
		velocity.x--;
		break;
	default:
		std::cout<<(int)key<<std::endl;
	}
}

void setMissileBox(bool sameMass)
{
	// Initialize missile box
	missileBox.setColor(cyclone::Vector3(1, 0, 0));
	cyclone::real mass = ( sameMass ? missileBox.getMass() : (MAXMASS + MINMASS)/2);
	initializeBox(&missileBox, cyclone::Vector3(NUMBEROFBOXES_WIDTH/2*BOXSIZE, BOXSIZE, 10.0f), mass);
	// Reset launch time
	launchTime = 0;
}

void initialize()
{
	// Initialize missile box
	setMissileBox(false);

	// Initialize velocity vector
	velocity = cyclone::Vector3(0.0f, 5.0f, 40.0f);
	
	// Initialize wall
	for ( int i = 0; i < NUMBEROFBOXES_WIDTH; i++ )
	{
		for ( int j = 0; j < NUMBEROFBOXES_HEIGHT; j++ ) 
		{
			cyclone::Vector3 position(i * 2 * (BOXSIZE + 0.2f), 
				j * 2 * (BOXSIZE + 0.2f) + 3.f, 50.0f);

			initializeBox(&boxes[i][j], position, rand() % (MAXMASS-MINMASS)+1 + MINMASS);
			boxPointers[(i*NUMBEROFBOXES_HEIGHT+j)] = &(boxes[i][j]);
		}
	}

	cData.contactArray = contacts;
}

void reset()
{
	// Reset missile box
	setMissileBox(true);
	
	// Initialize wall
	for ( int i = 0; i < NUMBEROFBOXES_WIDTH; i++ )
	{
		for ( int j = 0; j < NUMBEROFBOXES_HEIGHT; j++ ) 
		{
			cyclone::Vector3 position(i * 2 * (BOXSIZE + 0.2f), 
				j * 2 * (BOXSIZE + 0.2f) + 3.f, 50.0f);

			initializeBox(&boxes[i][j], position, boxes[i][j].getMass());
		}
	}

	//cData.contactArray = contacts;
}

void initializeBox(Box* box, cyclone::Vector3 position, cyclone::real mass)
{
	box->halfSize = cyclone::Vector3(BOXSIZE, BOXSIZE, BOXSIZE);
	
	box->setPosition(position);
	box->setDamping(0.95f, 0.8f);
	box->setOrientation(1, 0, 0, 0);
	box->setMass(mass);
	box->body->setVelocity(0.0f, 0.0f, 0.0f);

	box->calculateInertia();

    box->body->clearAccumulators();
    box->body->setAcceleration(0,-.5f,0);

	box->body->setCanSleep(false);
	box->body->setAwake();

	box->recalculate();
}

void launchBox(void)
{
	// Don't launch missile box a second time
	if ( launchTime == 0 ) {
		std::cout<<"Box launched"<<std::endl;
		missileBox.body->setVelocity(velocity);

		// Save launch time (for reseting the missile)
		launchTime = time(NULL);
	}
}

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(), 
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}