#pragma once
#include <DirectXMath.h>
#include "GameEntity.h"

// a square room that with entities inside of it
class Exhibit
{
public: // components for creating floors and walls
	static GameEntity* structureTemplate; // copied to make floors and walls
	static void SetStructureTemplate(GameEntity* structureTemplate);

public:
	Exhibit(DirectX::XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall);
	~Exhibit();
	void PlaceObject(GameEntity* entity, const DirectX::XMFLOAT3& position);
	void AttachTo(Exhibit* other, const DirectX::XMFLOAT3& direction);

private:
	DirectX::XMFLOAT3 origin;
	float size;
	GameEntity* floor;
	GameEntity* posXWall;
	GameEntity* negXWall;
	GameEntity* posZWall;
	GameEntity* negZWall;

	void PlaceStructures();
};