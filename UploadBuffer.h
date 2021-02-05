#include "d3dUtility.h"
#include "d3dx12.h"

#ifndef UPLOADBUFFER_H_ 
#define UPLOADBUFFER_H_

class UploadBuffer
{
public:
    UploadBuffer() {};
    UploadBuffer(ID3D12Device* device, UINT elementCount, UINT elementByteSize, bool isConstantBuffer) :
        m_isConstantBuffer(isConstantBuffer)
    {
        m_elementByteSize = elementByteSize;

        if (isConstantBuffer)
            m_elementByteSize = (elementByteSize + 255) & ~255;

        auto uploadProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)m_elementByteSize * elementCount);
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadProperties,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadBuffer)));

        ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    //UploadBuffer(const UploadBuffer& rhs) = delete;
    //UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if (m_uploadBuffer != nullptr)
            m_uploadBuffer->Unmap(0, nullptr);

        m_mappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return m_uploadBuffer.Get();
    }

    template <typename T>
    void CopyData(int elementIndex, const T& pData)
    {
        T data = pData;
        memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, m_elementByteSize);
    }

    UINT GetElementPaddedByteSize() {
        return (m_elementByteSize + 255) & ~255;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
    BYTE* m_mappedData = nullptr;

    UINT m_elementByteSize = 0;
    bool m_isConstantBuffer = false;

    struct MeshConstants
    {
        DirectX::XMFLOAT4X4 WorldViewProj = DirectX::XMFLOAT4X4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        // Explicit padding of 192 BYTES for a total of 256 as required by constant buffers.
        DirectX::XMFLOAT4X4 Pad0 = DirectX::XMFLOAT4X4();
        DirectX::XMFLOAT4X4 Pad1 = DirectX::XMFLOAT4X4();
        DirectX::XMFLOAT4X4 Pad2 = DirectX::XMFLOAT4X4();
    };

};

#endif
