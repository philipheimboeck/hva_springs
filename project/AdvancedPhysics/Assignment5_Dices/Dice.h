#pragma once

#include <cyclone\collide_fine.h>
#include <cyclone\joints.h>

class Dice
{
public:
	Dice(cyclone::real halfsize, cyclone::real x, cyclone::real y, cyclone::real z);
	~Dice(void);
	void integrate(cyclone::real duration);
	void render();
	void createContacts(cyclone::CollisionData *data);
	void createContactsDice(Dice &dice, cyclone::CollisionData *data);
	void createContactsPlane(cyclone::CollisionPlane &plane, cyclone::CollisionData *data);
	void resetPosition(cyclone::real halfsize, cyclone::real x, cyclone::real y, cyclone::real z);
	void recalculate();
private:

	cyclone::CollisionSphere vertices[8];
	cyclone::Joint joints[12];
};

