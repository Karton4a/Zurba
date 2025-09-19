module;
#include <cstdint>
#include <d3d12.h>
export module DX12ConstantBufferView;

import DX12Buffer;
import DX12DeviceManager;
import DX12ResourceView;
export class DX12ConstantBufferView : public DX12ResourceView
{
public:
	DX12ConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS aBuffer, uint32_t aSize);
	~DX12ConstantBufferView();
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const override;
};