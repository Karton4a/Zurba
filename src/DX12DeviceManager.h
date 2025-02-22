#pragma once
#include <d3d12.h>
#include <wrl.h>
class DX12DeviceManager
{
public:
	DX12DeviceManager() = default;
	DX12DeviceManager(DX12DeviceManager&) = delete;
	DX12DeviceManager(DX12DeviceManager&&) = delete;
	void operator=(const DX12DeviceManager&) = delete;

	void SetDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { m_Device = device; }
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return m_Device; };
	static void Initialize();
	static void Free();
	static DX12DeviceManager* GetInstance();
private:
	static DX12DeviceManager* s_Instance;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
};

