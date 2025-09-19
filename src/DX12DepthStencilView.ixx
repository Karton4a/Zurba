module;
#include <memory>
#include <d3d12.h>
export module DX12DepthStencilView;
import DX12PixelStorage;
import DX12DeviceManager;
import DX12ResourceView;
export class DX12DepthStencilView : public DX12ResourceView
{
public:
	DX12DepthStencilView(ID3D12Resource* aResource);
	~DX12DepthStencilView();
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const override;
};
