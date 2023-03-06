#include "Camera.h"
using namespace DirectX;

Camera::Camera(float x, float y, float z, float moveSpeed, float lookSpeed, float fov, float aspectRatio) :
	movementSpeed(moveSpeed),
	mouseLookSpeed(lookSpeed),
	fieldOfView(fov),
	aspectRatio(aspectRatio)
{
	// setup transform
	transform.SetPosition(x, y, z);
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
	Input::GetInstance().CenterMouse(); // start with the mouse in the center so there is no jump at the beginning
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
{
	// Get a reference to the input manager
	Input& input = Input::GetInstance();

	//Calculate current speed
	float speed = movementSpeed * dt;

	// Movement
	if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
	if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
	if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0, 0); }
	if (input.KeyDown('D')) { transform.MoveRelative(speed, 0, 0); }
	if (input.KeyDown(VK_SHIFT)) { transform.MoveAbsolute(0, -speed, 0); }
	if (input.KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, speed, 0); }

	// Limit vertical values
	if (transform.GetPosition().y < 1) {
		transform.SetPosition(transform.GetPosition().x, 1, transform.GetPosition().z);
	}
	else if (transform.GetPosition().y > 12) {
		transform.SetPosition(transform.GetPosition().x, 12, transform.GetPosition().z);
	}

	// Calculate how much the cursor changed
	float xDiff = dt * mouseLookSpeed * input.GetMouseXDelta();
	float yDiff = dt * mouseLookSpeed * input.GetMouseYDelta();

	// Roate the transform! SWAP X AND Y!
	transform.Rotate(yDiff, xDiff, 0);

	// limit vertical to straight up and down
	if (transform.GetPitchYawRoll().x > XM_PIDIV2 - 0.1f) {
		transform.SetRotation(XM_PIDIV2 - 0.1f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
	}
	else if (transform.GetPitchYawRoll().x < -XM_PIDIV2 + 0.1f) {
		transform.SetRotation(-XM_PIDIV2 + 0.1f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
	}

	// At the end, update the view
	UpdateViewMatrix();

	input.CenterMouse();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 fwd = transform.GetForward();

	XMMATRIX V = XMMatrixLookToLH(
		XMLoadFloat3(&pos),			// Camera's position
		XMLoadFloat3(&fwd),			// Camera's forward vector
		XMVectorSet(0, 1, 0, 0));   // World up (Y)

	XMStoreFloat4x4(&viewMatrix, V);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	this->aspectRatio = aspectRatio;
	XMMATRIX P = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, 0.01f, 100.0f);
	XMStoreFloat4x4(&projectionMatrix, P);
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projectionMatrix;
}

Transform* Camera::GetTransform()
{
	return &transform;
}

float Camera::GetFoV()
{
	return fieldOfView;
}

void Camera::SetFoV(float fov)
{
	fieldOfView = fov;
	UpdateProjectionMatrix(aspectRatio);
}
