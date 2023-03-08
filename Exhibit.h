#pragma once
#include <DirectXMath.h>
#include "GameEntity.h"

enum Direction {
	POSX,
	NEGX,
	POSZ,
	NEGZ
};

// a square room that with entities inside of it
class Exhibit
{
public:
	static std::vector<GameEntity*>* mainEntityList; // pointer the main entity list in Game.ccp so that walls are cleaned up automatically
	static Mesh* cube;
	static Material* cobblestone;

public:
	Exhibit(float size);
	//~Exhibit();
	void PlaceObject(GameEntity* entity, const XMFLOAT3& position);
	void AttachTo(Exhibit* other, Direction direction);
	void CheckCollisions(Camera* camera);
	bool IsInExhibit(const XMFLOAT3& position);

private:
	DirectX::XMFLOAT3 origin;
	float size;

	// these game objects are owned by Game.cpp
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