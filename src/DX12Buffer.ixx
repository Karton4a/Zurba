module;
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>
#include "DXHelpers.h"
export module DX12Buffer;
import DX12DeviceManager;

export class DX12Buffer
{
public:
    DX12Buffer(const void* data, uint64_t dataSize);

    void Flush(ID3D12GraphicsCommandList* pCmdList);

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_GPUBuffer->GetGPUVirtualAddress(); }
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_GPUBuffer;
	uint64_t m_BufferSize;
};