#include "Exhibit.h"

void Exhibit::SetStructureTemplate(GameEntity* structureTemplate)
{
	//Exhibit::structureTemplate = structureTemplate;
}

// origin: the top-middle of the floor, size: the width and height of the square
Exhibit::Exhibit(DirectX::XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall)
{
	origin.y = 0;
	this->origin = origin;
	this->size = size;

	// create floor
	//floor = new GameEntity(cube, surface);

	//// create walls
	//if (posXWall) {
	//	this->posXWall = new GameEntity(cube, surface);
	//}
	//if (negXWall) {
	//	this->negXWall = new GameEntity(cube, surface);
	//}
	//if (posZWall) {
	//	this->posZWall = new GameEntity(cube, surface);
	//}
	//if (negZWall) {
	//	this->negZWall = new GameEntity(cube, surface);
	//}

	PlaceStructures();
}

Exhibit::~Exhibit()
{
	delete floor;
	delete posXWall;
	delete negXWall;
	delete posZWall;
	delete negZWall;
}

// places the entity relative to the origin of this room for convenience
void Exhibit::PlaceObject(GameEntity* entity, const DirectX::XMFLOAT3& position)
{
	entity->GetTransform()->SetPosition(origin.x + position.x, origin.y + position.y, origin.z + position.z);
}

// places this exhibit up against another one in the desired direction. This does not move objects already within the exhibit
// the direction should be a unit vector, and most likely should have 0 as the y value
void Exhibit::AttachTo(Exhibit* other, const DirectX::XMFLOAT3& directionFromOther)
{
	float scale = (size + other->size) / 2;
	origin = DirectX::XMFLOAT3(other->origin.x + directionFromOther.x * scale, other->origin.y + directionFromOther.y * scale, other->origin.z + directionFromOther.z * scale);
	PlaceStructures();
}

// moves all floors and walls to the origin field
void Exhibit::PlaceStructures()
{

}
