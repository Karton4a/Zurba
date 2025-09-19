module;
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <vector>
export module DX12DescriptorHeap;
export enum class DescriptorHeapType
{
	CBV_SRV_UAV,
	RTV,
	DSV,
	SAMPLER,

};
export class DX12DescriptorHeap
{
public:
	DX12DescriptorHeap(DescriptorHeapType aHeapType, uint32_t aNumDescriptors);
	uint32_t AllocateDescriptor();
	void FreeDescriptor(uint32_t aDescriptorIndex);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t aDescriptorIndex);
	ID3D12DescriptorHeap* GetHeapResource() { return m_HeapResource.Get(); };
private:
	uint32_t m_HeapSize;
	DescriptorHeapType m_HeapType;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_HeapResource;
	std::vector<uint32_t> m_FreeDescriptors;
	uint32_t m_CurrentDescriptorTop = 0;
	UINT m_DescriptorSize = 0;
};