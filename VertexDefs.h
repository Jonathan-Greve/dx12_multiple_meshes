#ifndef VERTEXDEFS_H_ 
#define VERTEXDEFS_H_
#include <DirectXMath.h>

using DirectX::XMFLOAT4;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT2;

struct Vertex {
	Vertex() = default;

	Vertex(
		const XMFLOAT3& position,
		const XMFLOAT4& color,
		const XMFLOAT3& normal,
		const XMFLOAT3& tangent,
		const XMFLOAT2& texCord) :
		Position(position),
		Color(color),
		Normal(normal),
		TangentU(tangent),
		TexCord(texCord) {}

	Vertex(
		float posX, float posY, float posZ,
		float red, float blue, float green, float alpha,
		float normalX, float normalY, float normalZ,
		float tangentX, float tangentY, float tangentZ,
		float u, float v) :
		Position(posX, posY, posZ),
		Color(red, blue, green, alpha),
		Normal(normalX, normalY, normalZ),
		TangentU(tangentX, tangentY, tangentZ),
		TexCord(u, v) {}

	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT3 Normal;
	XMFLOAT3 TangentU;
	XMFLOAT2 TexCord;
};

#endif
