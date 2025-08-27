module;
#include <array>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <vector>
export module DX12PixelStorage;

import DX12DeviceManager;

export class DX12PixelStorage
{
public:
    DX12PixelStorage(D3D12_RESOURCE_DIMENSION type, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth);
    void LoadData(const uint8_t* data, uint32_t elementSize);
    void Flush(ID3D12GraphicsCommandList* pCmdList);
    ID3D12Resource* GetResource() { return m_GPUBuffer.Get(); };

private:
    void UpdateSubresources(std::vector<D3D12_SUBRESOURCE_DATA>& aSubresources);

    D3D12_RESOURCE_DESC m_Description = {};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_GPUBuffer;
    uint64_t m_RequiredIntermediateResourceSize;
    struct SubresourceData
    {
        static constexpr uint32_t MAXSUBRESOURCES = 12;
        std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, MAXSUBRESOURCES> Footprint;
        std::array<uint64_t, MAXSUBRESOURCES> RowSizeInBytes;
        std::array<uint32_t, MAXSUBRESOURCES> NumRows;
    };
    SubresourceData m_SubresourceData;
};