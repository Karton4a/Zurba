module;
#include <cassert>

module DX12DeviceManager;

DX12DeviceManager* DX12DeviceManager::s_Instance = nullptr;

void DX12DeviceManager::Initialize()
{
	assert(!s_Instance);
	s_Instance = new DX12DeviceManager;
}

void DX12DeviceManager::Free()
{
	delete s_Instance;
}

DX12DeviceManager* DX12DeviceManager::GetInstance()
{
	assert(s_Instance);
	return s_Instance;
}