module;
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>
#include "DXHelpers.h"
#include <cassert>
module DX12DescriptorHeap;
import DX12DeviceManager;

static D3D12_DESCRIPTOR_HEAP_TYPE ConvertDescriptorHeapType(DescriptorHeapType type)
{
    switch (type)
    {
    case DescriptorHeapType::CBV_SRV_UAV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case DescriptorHeapType::DSV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    case DescriptorHeapType::RTV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case DescriptorHeapType::SAMPLER:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    default:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }
}

DX12DescriptorHeap::DX12DescriptorHeap(DescriptorHeapType heapType, uint32_t heapSize)
	:m_HeapType(heapType), m_HeapSize(heapSize), m_FreeDescriptors()
{
	D3D12_DESCRIPTOR_HEAP_TYPE type = ConvertDescriptorHeapType(m_HeapType);
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = m_HeapSize;
    samplerHeapDesc.Type = type;
	D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_HeapType == DescriptorHeapType::CBV_SRV_UAV || m_HeapType == DescriptorHeapType::SAMPLER)
    {
		flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    }
    samplerHeapDesc.Flags = flags;

    Microsoft::WRL::ComPtr<ID3D12Device> device = DX12DeviceManager::GetInstance()->GetDevice();
    ThrowIfFailed(device->CreateDescriptorHeap(
        &samplerHeapDesc, IID_PPV_ARGS(&m_HeapResource)));
	m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);
}

uint32_t DX12DescriptorHeap::AllocateDescriptor()
{
    if(m_FreeDescriptors.empty())
    {
        assert(m_CurrentDescriptorTop < m_HeapSize, "No Free Descriptors");
        return m_CurrentDescriptorTop++;
    }
    else
    {
        uint32_t index = m_FreeDescriptors.back();
        m_FreeDescriptors.pop_back();
        return index;
	}
}

void DX12DescriptorHeap::FreeDescriptor(uint32_t aDescriptorIndex)
{
    assert(aDescriptorIndex < m_HeapSize, "Invalid Descriptor Index");
	m_FreeDescriptors.push_back(aDescriptorIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetCPUDescriptorHandle(uint32_t aDescriptorIndex)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_HeapResource->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += aDescriptorIndex * m_DescriptorSize;
    return handle;
}
