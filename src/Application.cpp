#include "Application.h"
#include "DXHelpers.h"
#include "d3dcompiler.h"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "DX12DeviceManager.h"
#include <format>

Application::~Application()
{
    DX12DeviceManager::Free();
}

void Application::Init(HWND hwnd)
{
    DX12DeviceManager::Initialize();
    m_WindowHandler = hwnd;
	LoadPipeline();
	LoadAssets();
}

void Application::Update(float dt)
{
    m_CameraMovementDirection = DirectX::XMFLOAT3(0,0,0);
    float cameraspeed = m_CameraSpeed;
    if (m_Input.IsKeyPressed(VK_SHIFT))
    {
        cameraspeed *= m_CameraAcceleration;
    }
    if (m_Input.IsKeyPressed('W')) 
    {
        m_CameraMovementDirection.z += cameraspeed * dt;
    }
    if (m_Input.IsKeyPressed('S'))
    {
        m_CameraMovementDirection.z -= cameraspeed * dt;
    }
    if (m_Input.IsKeyPressed('D'))
    {
        m_CameraMovementDirection.x += cameraspeed * dt;
    }
    if (m_Input.IsKeyPressed('A'))
    {
        m_CameraMovementDirection.x -= cameraspeed * dt;
    }
    if (m_Input.IsKeyPressed(VK_SPACE))
    {
        m_CameraMovementDirection.y += cameraspeed * dt;
    }
    if (m_Input.IsKeyPressed(VK_CONTROL))
    {
        m_CameraMovementDirection.y -= cameraspeed * dt;
    }

    DirectX::XMVECTOR movementDirection = DirectX::XMLoadFloat3(&m_CameraMovementDirection);
    m_CameraMovementPosition = DirectX::XMVectorAdd(m_CameraMovementPosition, movementDirection);

    DirectX::XMMATRIX model =  DirectX::XMMatrixTranslation(0,0,0) * DirectX::XMMatrixRotationZ(0);
    if (m_Input.IsMouseKeyPressed(Input::MouseKey::LeftKey))
    {
        auto mouseDelta = m_Input.GetMouseDelta();
        float coff = 0.01f;
        auto rotation = DirectX::XMQuaternionRotationRollPitchYaw(coff * mouseDelta.y,coff * mouseDelta.x,0);
        m_CameraViewDirection = DirectX::XMVector3Rotate(m_CameraViewDirection, rotation);
    }
    DirectX::XMVECTOR focusPosition = DirectX::XMVectorAdd(m_CameraMovementPosition, m_CameraViewDirection);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(m_CameraMovementPosition, focusPosition, DirectX::XMVectorSet(0, 1 , 0 , 0));

    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30), (float)m_WindowSize.x / m_WindowSize.y, 5000.0f, 0.0000001f);
    m_CBData.MVP = model * view * projection;

    memcpy(m_pCbvDataBegin, &m_CBData, sizeof(m_CBData));


}

void Application::Render()
{
    // Record all the commands we need to render the scene into the command list.
    WriteCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_SwapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void Application::OnKeyUp(UINT8 key)
{
    m_Input.RegisterKeyUp(key);
}

void Application::OnKeyDown(UINT8 key)
{
    m_Input.RegisterKeyDown(key);
}

void Application::OnMouseDown(Input::MouseKey key)
{
    m_Input.RegisterMouseDown(key);
}

void Application::OnMouseUp(Input::MouseKey key)
{
    m_Input.RegisterMouseUp(key);
}

void Application::SetMousePosition(POINTS point)
{
    m_Input.SetMousePosition(point.x, point.y);
}

std::vector<UINT8> Application::GenerateTextureData()
{
    const UINT rowPitch = 256 * sizeof(UINT8) * 4;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = 256 >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * 256;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += sizeof(UINT8) * 4)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

void Application::LoadPipeline()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();

   /* Microsoft::WRL::ComPtr<ID3D12Debug> spDebugController0;
    Microsoft::WRL::ComPtr<ID3D12Debug1> spDebugController1;
    (D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
    (spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
    spDebugController1->SetEnableGPUBasedValidation(true);*/
#endif


    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    SIZE_T maxDedicatedVideoMemory = 0;
    for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
        dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

        // Check to see if the adapter can create a D3D12 device without actually 
        // creating it. The adapter with the largest dedicated video memory
        // is favored.
        if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
            dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
        {
            maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
            FormatOutputDebug(L"{}\n", dxgiAdapterDesc1.Description);
            ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
        }
    }

    //DEVICE
    ThrowIfFailed(D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
    DX12DeviceManager::GetInstance()->SetDevice(m_Device);

#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(m_Device.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    //COMMAND QUEQUE 

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

    ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));

    //COMMAND LIST
    ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    //ThrowIfFailed(m_CommandList->Close());

    D3D12_VIEWPORT viewport;
    D3D12_RECT surfaceSize;

    surfaceSize.left = 0;
    surfaceSize.top = 0;
    surfaceSize.right = static_cast<LONG>(m_WindowSize.x);
    surfaceSize.bottom = static_cast<LONG>(m_WindowSize.y);

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_WindowSize.x);
    viewport.Height = static_cast<float>(m_WindowSize.y);
    viewport.MinDepth = .1f;
    viewport.MaxDepth = 5000.f;


    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.BufferCount = m_RenderTargets.size();
    swapchainDesc.Width = m_WindowSize.x;
    swapchainDesc.Height = m_WindowSize.y;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;

    ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_WindowHandler, &swapchainDesc, nullptr, nullptr, &swapchain));

    ThrowIfFailed(swapchain.As(&m_SwapChain));
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = m_RenderTargets.size();
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_Device->CreateDescriptorHeap(
            &rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap)));

        m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    for (UINT n = 0; n < m_RenderTargets.size(); n++)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
        m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += (1 * m_RTVDescriptorSize);

    }
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 2;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_Device->CreateDescriptorHeap(
            &cbvHeapDesc, IID_PPV_ARGS(&m_CBVHeap)));

    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.NumDescriptors = 1;
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_Device->CreateDescriptorHeap(
            &samplerHeapDesc, IID_PPV_ARGS(&m_SamplerHeap)));

    }
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_Device->CreateDescriptorHeap(
            &dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));
    }
}

void Application::LoadAssets()
{
    
    {
        D3D12_ROOT_SIGNATURE_DESC  rootSignatureDesc;
        rootSignatureDesc.NumParameters = 1;
        D3D12_ROOT_PARAMETER rootParameterCB;

        rootParameterCB.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameterCB.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_DESCRIPTOR_RANGE range;
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range.NumDescriptors = 1;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = 0;


        D3D12_DESCRIPTOR_RANGE rangeTexture;
        rangeTexture.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        rangeTexture.NumDescriptors = 1;
        rangeTexture.BaseShaderRegister = 0;
        rangeTexture.RegisterSpace = 0;
        rangeTexture.OffsetInDescriptorsFromTableStart = 1;

        D3D12_DESCRIPTOR_RANGE ranges[] = { range, rangeTexture };

        rootParameterCB.DescriptorTable.NumDescriptorRanges = 2;
        rootParameterCB.DescriptorTable.pDescriptorRanges = ranges;

        D3D12_ROOT_PARAMETER params[] = { rootParameterCB};

        rootSignatureDesc.pParameters = params;

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.pStaticSamplers = &sampler;

        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

        Microsoft::WRL::ComPtr<ID3DBlob> shaderError;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        (D3DCompileFromFile(L"./data/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &shaderError));
        if (shaderError != nullptr)
        {
            //static_cast<const char*>(shaderError->GetBufferPointer())
            FormatOutputDebugA("Vertex shader error: {}", static_cast<const char*>(shaderError->GetBufferPointer()));
        }

        (D3DCompileFromFile(L"./data/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &shaderError));
        if (shaderError != nullptr)
        {
            //static_cast<const char*>(shaderError->GetBufferPointer())
            FormatOutputDebugA("Pixel shader error: {}", static_cast<const char*>(shaderError->GetBufferPointer()));
        }
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        D3D12_SHADER_BYTECODE vertexByteCode;
        vertexByteCode.pShaderBytecode = vertexShader->GetBufferPointer();
        vertexByteCode.BytecodeLength = vertexShader->GetBufferSize();

        D3D12_SHADER_BYTECODE pixelByteCode;
        pixelByteCode.pShaderBytecode = pixelShader->GetBufferPointer();
        pixelByteCode.BytecodeLength = pixelShader->GetBufferSize();

        D3D12_RASTERIZER_DESC rasterizerDesc {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC blendDesc {};

        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = FALSE;
        blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_RootSignature.Get();
        psoDesc.VS = vertexByteCode;
        psoDesc.PS = pixelByteCode;
        psoDesc.RasterizerState = rasterizerDesc;
        psoDesc.BlendState = blendDesc;

        D3D12_DEPTH_STENCIL_DESC dsDesc;
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
       
        dsDesc.StencilEnable = FALSE;
       /* dsDesc.StencilReadMask = ~0;
        dsDesc.StencilWriteMask = ~0;
        dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
        dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

        dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
        dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;*/

        psoDesc.DepthStencilState = dsDesc;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
    }

    {
        fastObjMesh* mesh =  fast_obj_read("./data/Sponza/sponza.obj");
        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT3 normals;
            DirectX::XMFLOAT2 uv;
        };
        std::vector<Vertex> vertices;

        uint32_t readIndexes = 0;
        for (size_t i = 0; i < mesh->face_count; i++)
        {
            uint32_t vertexsPerFace = mesh->face_vertices[i];
            for (size_t j = 0; j < vertexsPerFace; j++)
            {
                fastObjIndex index = mesh->indices[readIndexes + j];
                Vertex vertex;
                vertex.position = DirectX::XMFLOAT3(mesh->positions + 3 * index.p);
                vertex.normals = DirectX::XMFLOAT3(mesh->normals + 3 * index.n);
                vertex.uv = DirectX::XMFLOAT2(mesh->texcoords + 2 * index.t);
                vertices.push_back(vertex);
            }
            readIndexes += vertexsPerFace;
        }
        m_SponzaVertexBuffer = std::make_shared<DX12Buffer>(vertices.data(), vertices.size() * sizeof(Vertex));
        m_SponzaVertexBuffer->Flush(m_CommandList.Get());



        m_SponzaView.BufferLocation = m_SponzaVertexBuffer->GetGPUAddress();
        m_SponzaView.StrideInBytes = sizeof(Vertex);
        m_SponzaView.SizeInBytes = vertices.size() * sizeof(Vertex);
        m_SponzaVertexCount = vertices.size();


        fast_obj_destroy(mesh);
    }

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { -300.0f, 300.0f , 1.0f }, { 0.0f,0.0f } },
            { { 300.0f, 300.0f, 1.0f }, { 1.0f, 0.0f } },
            { { 300.0f, -300.0f, 1.0f }, { 1.0f, 1.0f} },

            { { 300.0f, -300.0f , 1.0f }, { 1.0f, 1.0f } },
            { { -300.0f, -300.0f, 1.0f }, { 0.0f, 1.0f } },
            { { -300.0f, 300.0f, 1.0f }, { 0.0f, 0.0f} }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);
        //auto t = new DX12Buffer((void*)triangleVertices, vertexBufferSize);
        m_DX12VertexBuffer = std::make_shared<DX12Buffer>(triangleVertices, vertexBufferSize);
        //D3D12_HEAP_PROPERTIES heapProps;
        //heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        //heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        //heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        //heapProps.CreationNodeMask = 1;
        //heapProps.VisibleNodeMask = 1;


        //D3D12_RESOURCE_DESC vertexBufferResourceDesc;
        //vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        //vertexBufferResourceDesc.Alignment = 0;
        //vertexBufferResourceDesc.Width = vertexBufferSize;
        //vertexBufferResourceDesc.Height = 1;
        //vertexBufferResourceDesc.DepthOrArraySize = 1;
        //vertexBufferResourceDesc.MipLevels = 1;
        //vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        //vertexBufferResourceDesc.SampleDesc.Count = 1;
        //vertexBufferResourceDesc.SampleDesc.Quality = 0;
        //vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        //vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        //// Note: using upload heaps to transfer static data like vert buffers is not 
        //// recommended. Every time the GPU needs it, the upload heap will be marshalled 
        //// over. Please read up on Default Heap usage. An upload heap is used here for 
        //// code simplicity and because there are very few verts to actually transfer.
        //ThrowIfFailed(m_Device->CreateCommittedResource(
        //    &heapProps,
        //    D3D12_HEAP_FLAG_NONE,
        //    &vertexBufferResourceDesc,
        //    D3D12_RESOURCE_STATE_GENERIC_READ,
        //    nullptr,
        //    IID_PPV_ARGS(&m_VertexBuffer)));

        //// Copy the triangle data to the vertex buffer.
        //UINT8* pVertexDataBegin;
        //D3D12_RANGE readRange;        // We do not intend to read from this resource on the CPU.
        //readRange.Begin = 0;
        //readRange.End = 0;
        //ThrowIfFailed(m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        //memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        //m_VertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_VertexBufferView.BufferLocation = m_DX12VertexBuffer->GetGPUAddress();
        m_VertexBufferView.StrideInBytes = sizeof(Vertex);
        m_VertexBufferView.SizeInBytes = vertexBufferSize;

        m_DX12VertexBuffer->Flush(m_CommandList.Get());
    }

    // Create the constant buffer.
    {
        const UINT constantBufferSize = sizeof(Constants);    // CB size is required to be 256-byte aligned.

        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;


        D3D12_RESOURCE_DESC constantBufferResourceDesc;
        constantBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        constantBufferResourceDesc.Alignment = 0;
        constantBufferResourceDesc.Width = constantBufferSize;
        constantBufferResourceDesc.Height = 1;
        constantBufferResourceDesc.DepthOrArraySize = 1;
        constantBufferResourceDesc.MipLevels = 1;
        constantBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        constantBufferResourceDesc.SampleDesc.Count = 1;
        constantBufferResourceDesc.SampleDesc.Quality = 0;
        constantBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        constantBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ThrowIfFailed(m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;

        D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_CBVHeap->GetCPUDescriptorHandleForHeapStart());
        m_Device->CreateConstantBufferView(&cbvDesc, cbvHandle);

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.

        D3D12_RANGE readRange;        // We do not intend to read from this resource on the CPU.
        readRange.Begin = 0;
        readRange.End = 0;

        ThrowIfFailed(m_ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
        memcpy(m_pCbvDataBegin, &m_CBData, sizeof(m_CBData));
    }
    
    Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;

    // Create the texture.
    {
        int x, y, n;
        stbi_uc* data = stbi_load("./data/images/Flag.png", &x, &y, &n, 0);
        /*for (size_t i = 0; i < x*y*n; i++)
        {
            stbi_uc value = data[i];
            OutputDebugString(value == 0 ? L"1" : L"0");
        }*/

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = x;
        textureDesc.Height = y;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;


        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        ThrowIfFailed(m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_Texture)));

        //GetRequiredIntermediateSize 
        const UINT64 uploadBufferSize = x * y * sizeof(UINT8) * n + 100000;
        
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadBufferResourceDesc;
        uploadBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadBufferResourceDesc.Alignment = 0;
        uploadBufferResourceDesc.Width = uploadBufferSize;
        uploadBufferResourceDesc.Height = 1;
        uploadBufferResourceDesc.DepthOrArraySize = 1;
        uploadBufferResourceDesc.MipLevels = 1;
        uploadBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        uploadBufferResourceDesc.SampleDesc.Count = 1;
        uploadBufferResourceDesc.SampleDesc.Quality = 0;
        uploadBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        uploadBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Create the GPU upload buffer.
        ThrowIfFailed(m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.

        //std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = data;
        textureData.RowPitch = x * n * sizeof(stbi_uc);
        textureData.SlicePitch = textureData.RowPitch * y;

        //m_CommandList->CopyTextureRegion()
        //m_CommandList->CopyResource(m_Texture.Get(), textureUploadHeap.Get()); //?
       // m_CommandList->Copy
        UINT64 res =  UpdateSubresources<5>(m_CommandList.Get(), m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        if (res == 0)
        {
            OutputDebugString(L"UpdateSubresource error\n");
        }

        stbi_image_free(data);


        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.pResource = m_Texture.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        m_CommandList->ResourceBarrier(1, &barrier);

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        

        D3D12_CPU_DESCRIPTOR_HANDLE handle(m_CBVHeap->GetCPUDescriptorHandleForHeapStart());
        UINT descriptorRecordSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        handle.ptr += descriptorRecordSize;

        m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, handle);
    }
    //Create Depth Buffer
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
        textureDesc.Width = m_WindowSize.x;
        textureDesc.Height = m_WindowSize.y;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;

        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil.Depth = 0.0f; // Default depth value.
        optimizedClearValue.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_DepthStencilBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvView;
        dsvView.Format = DXGI_FORMAT_D32_FLOAT;
        dsvView.Flags = D3D12_DSV_FLAG_NONE;
        dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvView.Texture2D.MipSlice = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

        m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvView, dsvHandle);
    }

    ThrowIfFailed(m_CommandList->Close());
    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
        m_FenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }

}

void Application::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
   // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
   // sample illustrates how to use fences for efficient resource usage and to
   // maximize GPU utilization.

   // Signal and increment the fence value.
    const UINT64 fence = m_FenceValue;
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fence));
    m_FenceValue++;

    // Wait until the previous frame is finished.
    if (m_Fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void Application::WriteCommandList()
{
    // Command list allocators can only be reset when the associated 
   // command lists have finished execution on the GPU; apps should use 
   // fences to determine GPU execution progress.
    ThrowIfFailed(m_CommandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

    // Set necessary state.
    m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Height = m_WindowSize.y;
    viewport.Width = m_WindowSize.x;
    viewport.MaxDepth = 5000.0f;
    viewport.MinDepth = 0.000001f;
    D3D12_RECT scissorRect;
    scissorRect.left = 0;
    scissorRect.right = m_WindowSize.x;
    scissorRect.top = 0;
    scissorRect.bottom = m_WindowSize.y;

    m_CommandList->RSSetViewports(1, &viewport);
    m_CommandList->RSSetScissorRects(1, &scissorRect);



    ID3D12DescriptorHeap* ppHeaps[] = { m_CBVHeap.Get()};
    m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_CommandList->SetGraphicsRootDescriptorTable(0, m_CBVHeap->GetGPUDescriptorHandleForHeapStart());

    // Indicate that the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.pResource = m_RenderTargets[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList->ResourceBarrier(1, &barrier);

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescriptorSize);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

    rtvHandle.ptr += m_FrameIndex * m_RTVDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE dsHandle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

    m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsHandle);

    // Record commands.
    const float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->ClearDepthStencilView(dsHandle, D3D12_CLEAR_FLAG_DEPTH ,0.0f,0,0,NULL);
    
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->IASetVertexBuffers(0, 1, &m_SponzaView);
    m_CommandList->DrawInstanced(m_SponzaVertexCount, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.pResource = m_RenderTargets[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    m_CommandList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(m_CommandList->Close());
}

UINT64 Application::UpdateSubresources(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate, UINT FirstSubresource, UINT NumSubresources, UINT64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, const UINT* pNumRows, const UINT64* pRowSizesInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept
{
    // Minor validation
#if defined(_MSC_VER) || !defined(_WIN32)
    const auto IntermediateDesc = pIntermediate->GetDesc();
    const auto DestinationDesc = pDestinationResource->GetDesc();
#else
    D3D12_RESOURCE_DESC tmpDesc1, tmpDesc2;
    const auto& IntermediateDesc = *pIntermediate->GetDesc(&tmpDesc1);
    const auto& DestinationDesc = *pDestinationResource->GetDesc(&tmpDesc2);
#endif
    if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
        IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
        RequiredSize > SIZE_T(-1) ||
        (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
            (FirstSubresource != 0 || NumSubresources != 1)))
    {
        return 0;
    }

    BYTE* pData;
    HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
    if (FAILED(hr))
    {
        return 0;
    }

    for (UINT i = 0; i < NumSubresources; ++i)
    {
        if (pRowSizesInBytes[i] > SIZE_T(-1)) return 0;
        D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, SIZE_T(pLayouts[i].Footprint.RowPitch) * SIZE_T(pNumRows[i]) };
        MemcpySubresource(&DestData, &pSrcData[i], static_cast<SIZE_T>(pRowSizesInBytes[i]), pNumRows[i], pLayouts[i].Footprint.Depth);
    }
    pIntermediate->Unmap(0, nullptr);

    if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        pCmdList->CopyBufferRegion(
            pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
    }
    else
    {
        for (UINT i = 0; i < NumSubresources; ++i)
        {
            
            D3D12_TEXTURE_COPY_LOCATION Dst;
            Dst.pResource = pDestinationResource;
            Dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            Dst.PlacedFootprint = {};
            Dst.SubresourceIndex = i + FirstSubresource;

            D3D12_TEXTURE_COPY_LOCATION Src;

            Src.pResource = pIntermediate;
            Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            Src.PlacedFootprint = pLayouts[i];

            pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        }
    }
    return RequiredSize;
}

void Application::MemcpySubresource(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSrc, SIZE_T RowSizeInBytes, UINT NumRows, UINT NumSlices) noexcept
{
    for (UINT z = 0; z < NumSlices; ++z)
    {
        auto pDestSlice = static_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
        auto pSrcSlice = static_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * LONG_PTR(z);
        for (UINT y = 0; y < NumRows; ++y)
        {
            memcpy(pDestSlice + pDest->RowPitch * y,
                pSrcSlice + pSrc->RowPitch * LONG_PTR(y),
                RowSizeInBytes);
        }
    }
}
