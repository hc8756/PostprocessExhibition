#include "Exhibit.h"

//std::vector<GameEntity*>* Exhibit::mainEntityList;
Mesh* Exhibit::cube;
Material* Exhibit::cobblestone;

// the position defaults to (0, 0, 0), use AttachTo() to place one relative to another
Exhibit::Exhibit(float size)
{
	origin.y = 0;
	this->origin = XMFLOAT3(0, 0, 0);
	this->size = size;

	surfaces = new std::vector<GameEntity*>();

	// create floor
	GameEntity* floor = new GameEntity(cube, cobblestone);
	//mainEntityList->push_back(floor); // use Game.cpp entity list so that it draws and deletes automatically
	floor->GetTransform()->SetScale(size, THICKNESS, size);
	surfaces->push_back(floor); // floor must be first in the list

	// create all 4 walls by defualt, then remove them when attached to another exhibit
	GameEntity* posXWall = new GameEntity(cube, cobblestone);
	posXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	posXWall->GetTransform()->SetPosition(origin.x + size / 2, posXWall->GetTransform()->GetScale().y / 2, origin.z);
	surfaces->push_back(posXWall);

	GameEntity* negXWall = new GameEntity(cube, cobblestone);
	negXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	negXWall->GetTransform()->SetPosition(origin.x - size / 2, negXWall->GetTransform()->GetScale().y / 2, origin.z);
	surfaces->push_back(negXWall);

	GameEntity* posZWall = new GameEntity(cube, cobblestone);	
	posZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	posZWall->GetTransform()->SetPosition(origin.x, posZWall->GetTransform()->GetScale().y / 2, origin.z + size / 2);
	surfaces->push_back(posZWall);

	GameEntity* negZWall = new GameEntity(cube, cobblestone);
	negZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	negZWall->GetTransform()->SetPosition(origin.x, negZWall->GetTransform()->GetScale().y / 2, origin.z - size / 2);
	surfaces->push_back(negZWall);
}

Exhibit::~Exhibit()
{
	for (auto& surface : *surfaces) {
		delete surface;
		surface = nullptr;
	}
}

// allows Game.cpp to draw the entities here
const std::vector<GameEntity*>* Exhibit::GetEntities()
{
	return surfaces;
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
	XMFLOAT3 oldOrigin = origin;
	origin = XMFLOAT3(other->origin.x + shiftDir.x * scale, 0, other->origin.z + shiftDir.y * scale);

	// shift walls and floors
	XMFLOAT3 shift = XMFLOAT3(origin.x - oldOrigin.x, 0, origin.z - oldOrigin.z);
	for (GameEntity* surface : *surfaces) {
		surface->GetTransform()->MoveAbsolute(shift.x, 0, shift.z);
	}

	// replace full walls with doorway

}

void Exhibit::CheckCollisions(Camera* camera)
{
	if (surfaces->size() < 1) {
		return;
	}

	XMFLOAT3 camPos = camera->GetTransform()->GetPosition();
	/*if (posXWall != nullptr && IsInWall(camPos, posXWall)) {
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
	}*/

	int extraWidth = 1; // added distance from the wall for the collision
	for (int i = 1; i < surfaces->size(); i++) { // skip the first element because that is the floor
		GameEntity* wall = (*surfaces)[i];

		// check if inside the wall or not
		XMFLOAT3 wallPos = wall->GetTransform()->GetPosition();
		XMFLOAT3 wallScale = wall->GetTransform()->GetScale();

		if (camPos.x < wallPos.x - wallScale.x / 2 - extraWidth
			|| camPos.x > wallPos.x + wallScale.x / 2 + extraWidth
			|| camPos.z < wallPos.z - wallScale.z / 2 - extraWidth
			|| camPos.z > wallPos.z + wallScale.z / 2 + extraWidth
		) {
			continue; // outside the wall
		}

		// shift away from wall. Assumes camera moves slow enough to never actually make it inside the wall
		if (camPos.x < wallPos.x - wallScale.x / 2) {
			camera->GetTransform()->SetPosition(wallPos.x - wallScale.x / 2 - extraWidth, camPos.y, camPos.z);
		}
		else if (camPos.x > wallPos.x + wallScale.x / 2) {
			camera->GetTransform()->SetPosition(wallPos.x + wallScale.x / 2 + extraWidth, camPos.y, camPos.z);
		}
		else if (camPos.z < wallPos.z - wallScale.z / 2) {
			camera->GetTransform()->SetPosition(camPos.x, camPos.y, wallPos.z - wallScale.z / 2 - extraWidth);
		}
		else if (camPos.z > wallPos.z + wallScale.z / 2) {
			camera->GetTransform()->SetPosition(camPos.x, camPos.y, wallPos.z + wallScale.z / 2 + extraWidth);
		}
	}
}

bool Exhibit::IsInExhibit(const XMFLOAT3& position)
{
	return position.x > origin.x - size / 2
		&& position.x < origin.x + size / 2
		&& position.z > origin.z - size / 2
		&& position.z < origin.z + size / 2;
}
