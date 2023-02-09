#pragma once
#include <DirectXMath.h>
class Transform
{
public:
    Transform();
    ~Transform();

    // Getters
    DirectX::XMFLOAT3 GetPosition();
    DirectX::XMFLOAT3 GetPitchYawRoll();
    DirectX::XMFLOAT3 GetScale();
    DirectX::XMFLOAT4X4 GetWorldMatrix();
    DirectX::XMFLOAT4X4 GetWorldInverseTranspose();
    DirectX::XMFLOAT3 GetUp();
    DirectX::XMFLOAT3 GetRight();
    DirectX::XMFLOAT3 GetForward();


    // Setters
    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw, float roll);
    void SetScale(float x, float y, float z);

    // Transformers
    void MoveAbsolute(float x, float y, float z);    
    void MoveRelative(float x, float y, float z);
    void Rotate(float pitch, float yaw, float roll);
    void Scale(float x, float y, float z);

private:
    // Raw transformation data
    DirectX::XMFLOAT3 position;    // default 0,0,0
    DirectX::XMFLOAT3 pitchYawRoll;    // default 0,0,0
    DirectX::XMFLOAT3 scale;    // default 1,1,1

    // Matrices
    bool matricesDirty;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

    // Getter for matrices
    // Creates separate translation, rotation, scale matricies
    // Combines them to create and store a new matrix
    void UpdateMatrices();
};
