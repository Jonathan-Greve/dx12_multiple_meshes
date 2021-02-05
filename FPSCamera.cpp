#include "FPSCamera.h"

using namespace DirectX;

XMVECTOR FPSCamera::GetPosition() const
{
	return XMLoadFloat3(&m_position);
}

XMFLOAT3 FPSCamera::GetPosition3f() const
{
	return m_position;
}

void FPSCamera::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
}

XMVECTOR FPSCamera::GetRight() const
{
	return XMLoadFloat3(&m_right);
}

DirectX::XMFLOAT3 FPSCamera::GetRight3f() const
{
	return m_right;
}

XMVECTOR FPSCamera::GetUp() const
{
	return XMLoadFloat3(&m_up);
}

DirectX::XMFLOAT3 FPSCamera::GetUp3f() const
{
	return m_up;
}

XMVECTOR FPSCamera::GetLook() const
{
	return XMLoadFloat3(&m_look);
}

DirectX::XMFLOAT3 FPSCamera::GetLook3f() const
{
	return m_look;
}

void FPSCamera::SetFrustum(float fovY, float aspectRatio, float zNear, float zFar)
{
	XMStoreFloat4x4(&m_proj, XMMatrixPerspectiveFovLH(fovY, aspectRatio, zNear, zFar));
}

void FPSCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR look = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(worldUp, look));
	XMVECTOR up = XMVector3Cross(right, look);

	XMStoreFloat3(&m_look, look);
	XMStoreFloat3(&m_right, right);
	XMStoreFloat3(&m_up, up);
	XMStoreFloat3(&m_position, pos);
}

void FPSCamera::Strafe(float d)
{
	XMVECTOR dVec = XMVectorReplicate(d);
	XMVECTOR newPos = XMVectorMultiplyAdd(XMLoadFloat3(&m_right), dVec, XMLoadFloat3(&m_position));
	XMStoreFloat3(&m_position, newPos);
}

void FPSCamera::Walk(float d)
{
	XMVECTOR dVec = XMVectorReplicate(d);
	XMVECTOR newPos = XMVectorMultiplyAdd(XMLoadFloat3(&m_look), dVec, XMLoadFloat3(&m_position));
	XMStoreFloat3(&m_position, newPos);
}

void FPSCamera::Pitch(float angle)
{
	auto rotationMatrix = XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);
	XMVECTOR up = XMVector3TransformNormal(XMLoadFloat3(&m_up), rotationMatrix);
	XMVECTOR look = XMVector3TransformNormal(XMLoadFloat3(&m_look), rotationMatrix);
	XMStoreFloat3(&m_up, up);
	XMStoreFloat3(&m_look, look);
}

void FPSCamera::RotateWorldY(float angle)
{
	auto rotationMatrix = XMMatrixRotationY(angle);
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotationMatrix));
	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotationMatrix));
	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), rotationMatrix));
}

DirectX::XMMATRIX FPSCamera::GetView() const
{
	return XMLoadFloat4x4(&m_view);
}

DirectX::XMFLOAT4X4 FPSCamera::GetView4x4() const
{
	return m_view;
}

DirectX::XMMATRIX FPSCamera::GetProj() const
{
	return XMLoadFloat4x4(&m_proj);
}

DirectX::XMFLOAT4X4 FPSCamera::GetProj4x4() const
{
	return m_proj;
}

void FPSCamera::UpdateViewMatrix()
{
	XMVECTOR look = XMLoadFloat3(&m_look);
	XMVECTOR up = XMLoadFloat3(&m_up);
	XMVECTOR right = XMLoadFloat3(&m_right);
	XMVECTOR position = XMLoadFloat3(&m_position);

	// Re-orthorganize the camera basis
	look = XMVector3Normalize(look);
	up = XMVector3Normalize(XMVector3Cross(look, right));
	right = XMVector3Normalize(XMVector3Cross(up, look));

	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(position, look, up));

	//XMStoreFloat3(&m_look, look);
	//XMStoreFloat3(&m_up, up);
	//XMStoreFloat3(&m_right, right);
	//XMStoreFloat3(&m_position, position);

	//float x = -XMVectorGetX(XMVector3Dot(position, right));
	//float y = -XMVectorGetX(XMVector3Dot(position, up));
	//float z = -XMVectorGetX(XMVector3Dot(position, look));

	//// Col 1 
	//m_view(0, 0) = m_right.x;
	//m_view(1, 0) = m_right.y;
	//m_view(2, 0) = m_right.z;
	//m_view(3, 0) = x;

	//// Col 2
	//m_view(0, 1) = m_up.x;
	//m_view(1, 1) = m_up.y;
	//m_view(2, 1) = m_up.z;
	//m_view(3, 1) = y;

	//// Col 3
	//m_view(0, 2) = m_look.x;
	//m_view(1, 2) = m_look.y;
	//m_view(2, 2) = m_look.z;
	//m_view(3, 2) = z;

	//// Col 4
	//m_view(0, 3) = 0;
	//m_view(1, 3) = 0;
	//m_view(2, 3) = 0;
	//m_view(3, 3) = 1;
}
