#pragma once
#include <wtypes.h>
#include <exception>
#include <wrl.h>
#include <d3d12.h>
#include<dxgi1_6.h>
#include <array>
#include <directxmath.h>

class DX12Context
{
public:
	DX12Context(UINT32 width, UINT32 height)
		:m_WindowHandler(0), 
		m_WindowSize(width, height),
		m_FrameIndex(0), 
		m_FenceValue(0), 
		m_FenceEvent(0),
		m_RTVDescriptorSize(0),
		m_VertexBufferView()
	{}

	void Init(HWND hwnd);
	void Update();
	void Render();
	void Destroy();
private:
	void LoadPipeline();
	void LoadAssets();
	void WaitForPreviousFrame();
	void WriteCommandList();
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
	struct __declspec(align(256)) Constants
	{
		DirectX::XMMATRIX MVP;
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBVHeap;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;
	Constants m_CBData;
	UINT8* m_pCbvDataBegin;
	UINT m_RTVDescriptorSize;

	UINT64 m_FrameIndex;
	HANDLE m_FenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> m_RenderTargets;
	HWND m_WindowHandler;
	DirectX::XMUINT2 m_WindowSize;
};

