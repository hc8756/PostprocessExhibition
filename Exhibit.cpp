#include "Exhibit.h"

// origin: the top-middle of the floor, size: the width and height
Exhibit::Exhibit(DirectX::XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall)
{
	this->origin = origin;
	this->size = size;

	// create floor

	// create walls
	if (posXWall) {

	}
	if (negXWall) {

	}
	if (posZWall) {

	}
	if (negZWall) {

	}
}

Exhibit::~Exhibit()
{
}

// places the entity relative to the origin of this room for convenience
void Exhibit::PlaceObject(GameEntity* entity, const DirectX::XMFLOAT3& position)
{
	entity->GetTransform()->SetPosition(origin.x + position.x, origin.y + position.y, origin.z + position.z);
}

// places another exhibit up against this one in the desired direction. This does not move objects already within the exhibit
// the direction should be a unit vector, and most likely should have 0 as the y value
void Exhibit::Attach(Exhibit* other, const DirectX::XMFLOAT3& direction)
{
	float scale = (size + other->size) / 2;
	other->origin = DirectX::XMFLOAT3(origin.x + direction.x * scale, origin.y + direction.y * scale, origin.z + direction.z * scale);
	// move walls and floors
}
