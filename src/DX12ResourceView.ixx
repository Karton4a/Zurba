module;
#include <cstdint>
#include <d3d12.h>
export module DX12ResourceView;
import DX12DeviceManager;

export class DX12ResourceView
{
	public:
	DX12ResourceView() = default;
	virtual ~DX12ResourceView() = default;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const = 0;
	uint32_t GetDescriptorIndex() const { return m_DescriptorIndex; };
protected:
	uint32_t m_DescriptorIndex = 0;
};