module;
#include <memory>
#include <d3d12.h>
export module DX12ShaderResourceView;
import DX12PixelStorage;
import DX12DeviceManager;
import DX12ResourceView;
import DX12DescriptorHeap;
export class DX12ShaderResourceView : public DX12ResourceView
{
public:
	DX12ShaderResourceView(std::shared_ptr<DX12PixelStorage>& aPixelStorage, D3D12_SRV_DIMENSION aDimension, D3D12_SHADER_RESOURCE_VIEW_DESC& aDesc);
	~DX12ShaderResourceView();
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const override;
};
