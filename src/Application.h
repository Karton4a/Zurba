#pragma once
#include <wtypes.h>
#include <exception>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <array>
#include <directxmath.h>

#include <vector>
#include <memory>
#include "DX12Buffer.h"
#include "Input.h"
#include "Camera.h"

class Application
{
public:
	Application(UINT32 width, UINT32 height)
		:m_WindowHandler(0), 
		m_WindowSize(width, height),
		m_FrameIndex(0), 
		m_FenceValue(0), 
		m_FenceEvent(0),
		m_RTVDescriptorSize(0),
		m_VertexBufferView(),
		m_Camera(DirectX::XMFLOAT3(0,100,0), 30, (float)width / height, 5000.0f, 0.01f)
	{}
	~Application();
	void Init(HWND hwnd);
	void Update(float dt);
	void Render();
	void Destroy();
	void OnKeyUp(UINT8 key);
	void OnKeyDown(UINT8 key);
	void OnMouseDown(Input::MouseKey key);
	void OnMouseUp(Input::MouseKey key);
	void SetMousePosition(POINTS point);
	std::vector<UINT8> GenerateTextureData();
private:
	void LoadPipeline();
	void LoadAssets();
	void WaitForPreviousFrame();
	void WriteCommandList();
	template <UINT MaxSubresources>
	UINT64 UpdateSubresources(
		ID3D12GraphicsCommandList* pCmdList, 
		ID3D12Resource* pDestinationResource, 
		ID3D12Resource* pIntermediate,
		UINT64 IntermediateOffset, 
		UINT FirstSubresource, 
		UINT NumSubresources, 
		const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept;

	UINT64 UpdateSubresources(
		ID3D12GraphicsCommandList* pCmdList,
		ID3D12Resource* pDestinationResource,
		ID3D12Resource* pIntermediate,
		UINT FirstSubresource,
		UINT NumSubresources,
		UINT64 RequiredSize,
		const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
		const UINT* pNumRows,
		const UINT64* pRowSizesInBytes,
		const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept;

	void MemcpySubresource(
		const D3D12_MEMCPY_DEST* pDest,
		const D3D12_SUBRESOURCE_DATA* pSrc,
		SIZE_T RowSizeInBytes,
		UINT NumRows,
		UINT NumSlices) noexcept;

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
	std::shared_ptr<DX12Buffer> m_DX12VertexBuffer;
	std::shared_ptr<DX12Buffer> m_SponzaVertexBuffer;
	uint32_t m_SponzaVertexCount;
	D3D12_VERTEX_BUFFER_VIEW m_SponzaView;
	Constants m_CBData;
	UINT8* m_pCbvDataBegin;
	UINT m_RTVDescriptorSize;

	UINT64 m_FrameIndex;
	HANDLE m_FenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> m_RenderTargets;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	HWND m_WindowHandler;
	DirectX::XMUINT2 m_WindowSize;
	Camera m_Camera;
	float m_CameraSpeed = 100.0f;
	float m_CameraRotaionSpeed = 10.0f;
	float m_CameraAcceleration = 10.0f;
	Input m_Input;
};

template<UINT MaxSubresources>
inline UINT64 Application::UpdateSubresources(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept
{
	UINT64 RequiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[MaxSubresources];
	UINT NumRows[MaxSubresources];
	UINT64 RowSizesInBytes[MaxSubresources];

#if defined(_MSC_VER) || !defined(_WIN32)
	const auto Desc = pDestinationResource->GetDesc();
#else
	D3D12_RESOURCE_DESC tmpDesc;
	const auto& Desc = *pDestinationResource->GetDesc(&tmpDesc);
#endif
	ID3D12Device* pDevice = nullptr;
	pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
	pDevice->Release();

	return UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, Layouts, NumRows, RowSizesInBytes, pSrcData);
}
