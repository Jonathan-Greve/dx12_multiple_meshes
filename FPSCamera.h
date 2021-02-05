#ifndef FPSCamera_H_ 
#define FPSCamera_H_

#include "ICamera.h"
class FPSCamera : public ICamera
{
public:
	// Inherited via ICamera
	virtual DirectX::XMVECTOR GetPosition() const override;
	virtual DirectX::XMFLOAT3 GetPosition3f() const override;

	virtual void SetPosition(float x, float y, float z) override;
	virtual DirectX::XMVECTOR GetRight() const override;
	virtual DirectX::XMFLOAT3 GetRight3f() const override;

	virtual DirectX::XMVECTOR GetUp() const override;
	virtual DirectX::XMFLOAT3 GetUp3f() const override;

	virtual DirectX::XMVECTOR GetLook() const override;
	virtual DirectX::XMFLOAT3 GetLook3f() const override;

	virtual void SetFrustum(float fovY, float aspect, float zn, float zf) override;
	virtual void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp) override;
	virtual void Strafe(float d) override;
	virtual void Walk(float d) override;
	virtual void Pitch(float angle) override;
	virtual void RotateWorldY(float angle) override;

	virtual DirectX::XMMATRIX GetView() const override;
	virtual DirectX::XMFLOAT4X4 GetView4x4() const override;
	virtual DirectX::XMMATRIX GetProj() const override;
	virtual DirectX::XMFLOAT4X4 GetProj4x4() const override;

	virtual void UpdateViewMatrix() override;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT3 m_look;

	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_proj;
};

#endif
