#include "MeshGenerator.h"
#include "VertexDefs.h"
#include <DirectXColors.h>
#include <array>
#include <GeometricPrimitive.h>

using namespace DirectX;

Mesh MeshGenerator::GenerateTriangle(XMFLOAT3 v1, XMFLOAT3 v2, XMFLOAT3 v3, std::string name)
{
	return Mesh();
}

Mesh MeshGenerator::GenerateUnitCircle(std::string name)
{
	return Mesh();
}

Mesh MeshGenerator::GenerateUnitBox(std::string name)
{
	std::vector<Vertex> vertices =
	{
		Vertex(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan), XMFLOAT3(), XMFLOAT3(), XMFLOAT2()),
		Vertex(XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta), XMFLOAT3(), XMFLOAT3(), XMFLOAT2())
	};

	// Indices defined in a clockwise order to indicate the front facing side
	std::vector<std::uint32_t> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	Mesh boxMesh = Mesh();
	boxMesh.Name = name;
	boxMesh.Vertices = vertices;
	boxMesh.Indices32 = indices;

	int verticesByteSize = boxMesh.Vertices.size() * sizeof(Vertex);
	int indicesByteSize = boxMesh.Indices32.size() * sizeof(uint32_t);


	boxMesh.VertexByteStride = sizeof(Vertex);
	boxMesh.VertexBufferByteSize = verticesByteSize;
	boxMesh.IndexFormat = DXGI_FORMAT_R32_UINT;
	boxMesh.IndexBufferByteSize = indicesByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = boxMesh.Indices32.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	boxMesh.DrawArgs[boxMesh.Name] = submesh;

	return boxMesh;
}

Mesh MeshGenerator::GenerateGrid(std::string name, int width, int length)
{
	Mesh gridMesh = Mesh();

	// Compute grid vertices
	for (int row = 0; row <= width; row++) {
		for (int col = 0; col <= length; col++) {
			auto color = col * row % 2 == 0 ? XMFLOAT4(Colors::Black) : XMFLOAT4(Colors::White);
			Vertex vertex = Vertex(
				XMFLOAT3(-width / 2 + row, -1.0f, -length / 2 + col),
				color,
				XMFLOAT3(), // Normal
				XMFLOAT3(), // Tangent
				XMFLOAT2()  // Tex Coord
			);
			gridMesh.Vertices.push_back(vertex);
		}
	}

	// Compute grid indices
	for (int row = 0; row < width; row++) {
		for (int col = 0; col < length; col++) {
			int tileNum = row * (length + 1) + col;
			int aboveTileNum = tileNum + (length + 1);
			int aboveRightTileNum = aboveTileNum + 1;
			int nextTileNum = tileNum + 1;

			// Add them in counterclockwise order because we want to look at them from the back
			gridMesh.Indices32.push_back(tileNum);
			gridMesh.Indices32.push_back(aboveRightTileNum);
			gridMesh.Indices32.push_back(aboveTileNum);

			gridMesh.Indices32.push_back(tileNum);
			gridMesh.Indices32.push_back(nextTileNum);
			gridMesh.Indices32.push_back(aboveRightTileNum);

			//// Left triangle in quad
			//gridMesh.Indices32.push_back(tileNum);
			//gridMesh.Indices32.push_back(aboveTileNum);
			//gridMesh.Indices32.push_back(aboveRightTileNum);

			//// Right triangle in quad
			//gridMesh.Indices32.push_back(tileNum);
			//gridMesh.Indices32.push_back(aboveRightTileNum);
			//gridMesh.Indices32.push_back(nextTileNum);
		}
	}

	gridMesh.Name = name;

	int verticesByteSize = gridMesh.Vertices.size() * sizeof(Vertex);
	int indicesByteSize = gridMesh.Indices32.size() * sizeof(uint32_t);


	gridMesh.VertexByteStride = sizeof(Vertex);
	gridMesh.VertexBufferByteSize = verticesByteSize;
	gridMesh.IndexFormat = DXGI_FORMAT_R32_UINT;
	gridMesh.IndexBufferByteSize = indicesByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = gridMesh.Indices32.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	gridMesh.DrawArgs[gridMesh.Name] = submesh;

	return gridMesh;
}

Mesh MeshGenerator::GenerateSphere(std::string name, float radius)
{
	Mesh sphereMesh = Mesh();
	std::vector<uint16_t> indices;
	auto vertices = std::vector<GeometricPrimitive::VertexType>();
	GeometricPrimitive::CreateSphere(vertices, indices, radius, 16, false);

	for (auto v : vertices) {
		auto color = XMFLOAT4(v.position.x, v.position.y, v.position.z, 1.0f);
		sphereMesh.Vertices.push_back(
			Vertex(v.position, color, XMFLOAT3(), XMFLOAT3(), XMFLOAT2())
		);
	}

	for (auto indice16 : indices) {
		sphereMesh.Indices32.push_back(indice16);
	}

	sphereMesh.Name = name;

	int verticesByteSize = sphereMesh.Vertices.size() * sizeof(Vertex);
	int indicesByteSize = sphereMesh.Indices32.size() * sizeof(uint32_t);


	sphereMesh.VertexByteStride = sizeof(Vertex);
	sphereMesh.VertexBufferByteSize = verticesByteSize;
	sphereMesh.IndexFormat = DXGI_FORMAT_R32_UINT;
	sphereMesh.IndexBufferByteSize = indicesByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = sphereMesh.Indices32.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	sphereMesh.DrawArgs[sphereMesh.Name] = submesh;

	return sphereMesh;
}

Mesh MeshGenerator::GenerateTeapot(std::string name)
{
	Mesh teapotMesh = Mesh();
	std::vector<uint16_t> indices;
	auto vertices = std::vector<GeometricPrimitive::VertexType>();
	GeometricPrimitive::CreateTeapot(vertices, indices, 1.0f, 16, false);

	for (auto v : vertices) {
		auto color = XMFLOAT4(v.position.x, v.position.y, v.position.z, 1.0f);
		teapotMesh.Vertices.push_back(
			Vertex(v.position, color, XMFLOAT3(), XMFLOAT3(), XMFLOAT2())
		);
	}

	for (auto indice16 : indices) {
		teapotMesh.Indices32.push_back(indice16);
	}

	teapotMesh.Name = name;

	int verticesByteSize = teapotMesh.Vertices.size() * sizeof(Vertex);
	int indicesByteSize = teapotMesh.Indices32.size() * sizeof(uint32_t);


	teapotMesh.VertexByteStride = sizeof(Vertex);
	teapotMesh.VertexBufferByteSize = verticesByteSize;
	teapotMesh.IndexFormat = DXGI_FORMAT_R32_UINT;
	teapotMesh.IndexBufferByteSize = indicesByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = teapotMesh.Indices32.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	teapotMesh.DrawArgs[teapotMesh.Name] = submesh;

	return teapotMesh;
}
