module;
#include <d3d12.h>
#include <cassert>
#include <wrl.h>
module DX12ShaderResourceView;
import DX12DeviceManager;


DX12ShaderResourceView::DX12ShaderResourceView(std::shared_ptr<DX12PixelStorage>& aPixelStorage,
	D3D12_SRV_DIMENSION aDimension,
	D3D12_SHADER_RESOURCE_VIEW_DESC& aDesc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //TODO think how to pass Shader4ComponentMapping
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = aPixelStorage->GetFormat();
    srvDesc.ViewDimension = aDimension;
    switch (aDimension)
    {
    case D3D12_SRV_DIMENSION_UNKNOWN:
        assert(false);
		break;
    case D3D12_SRV_DIMENSION_BUFFER:
		srvDesc.Buffer = aDesc.Buffer;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE1D:
		srvDesc.Texture1D = aDesc.Texture1D;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
		srvDesc.Texture1DArray = aDesc.Texture1DArray;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2D:
		srvDesc.Texture2D = aDesc.Texture2D;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
		srvDesc.Texture2DArray = aDesc.Texture2DArray;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2DMS:
		srvDesc.Texture2DMS = aDesc.Texture2DMS;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
		srvDesc.Texture2DMSArray = aDesc.Texture2DMSArray;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE3D:
		srvDesc.Texture3D = aDesc.Texture3D;
        break;
    case D3D12_SRV_DIMENSION_TEXTURECUBE:
		srvDesc.TextureCube = aDesc.TextureCube;
        break;
    case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
		srvDesc.TextureCubeArray = aDesc.TextureCubeArray;
        break;
    case D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE:
		srvDesc.RaytracingAccelerationStructure = aDesc.RaytracingAccelerationStructure;
        break;
    default:
        assert(false);
        break;
    }
    Microsoft::WRL::ComPtr<ID3D12Device> device = DX12DeviceManager::GetInstance()->GetDevice();

    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();

    m_DescriptorIndex = descriptorHeap->AllocateDescriptor();
    device->CreateShaderResourceView(aPixelStorage->GetResource(), & srvDesc, descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex));

}

DX12ShaderResourceView::~DX12ShaderResourceView()
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();
	descriptorHeap->FreeDescriptor(m_DescriptorIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ShaderResourceView::GetHandle() const
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();
    return descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex);
}
