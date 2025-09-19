module;
#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include <memory>

export module DX12DeviceManager;

import DX12DescriptorHeap;

export class DX12DeviceManager
{
public:
	DX12DeviceManager() = default;
	DX12DeviceManager(DX12DeviceManager&) = delete;
	DX12DeviceManager(DX12DeviceManager&&) = delete;
	void operator=(const DX12DeviceManager&) = delete;

	void SetDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { m_Device = device; }
	void SetShaderVisibleDescriptorHeap(std::unique_ptr<DX12DescriptorHeap>&& aHeap) { m_SVHeap = std::move(aHeap); }
	std::unique_ptr<DX12DescriptorHeap>& GetShaderVisibleDescriptorHeap() { return m_SVHeap; }

	void SetRenderTargetDescriptorHeap(std::unique_ptr<DX12DescriptorHeap>&& aHeap) { m_RTHeap = std::move(aHeap); }
	std::unique_ptr<DX12DescriptorHeap>& GetRenderTargetDescriptorHeap() { return m_RTHeap; }

	void SetDepthStencilDescriptorHeap(std::unique_ptr<DX12DescriptorHeap>&& aHeap) { m_DSHeap = std::move(aHeap); }
	std::unique_ptr<DX12DescriptorHeap>& GetDepthStencilDescriptorHeap() { return m_DSHeap; }

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return m_Device; };

	static void Initialize();
	static void Free();
	static DX12DeviceManager* GetInstance();
private:
	static DX12DeviceManager* s_Instance;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	std::unique_ptr<DX12DescriptorHeap> m_SVHeap;
	std::unique_ptr<DX12DescriptorHeap> m_RTHeap;
	std::unique_ptr<DX12DescriptorHeap> m_DSHeap;
};