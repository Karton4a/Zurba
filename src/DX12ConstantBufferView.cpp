module;
#include <d3d12.h>
#include <wrl.h>
module DX12ConstantBufferView;
import DX12DeviceManager;

DX12ConstantBufferView::DX12ConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS aBuffer, uint32_t aSize)
{
    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = aBuffer;
    cbvDesc.SizeInBytes = aSize;

    Microsoft::WRL::ComPtr<ID3D12Device> device = DX12DeviceManager::GetInstance()->GetDevice();

	std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();

	m_DescriptorIndex = descriptorHeap->AllocateDescriptor();
    device->CreateConstantBufferView(&cbvDesc, descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex));
}

DX12ConstantBufferView::~DX12ConstantBufferView()
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();
	descriptorHeap->FreeDescriptor(m_DescriptorIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ConstantBufferView::GetHandle() const
{
    std::unique_ptr<DX12DescriptorHeap>& descriptorHeap = DX12DeviceManager::GetInstance()->GetShaderVisibleDescriptorHeap();
    return descriptorHeap->GetCPUDescriptorHandle(m_DescriptorIndex);
}
