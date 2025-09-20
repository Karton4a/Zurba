module;
#include <d3d12.h>
#include <wrl.h>
#include <optional>
#include <cassert>

#include "../../DXHelpers.h"
module DX12GPUStackAllocator;
import DX12DeviceManager;
import CompitmeConstants;

static u64 AlignUp(u64 value, u64 alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}

DX12HeapHandler::DX12HeapHandler(size_t aSize, D3D12_HEAP_TYPE aType)
{
	m_HeapSize = aSize;
	m_Offset = 0;
	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = m_HeapSize;
	heapDesc.Properties.Type = aType;
	HRESULT h = DX12DeviceManager::GetInstance()->GetDevice()->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));
	ThrowIfFailed(h);
	if constexpr (_Debug)
	{
		m_Heap->SetName(L"Stack Allocator Heap");
	}
}

std::optional<u64> DX12HeapHandler::Allocate(size_t size, size_t alignment)
{
	u64 addres = AlignUp(m_Offset, alignment);

	if (addres + size > m_HeapSize)
	{
		return std::nullopt;
	}

	m_Offset = addres + size;
	return addres;
}

void DX12HeapHandler::Reset()
{
	m_Offset = 0;
}


DX12GPUStackAllocator::~DX12GPUStackAllocator()
{
	m_GPUOnlyHeapHandlers.clear();
	m_ReadBackHeapHandlers.clear();
	m_UploadHeapHandlers.clear();
}

DX12GPUStackAllocator::AllocationHandle DX12GPUStackAllocator::AllocateUpload(size_t aSize, size_t aAlignment)
{
	return AllocateGeneral(m_UploadHeapHandlers, 1024, D3D12_HEAP_TYPE_UPLOAD, aSize, aAlignment);
}

DX12GPUStackAllocator::AllocationHandle DX12GPUStackAllocator::AllocateReadBack(size_t aSize, size_t aAlignment)
{
	return AllocateGeneral(m_ReadBackHeapHandlers, 1024, D3D12_HEAP_TYPE_READBACK, aSize, aAlignment);
}

DX12GPUStackAllocator::AllocationHandle DX12GPUStackAllocator::AllocateGPUOnly(size_t aSize, size_t aAlignment)
{
	return AllocateGeneral(m_GPUOnlyHeapHandlers, 1024, D3D12_HEAP_TYPE_DEFAULT, aSize, aAlignment);
}

void DX12GPUStackAllocator::Reset()
{
	for (auto& handler : m_GPUOnlyHeapHandlers)
	{
		handler.Reset();
	}
	for (auto& handler : m_ReadBackHeapHandlers)
	{
		handler.Reset();
	}
	for (auto& handler : m_UploadHeapHandlers)
	{
		handler.Reset();
	}
}

DX12GPUStackAllocator::AllocationHandle DX12GPUStackAllocator::AllocateGeneral(
	std::vector<DX12HeapHandler>& aHeapHandlers, 
	size_t aHeapPageSize,
	D3D12_HEAP_TYPE aType,
	size_t aAllocationSize,
	size_t aAlignment)
{
	std::optional<u64> allocationOffset;
	ID3D12Heap* heap = nullptr;

	for (auto& handler : aHeapHandlers)
	{
		allocationOffset = handler.Allocate(aAllocationSize, aAlignment);
		if (allocationOffset)
		{
			heap = handler.GetHeap();
			break;
		}
	}

	if (!allocationOffset)
	{
		aHeapHandlers.emplace_back(aHeapPageSize, aType);
		allocationOffset = aHeapHandlers.back().Allocate(aAllocationSize, aAlignment);
		heap = aHeapHandlers.back().GetHeap();
	}

	assert(allocationOffset);
	assert(heap);

	return { *allocationOffset, heap };
}
