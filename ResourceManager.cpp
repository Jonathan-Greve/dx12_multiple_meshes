#include "ResourceManager.h"
#include "d3dUtility.h"
#include "d3dx12.h"
using namespace Microsoft::WRL;

ResourceManager::ResourceManager()
{
}

ResourceManager::ResourceManager(ID3D12GraphicsCommandList* commandList, ID3D12Device* device) :
	m_commandList(commandList),
	m_device(device)
{
}

void ResourceManager::AddMesh(Mesh mesh)
{
	if (!m_freeCBPerObjectIndex.empty()) {
		mesh.cbPerObjectIndex = m_freeCBPerObjectIndex.back();
		m_freeCBPerObjectIndex.pop_back();
	}
	else {
		mesh.cbPerObjectIndex = m_newCBPerObjectIndex;
		m_newCBPerObjectIndex++;
	}
	if (m_meshes.count(mesh.Name) == 0) {
		mesh.VertexBufferGPU = CreateDefaultBuffer(m_device,
			m_commandList, mesh.Vertices.data(), mesh.VertexBufferByteSize, mesh.VertexBufferUploader);

		mesh.IndexBufferGPU = CreateDefaultBuffer(m_device,
			m_commandList, mesh.Indices32.data(), mesh.IndexBufferByteSize, mesh.IndexBufferUploader);

		m_meshes[mesh.Name] = mesh;
	}
	else {
		UpdateMesh(mesh);
	}
}

void ResourceManager::UpdateMesh(Mesh mesh)
{
	throw("UpdateMesh() function not implemented");
}

void ResourceManager::DeleteMesh(const Mesh& mesh)
{
	m_meshes.erase(mesh.Name);
	m_freeCBPerObjectIndex.push_back(mesh.cbPerObjectIndex);
}

Mesh ResourceManager::GetMesh(std::string meshName)
{
	return m_meshes[meshName];
}

std::unordered_map<std::string, Mesh> ResourceManager::GetAllMeshes()
{
	return m_meshes;
}

void ResourceManager::AddConstantBuffer(std::string name, UINT elementByteSize, UINT numOfElements, ID3D12DescriptorHeap* cbvHeap, UINT cbvHeapDescriptorSize)
{
	m_constantBuffers[name] = std::make_unique<UploadBuffer>(m_device, numOfElements, elementByteSize, true);

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_constantBuffers[name]->Resource()->GetGPUVirtualAddress();
	UINT objCBByteSize = m_constantBuffers[name]->GetElementPaddedByteSize();

	for (int i = 0; i < numOfElements; i++) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress + i * objCBByteSize;
		cbvDesc.SizeInBytes = objCBByteSize * numOfElements;

		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(m_currCBVHeapIndex, cbvHeapDescriptorSize);


		m_device->CreateConstantBufferView(&cbvDesc, handle);

		m_currCBVHeapIndex++;
	}

}

void ResourceManager::RemoveConstantBuffer(std::string name)
{
	m_constantBuffers.erase(name);
}

template <typename T>
void ResourceManager::UpdateConstantBuffer(std::string name, int elementIndex, const T& pData)
{
	m_constantBuffers[name]->CopyData(elementIndex, pData);
}

Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	const void* pData,
	UINT64 pDataByteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	auto defaultProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto defaultDesc = CD3DX12_RESOURCE_DESC::Buffer(pDataByteSize);
	device->CreateCommittedResource(
		&defaultProperties,
		D3D12_HEAP_FLAG_NONE,
		&defaultDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	auto uploadProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(pDataByteSize);
	device->CreateCommittedResource(
		&uploadProperties,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf()));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pData;
	subResourceData.RowPitch = pDataByteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.

	auto defaultTransitionAsDest = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(1, &defaultTransitionAsDest);

	UpdateSubresources<1>(commandList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

	auto defaultTransitionAsGPURead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &defaultTransitionAsGPURead);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.

	return defaultBuffer;

}

template void ResourceManager::UpdateConstantBuffer<MeshConstants>(std::string name, int elementIndex, const MeshConstants& pData);
template void ResourceManager::UpdateConstantBuffer<PassConstants>(std::string name, int elementIndex, const PassConstants& pData);
