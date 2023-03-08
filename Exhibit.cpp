#include "Exhibit.h"

std::vector<GameEntity*>* Exhibit::mainEntityList;
Mesh* Exhibit::cube;
Material* Exhibit::cobblestone;

// the position defaults to (0, 0, 0), use AttachTo() to place on relative to another
Exhibit::Exhibit(float size)
{
	origin.y = 0;
	this->origin = XMFLOAT3(0, 0, 0);
	this->size = size;

	// create floor
	floor = new GameEntity(cube, cobblestone);
	mainEntityList->push_back(floor); // use Game.cpp entity list so that it draws and deletes automatically
	floor->GetTransform()->SetScale(size, THICKNESS, size);

	// create all 4 walls by defualt, then remove them when attached to another exhibit
	this->posXWall = new GameEntity(cube, cobblestone);
	mainEntityList->push_back(this->posXWall);
	this->posXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	this->negXWall = new GameEntity(cube, cobblestone);
	mainEntityList->push_back(this->negXWall);
	this->negXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	this->posZWall = new GameEntity(cube, cobblestone);
	mainEntityList->push_back(this->posZWall);
	this->posZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	this->negZWall = new GameEntity(cube, cobblestone);
	mainEntityList->push_back(this->negZWall);
	this->negZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);

	PlaceStructures();
}

// places the entity relative to the origin of this room for convenience
void Exhibit::PlaceObject(GameEntity* entity, const DirectX::XMFLOAT3& position)
{
	entity->GetTransform()->SetPosition(origin.x + position.x, origin.y + position.y, origin.z + position.z);
}

// places this exhibit up against another one in the desired direction. This does not move objects already within the exhibit
void Exhibit::AttachTo(Exhibit* other, Direction direction)
{
	XMFLOAT2 shiftDir = XMFLOAT2();
	switch (direction) {
		case POSX:
			shiftDir = XMFLOAT2(1, 0);
			//mainEntityList.remove();
			negXWall = nullptr;
			other->posXWall = nullptr;
			// need to delete wall
			break;
		case NEGX:
			shiftDir = XMFLOAT2(-1, 0);
			break;
		case POSZ:
			shiftDir = XMFLOAT2(0, 1);
			break;
		case NEGZ:
			shiftDir = XMFLOAT2(0, -1);
			break;
	}

	float scale = (size + other->size) / 2;
	origin = XMFLOAT3(other->origin.x + shiftDir.x * scale, 0, other->origin.z + shiftDir.y * scale);
	PlaceStructures();
}

void Exhibit::CheckCollisions(Camera* camera)
{
	float buffer = 1.0f; // should match value in IsInWall()
	XMFLOAT3 camPos = camera->GetTransform()->GetPosition();
	if (posXWall != nullptr && IsInWall(camPos, posXWall)) {
		camera->GetTransform()->SetPosition(posXWall->GetTransform()->GetPosition().x - posXWall->GetTransform()->GetScale().x / 2 - buffer, camPos.y, camPos.z);
	}
	if (negXWall != nullptr && IsInWall(camPos, negXWall)) {
		camera->GetTransform()->SetPosition(negXWall->GetTransform()->GetPosition().x + negXWall->GetTransform()->GetScale().x / 2 + buffer, camPos.y, camPos.z);
	}
	if (posZWall != nullptr && IsInWall(camPos, posZWall)) {
		camera->GetTransform()->SetPosition(camPos.x, camPos.y, posZWall->GetTransform()->GetPosition().z - posZWall->GetTransform()->GetScale().z / 2 - buffer);
	}
	if (negZWall != nullptr && IsInWall(camPos, negZWall)) {
		camera->GetTransform()->SetPosition(camPos.x, camPos.y, negZWall->GetTransform()->GetPosition().z + negZWall->GetTransform()->GetScale().z / 2 + buffer);
	}
}

bool Exhibit::IsInExhibit(const XMFLOAT3& position)
{
	return position.x > origin.x - size / 2
		&& position.x < origin.x + size / 2
		&& position.z > origin.z - size / 2
		&& position.z < origin.z + size / 2;
}

// moves all floors and walls to the correct
void Exhibit::PlaceStructures()
{
	floor->GetTransform()->SetPosition(origin.x, -floor->GetTransform()->GetScale().y / 2, origin.z); // top is at y = 0
	if(posXWall != nullptr)
		posXWall->GetTransform()->SetPosition(origin.x + size / 2, posXWall->GetTransform()->GetScale().y / 2, origin.z);
	if(negXWall != nullptr)
		negXWall->GetTransform()->SetPosition(origin.x - size / 2, negXWall->GetTransform()->GetScale().y / 2, origin.z);
	if(posZWall != nullptr)
		posZWall->GetTransform()->SetPosition(origin.x, posZWall->GetTransform()->GetScale().y / 2, origin.z + size / 2);
	if(negZWall != nullptr)
		negZWall->GetTransform()->SetPosition(origin.x, negZWall->GetTransform()->GetScale().y / 2, origin.z - size / 2);
}

// ignores wall height
bool Exhibit::IsInWall(XMFLOAT3 position, GameEntity* wall)
{
	float buffer = 1.0f;
	XMFLOAT3 wallPos = wall->GetTransform()->GetPosition();
	XMFLOAT3 wallScale = wall->GetTransform()->GetScale();
	return position.x > wallPos.x - wallScale.x / 2 - buffer
		&& position.x < wallPos.x + wallScale.x / 2 + buffer
		&& position.z > wallPos.z - wallScale.z / 2 - buffer
		&& position.z < wallPos.z + wallScale.z / 2 + buffer;
}
