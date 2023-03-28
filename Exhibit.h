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
	static Mesh* cube;
	static Material* cobblestone;

public:
	Exhibit(float size);
	~Exhibit();
	const std::vector<GameEntity*>* GetEntities();
	void PlaceObject(GameEntity* entity, const XMFLOAT3& position);
	void AttachTo(Exhibit* other, Direction direction);
	void CheckCollisions(Camera* camera);
	bool IsInExhibit(const XMFLOAT3& position);

private:
	DirectX::XMFLOAT3 origin;
	float size;
	std::vector<GameEntity*>* surfaces; // the floor and walls

	const float THICKNESS = 1;
	const float WALL_HEIGHT = 10;
};