#include "MeshGenerator.h"
#include "VertexDefs.h"
#include <DirectXColors.h>
#include <array>

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
	for (int row = 1; row < width + 1; row++) {
		for (int col = 1; col < length + 1; col++) {
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
	for (int row = 0; row < width - 1; row++) {
		for (int col = 0; col < length - 1; col++) {
			int tileNum = row * width + col;
			int aboveTileNum = (row+1) * width + col;
			int aboveRightTileNum = (row+1) * width + (col + 1);
			int nextTileNum = row * width + (col + 1);

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
