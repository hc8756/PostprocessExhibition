#include "Exhibit.h"

Mesh* Exhibit::cube;
Material* Exhibit::cobblestone;

// the position defaults to (0, 0, 0), use AttachTo() to place on relative to another
Exhibit::Exhibit(std::vector<GameEntity*>& entityList, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall)
{
	origin.y = 0;
	this->origin = XMFLOAT3(0, 0, 0);
	this->size = size;

	// create floor
	floor = new GameEntity(cube, cobblestone);
	entityList.push_back(floor); // use Game.cpp entity list so that it draws and deletes automatically
	floor->GetTransform()->SetScale(size, THICKNESS, size);

	// create walls
	if (posXWall) {
		this->posXWall = new GameEntity(cube, cobblestone);
		entityList.push_back(this->posXWall);
		this->posXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	}
	if (negXWall) {
		this->negXWall = new GameEntity(cube, cobblestone);
		entityList.push_back(this->negXWall);
		this->negXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	}
	if (posZWall) {
		this->posZWall = new GameEntity(cube, cobblestone);
		entityList.push_back(this->posZWall);
		this->posZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	}
	if (negZWall) {
		this->negZWall = new GameEntity(cube, cobblestone);
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
