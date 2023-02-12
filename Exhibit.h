#pragma once
#include <DirectXMath.h>
#include "GameEntity.h"

// a square room that with entities inside of it
class Exhibit
{
public:
	Exhibit(DirectX::XMFLOAT3 origin, float size, bool posXWall, bool negXWall, bool posZWall, bool negZWall);
	~Exhibit();
	void PlaceObject(GameEntity* entity, const DirectX::XMFLOAT3& position);
	void Attach(Exhibit* other, const DirectX::XMFLOAT3& direction);

private:
	DirectX::XMFLOAT3 origin;
	float size;
	// list of walls/floor	
};