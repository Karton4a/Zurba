module;
#include <d3d12.h>
#include <wrl.h>
#include <optional>
#include <vector>
export module DX12GPUStackAllocator;

import Types;

class DX12HeapHandler
{
public:
	DX12HeapHandler(size_t aSize, D3D12_HEAP_TYPE aType);
	std::optional<u64> Allocate(size_t aSize, size_t aAlignment);
	void Reset();
	ID3D12Heap* GetHeap() { return m_Heap.Get(); }
private:
	u64 m_Offset = 0;
	u64 m_HeapSize = 0;
	Microsoft::WRL::ComPtr<ID3D12Heap> m_Heap;
};

export class DX12GPUStackAllocator
{
public:
	DX12GPUStackAllocator() = default;
	~DX12GPUStackAllocator();

	DX12GPUStackAllocator(const DX12GPUStackAllocator&) = delete;
	DX12GPUStackAllocator& operator=(const DX12GPUStackAllocator&) = delete;
	DX12GPUStackAllocator(DX12GPUStackAllocator&&) = delete;
	DX12GPUStackAllocator& operator=(DX12GPUStackAllocator&&) = delete;

	struct AllocationHandle
	{
		u64 Offset;
		ID3D12Heap* m_Heap;
	};

	AllocationHandle AllocateUpload(size_t aSize, size_t aAlignment);
	AllocationHandle AllocateReadBack(size_t aSize, size_t aAlignment);
	AllocationHandle AllocateGPUOnly(size_t aSize, size_t aAlignment);
	void Reset();
private:
	AllocationHandle AllocateGeneral(std::vector<DX12HeapHandler>& heapHandlers, size_t aHeapPageSize, D3D12_HEAP_TYPE aType, size_t size, size_t alignment);
private:
	std::vector<DX12HeapHandler> m_UploadHeapHandlers;
	std::vector<DX12HeapHandler> m_ReadBackHeapHandlers;
	std::vector<DX12HeapHandler> m_GPUOnlyHeapHandlers;
};