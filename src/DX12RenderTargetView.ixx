module;
#include <cstdint>
#include <d3d12.h>
export module DX12RenderTargetView;
import DX12ResourceView;
import DX12DeviceManager;
export class DX12RenderTargetView : public DX12ResourceView
{
public:
	DX12RenderTargetView(ID3D12Resource* aResource);
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const override;
	virtual ~DX12RenderTargetView();
};