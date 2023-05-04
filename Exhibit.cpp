#include "Exhibit.h"

//std::vector<GameEntity*>* Exhibit::mainEntityList;
Mesh* Exhibit::cube;
Material* Exhibit::cobblestone;
Material* Exhibit::marble;

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
	GameEntity* posXWall = new GameEntity(cube, marble);
	posXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	posXWall->GetTransform()->SetPosition(origin.x + size / 2, posXWall->GetTransform()->GetScale().y / 2, origin.z);
	surfaces->push_back(posXWall);

	GameEntity* negXWall = new GameEntity(cube, marble);
	negXWall->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, size);
	negXWall->GetTransform()->SetPosition(origin.x - size / 2, negXWall->GetTransform()->GetScale().y / 2, origin.z);
	surfaces->push_back(negXWall);

	GameEntity* posZWall = new GameEntity(cube, marble);
	posZWall->GetTransform()->SetScale(size, WALL_HEIGHT, THICKNESS);
	posZWall->GetTransform()->SetPosition(origin.x, posZWall->GetTransform()->GetScale().y / 2, origin.z + size / 2);
	surfaces->push_back(posZWall);

	GameEntity* negZWall = new GameEntity(cube, marble);
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

// places this exhibit up against another one in the desired direction. This does not move objects already within the exhibit and can only be used once per exhibit
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

	// create doorway
	XMFLOAT3 targetMid = XMFLOAT3(other->origin.x + shiftDir.x * other->size / 2, 0, other->origin.z + shiftDir.y * other->size / 2); // ignore y
	GameEntity* wall1 = nullptr;
	for (GameEntity* wall : *surfaces) {
		XMFLOAT3 wallPos = wall->GetTransform()->GetPosition();
		if (wallPos.x == targetMid.x && wallPos.z == targetMid.z) {
			wall1 = wall;
			break;
		}
	}
	
	targetMid = XMFLOAT3(origin.x - shiftDir.x * size / 2, 0, origin.z - shiftDir.y * size / 2); // ignore y
	GameEntity* wall2 = nullptr;
	for (GameEntity* wall : *(other->surfaces)) {
		XMFLOAT3 wallPos = wall->GetTransform()->GetPosition();
		if (wallPos.x == targetMid.x && wallPos.z == targetMid.z) {
			wall2 = wall;
			break;
		}
	}

	if (wall1 == nullptr || wall2 == nullptr) {
		return;
	}

	float DOOR_GAP = 7.0f;
	float greaterLength = (size > other->size ? size : other->size);
	float newWidth = (greaterLength - DOOR_GAP) / 2;

	if (direction == POSX || direction == NEGX) {
		wall1->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, newWidth);
		wall2->GetTransform()->SetScale(THICKNESS, WALL_HEIGHT, newWidth);
	} else {
		wall1->GetTransform()->SetScale(newWidth, WALL_HEIGHT, THICKNESS);
		wall2->GetTransform()->SetScale(newWidth, WALL_HEIGHT, THICKNESS);
	}

	float wallShift = (newWidth + DOOR_GAP) / 2;
	wall1->GetTransform()->MoveAbsolute(shiftDir.y * wallShift, 0, shiftDir.x * wallShift);
	wall2->GetTransform()->MoveAbsolute(shiftDir.y * -wallShift, 0, shiftDir.x * -wallShift);
}

void Exhibit::CheckCollisions(Camera* camera)
{
	if (surfaces->size() < 1) {
		return;
	}

	XMFLOAT3 camPos = camera->GetTransform()->GetPosition();

	int extraWidth = 1; // added distance from the wall for the collision
	for (int i = 1; i < surfaces->size(); i++) { // skip the first element because that is the floor
		GameEntity* wall = (*surfaces)[i];

		// check if inside the wall or not
		XMFLOAT3 wallPos = wall->GetTransform()->GetPosition();
		XMFLOAT3 wallScale = wall->GetTransform()->GetScale();

		if (camPos.x <= wallPos.x - wallScale.x / 2 - extraWidth
			|| camPos.x >= wallPos.x + wallScale.x / 2 + extraWidth
			|| camPos.z <= wallPos.z - wallScale.z / 2 - extraWidth
			|| camPos.z >= wallPos.z + wallScale.z / 2 + extraWidth
		) {
			continue; // outside the wall
		}

		// shift away from wall. Assumes camera moves slow enough to never actually make it inside the wall
		XMFLOAT3 newPos = camera->GetTransform()->GetPosition();
		if (camPos.x < wallPos.x - wallScale.x / 2) {
			newPos.x = wallPos.x - wallScale.x / 2 - extraWidth;
		}
		else if (camPos.x > wallPos.x + wallScale.x / 2) {
			newPos.x = wallPos.x + wallScale.x / 2 + extraWidth;
		}
		if (camPos.z < wallPos.z - wallScale.z / 2) {
			newPos.z = wallPos.z - wallScale.z / 2 - extraWidth;
		}
		else if (camPos.z > wallPos.z + wallScale.z / 2) {
			newPos.z = wallPos.z + wallScale.z / 2 + extraWidth;
		}
		camera->GetTransform()->SetPosition(newPos.x, newPos.y, newPos.z);
	}
}

bool Exhibit::IsInExhibit(const XMFLOAT3& position)
{
	return position.x > origin.x - size / 2
		&& position.x < origin.x + size / 2
		&& position.z > origin.z - size / 2
		&& position.z < origin.z + size / 2;
}
