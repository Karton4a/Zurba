module;
#include <d3d12.h>
#include <wrl.h>
module DX12DepthStencilView;

DX12DepthStencilView::DX12DepthStencilView(ID3D12Resource* aResource)
{
    D3D12_RESOURCE_DESC desc = aResource->GetDesc();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = desc.Format;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    Microsoft::WRL::ComPtr<ID3D12Device> device = DX12DeviceManager::GetInstance()->GetDevice();

    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetDepthStencilDescriptorHeap();

    m_DescriptorIndex = descriptorHeap->AllocateDescriptor();
    device->CreateDepthStencilView(aResource, &dsvDesc, descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex));
}

DX12DepthStencilView::~DX12DepthStencilView()
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetDepthStencilDescriptorHeap();
	descriptorHeap->FreeDescriptor(m_DescriptorIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DepthStencilView::GetHandle() const
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetDepthStencilDescriptorHeap();
    return descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex);
}
