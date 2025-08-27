module;
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

export module DX12DeviceManager;

export class DX12DeviceManager
{
public:
	DX12DeviceManager() = default;
	DX12DeviceManager(DX12DeviceManager&) = delete;
	DX12DeviceManager(DX12DeviceManager&&) = delete;
	void operator=(const DX12DeviceManager&) = delete;

	void SetDevice(Microsoft::WRL::ComPtr<ID3D12Device> device) { m_Device = device; }
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return m_Device; };
	static void Initialize()
	{
		assert(!s_Instance);
		s_Instance = new DX12DeviceManager;
	}

	static void Free()
	{
		delete s_Instance;
	}

	static DX12DeviceManager* GetInstance()
	{
		assert(s_Instance);
		return s_Instance;
	}
private:
	static DX12DeviceManager* s_Instance;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
};
DX12DeviceManager* DX12DeviceManager::s_Instance = nullptr;