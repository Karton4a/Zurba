module;
#include <d3d12.h>
#include <wrl.h>
module DX12RenderTargetView;
import DX12DeviceManager;

DX12RenderTargetView::DX12RenderTargetView(ID3D12Resource* aResource)
{
	Microsoft::WRL::ComPtr<ID3D12Device> device = DX12DeviceManager::GetInstance()->GetDevice();
	std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetRenderTargetDescriptorHeap();
	m_DescriptorIndex = descriptorHeap->AllocateDescriptor();
	device->CreateRenderTargetView(aResource, nullptr, descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex));
}
D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderTargetView::GetHandle() const
{
	std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetRenderTargetDescriptorHeap();
	return descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex);
}
DX12RenderTargetView::~DX12RenderTargetView()
{
	std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetRenderTargetDescriptorHeap();
	descriptorHeap->FreeDescriptor(m_DescriptorIndex);
}