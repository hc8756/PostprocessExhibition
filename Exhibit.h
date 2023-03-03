#pragma once
#include <DirectXMath.h>
#include "GameEntity.h"

// a square room that with entities inside of it
class Exhibit
{
public:
	Exhibit(std::vector<GameEntity*>& entityList,  Mesh* cube, Material* surface, XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall);
	//~Exhibit();
	void PlaceObject(GameEntity* entity, const XMFLOAT3& position);
	void AttachTo(Exhibit* other, const XMFLOAT3& direction);
	void CheckCollisions(Camera* camera);

private:
	DirectX::XMFLOAT3 origin;
	float size;
	GameEntity* floor;
	GameEntity* posXWall;
	GameEntity* negXWall;
	GameEntity* posZWall;
	GameEntity* negZWall;

	const float THICKNESS = 1;
	const float WALL_HEIGHT = 6;

	void PlaceStructures();
	bool IsInWall(XMFLOAT3 position, GameEntity* wall);
};