#ifndef RESOURCEMANAGER_H_ 
#define RESOURCEMANAGER_H_

#include "Mesh.h"
#include "UploadBuffer.h"
#include <DirectXMath.h>

class ResourceManager
{
public:
	ResourceManager();
	ResourceManager(ID3D12GraphicsCommandList* commandList, ID3D12Device* device);

	void AddMesh(Mesh mesh);
	void UpdateMesh(Mesh mesh);
	void DeleteMesh(const Mesh& mesh);
	Mesh GetMesh(std::string meshName);
	std::unordered_map<std::string, Mesh> GetAllMeshes ();

	void AddConstantBuffer(std::string name, UINT elementByteSize, UINT numOfElements, ID3D12DescriptorHeap* cbvHeap, UINT cbvHeapDescriptorSize);
	void RemoveConstantBuffer(std::string name);

	template <typename T>
	void UpdateConstantBuffer(std::string name, int elementIndex, const T& pData);

private:
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12Device* m_device = nullptr;
	std::unordered_map<std::string, Mesh> m_meshes;
	std::unordered_map<std::string, std::unique_ptr<UploadBuffer>> m_constantBuffers;

	UINT m_currCBVHeapIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList,
		const void* pData,
		UINT64 pDataByteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
};

struct MeshConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper().GetIdentity4x4();

	// Explicit padding of 192 BYTES for a total of 256 as required by constant buffers.
	DirectX::XMFLOAT4X4 Pad0 = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 Pad1 = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 Pad2 = DirectX::XMFLOAT4X4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 ViewProj = MathHelper().GetIdentity4x4();

	// Explicit padding of 192 BYTES for a total of 256 as required by constant buffers.
	DirectX::XMFLOAT4X4 Pad0 = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 Pad1 = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 Pad2 = DirectX::XMFLOAT4X4();
};

#endif