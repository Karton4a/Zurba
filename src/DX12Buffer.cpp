#include "DX12Buffer.h"
#include "DXHelpers.h"
#include "DX12DeviceManager.h"

DX12Buffer::DX12Buffer(const void* data, uint64_t dataSize)
	:m_BufferSize(dataSize)
{
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;


    D3D12_RESOURCE_DESC vertexBufferResourceDesc;
    vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferResourceDesc.Alignment = 0;
    vertexBufferResourceDesc.Width = dataSize;
    vertexBufferResourceDesc.Height = 1;
    vertexBufferResourceDesc.DepthOrArraySize = 1;
    vertexBufferResourceDesc.MipLevels = 1;
    vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferResourceDesc.SampleDesc.Count = 1;
    vertexBufferResourceDesc.SampleDesc.Quality = 0;
    vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(DX12DeviceManager::GetInstance()->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_UploadBuffer)));

    UINT8* pVertexDataBegin;
    D3D12_RANGE readRange;        // We do not intend to read from this resource on the CPU.
    readRange.Begin = 0;
    readRange.End = 0;
    ThrowIfFailed(m_UploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, data, dataSize);
    m_UploadBuffer->Unmap(0, nullptr);


    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    ThrowIfFailed(DX12DeviceManager::GetInstance()->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_GPUBuffer)));

}

void DX12Buffer::Flush(ID3D12GraphicsCommandList* pCmdList)
{
    pCmdList->CopyBufferRegion(m_GPUBuffer.Get(), 0, m_UploadBuffer.Get(), 0, m_BufferSize);
}
