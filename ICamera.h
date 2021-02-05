#include <DirectXMath.h>
#ifndef ICAMERA_H
#define ICAMERA_H

class ICamera {
public:
	virtual ~ICamera() {}

	virtual DirectX::XMVECTOR GetPosition() const = 0;
	virtual DirectX::XMFLOAT3 GetPosition3f() const = 0;
	virtual void SetPosition(float x, float y, float z) = 0;

	// Get camera basis vectors.
	virtual DirectX::XMVECTOR GetRight()const = 0;
	virtual DirectX::XMFLOAT3 GetRight3f()const = 0;
	virtual DirectX::XMVECTOR GetUp()const = 0;
	virtual DirectX::XMFLOAT3 GetUp3f()const = 0;
	virtual DirectX::XMVECTOR GetLook()const = 0;
	virtual DirectX::XMFLOAT3 GetLook3f()const = 0;

	virtual void SetFrustum(float fovY, float aspect, float zn, float zf) = 0;

	// Define camera space via LookAt parameters.
	virtual void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp) = 0;

	virtual void Strafe(float d) = 0;
	virtual void Walk(float d) = 0;

	// Rotate the camera.
	virtual void Pitch(float angle) = 0;
	virtual void RotateWorldY(float angle) = 0;

	virtual DirectX::XMMATRIX GetView()const = 0;
	virtual DirectX::XMFLOAT4X4 GetView4x4()const = 0;
	virtual DirectX::XMMATRIX GetProj()const = 0;
	virtual DirectX::XMFLOAT4X4 GetProj4x4()const = 0;

	// After modifying camera position/orientation, call to rebuild the view matrix.
	virtual void UpdateViewMatrix() = 0;

};

#endif