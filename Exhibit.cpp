#include "Exhibit.h"

// origin: the top-middle of the floor, size: the width and height of the square
Exhibit::Exhibit(std::vector<GameEntity*>& entityList, Mesh* cube, Material* surface, DirectX::XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall)
{
	origin.y = 0;
	this->origin = origin;
	this->size = size;

	// create floor
	floor = new GameEntity(cube, surface);
	entityList.push_back(floor); // use Game.cpp entity list so that it draws and deletes automatically
	floor->GetTransform()->SetScale(size, THICKNESS, size);

	// create walls
	if (posXWall) {
		this->posXWall = new GameEntity(cube, surface);
		entityList.push_back(this->posXWall);
		this->posXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	}
	if (negXWall) {
		this->negXWall = new GameEntity(cube, surface);
		entityList.push_back(this->negXWall);
		this->negXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	}
	if (posZWall) {
		this->posZWall = new GameEntity(cube, surface);
		entityList.push_back(this->posZWall);
		this->posZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	}
	if (negZWall) {
		this->negZWall = new GameEntity(cube, surface);
		entityList.push_back(this->negZWall);
		this->negZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	}

	PlaceStructures();
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

// moves all floors and walls to the correct
void Exhibit::PlaceStructures()
{
	//floor->GetTransform()->SetPosition(origin.x, -floor->GetTransform()->GetScale().y / 2, origin.z); // top is at y = 0
	//posXWall->GetTransform()->SetPosition(origin.x + size, floor->GetTransform()->GetScale().y / 2, origin.z);
	//negXWall->GetTransform()->SetPosition(origin.x - size, floor->GetTransform()->GetScale().y / 2, origin.z);
	//posZWall->GetTransform()->SetPosition(origin.x, floor->GetTransform()->GetScale().y / 2, origin.z + size);
	//negZWall->GetTransform()->SetPosition(origin.x, floor->GetTransform()->GetScale().y / 2, origin.z - size);
}
