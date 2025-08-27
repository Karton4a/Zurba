#include "DX12DeviceManager.h"
#include "DX12PixelStorage.h"
#include "DXHelpers.h"
#include <cstdint>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <winnt.h>
#include <wrl/client.h>
#include <cassert>

DX12PixelStorage::DX12PixelStorage(D3D12_RESOURCE_DIMENSION type, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth)
{
    m_Description.Dimension = type;
    m_Description.MipLevels = 1;
    m_Description.Format = format;
    m_Description.Width = width;
    m_Description.Height = height;
    m_Description.Flags = D3D12_RESOURCE_FLAG_NONE;
    m_Description.DepthOrArraySize = depth;
    m_Description.SampleDesc.Count = 1;
    m_Description.SampleDesc.Quality = 0;
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    ThrowIfFailed(DX12DeviceManager::GetInstance()->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &m_Description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&m_GPUBuffer)));

    Microsoft::WRL::ComPtr<ID3D12Device> pDevice = DX12DeviceManager::GetInstance()->GetDevice();
    pDevice->GetCopyableFootprints(&m_Description, 0, 1, 0, m_SubresourceData.Footprint.data(),
                                   m_SubresourceData.NumRows.data(), m_SubresourceData.RowSizeInBytes.data(),
                                   &m_RequiredIntermediateResourceSize);
}

void DX12PixelStorage::LoadData(const uint8_t *data, uint32_t elementSize)
{

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC uploadBufferResourceDesc;
    uploadBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadBufferResourceDesc.Alignment = 0;
    uploadBufferResourceDesc.Width = m_RequiredIntermediateResourceSize;
    uploadBufferResourceDesc.Height = 1;
    uploadBufferResourceDesc.DepthOrArraySize = 1;
    uploadBufferResourceDesc.MipLevels = 1;
    uploadBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadBufferResourceDesc.SampleDesc.Count = 1;
    uploadBufferResourceDesc.SampleDesc.Quality = 0;
    uploadBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create the GPU upload buffer.
    ThrowIfFailed(DX12DeviceManager::GetInstance()->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_UploadBuffer)));

    // Copy data to the intermediate upload heap and then schedule a copy
    // from the upload heap to the Texture2D.

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = data;
    textureData.RowPitch = m_Description.Width * elementSize;
    textureData.SlicePitch = textureData.RowPitch * m_Description.Height;

    std::vector<D3D12_SUBRESOURCE_DATA> subresources = { textureData };

    UpdateSubresources(subresources);

}
void DX12PixelStorage::Flush(ID3D12GraphicsCommandList* pCmdList)
{
    for (UINT i = 0; i < 1; ++i)
    {

        D3D12_TEXTURE_COPY_LOCATION Dst;
        Dst.pResource = m_GPUBuffer.Get();
        Dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        Dst.PlacedFootprint = {};
        Dst.SubresourceIndex = i;

        D3D12_TEXTURE_COPY_LOCATION Src;

        Src.pResource = m_UploadBuffer.Get();
        Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        Src.PlacedFootprint = m_SubresourceData.Footprint[i];

        pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
    }
    //TODO Can Delete upload buffer
}
void DX12PixelStorage::UpdateSubresources(std::vector<D3D12_SUBRESOURCE_DATA>& aSubresources)
{
    uint8_t *pData;
    HRESULT hr = m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void **>(&pData));
    if (FAILED(hr))
    {
        return;
    }

    assert(aSubresources.size() <= SubresourceData::MAXSUBRESOURCES);

    for (UINT i = 0; i < aSubresources.size(); ++i)
    {
        if (m_SubresourceData.RowSizeInBytes[i] > SIZE_T(-1))
            return;

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint = m_SubresourceData.Footprint[i];
        D3D12_MEMCPY_DEST DestData = { pData + footprint.Offset, footprint.Footprint.RowPitch,
                                      SIZE_T(footprint.Footprint.RowPitch) * SIZE_T(m_SubresourceData.NumRows[i]) };

        auto NumSlices = footprint.Footprint.Depth;
        auto NumRows = m_SubresourceData.NumRows[i];
        auto RowSizeInBytes = m_SubresourceData.RowSizeInBytes[i];
        D3D12_SUBRESOURCE_DATA& sourceData = aSubresources[i];
        for (UINT z = 0; z < NumSlices; ++z)
        {
            auto pDestSlice = static_cast<BYTE*>(DestData.pData) + DestData.SlicePitch * z;
            auto pSrcSlice = static_cast<const BYTE*>(sourceData.pData) + sourceData.SlicePitch * LONG_PTR(z);
            for (UINT y = 0; y < NumRows; ++y)
            {
                memcpy(pDestSlice + DestData.RowPitch * y,
                    pSrcSlice + sourceData.RowPitch * LONG_PTR(y),
                    RowSizeInBytes);
            }
        }
    }

    m_UploadBuffer->Unmap(0, nullptr);
}
