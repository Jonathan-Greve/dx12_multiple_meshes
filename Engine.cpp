#include "Engine.h"
#include <wtypes.h>
#include <Windows.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>
#include "ResourceManager.h"
#include "d3dUtility.h"
#include "MeshGenerator.h"


using namespace Microsoft::WRL;

Engine::Engine(WindowManager& windowManager, ICamera& camera, InputManager& inputManager) :
	m_windowManager{ windowManager },
	m_camera{ camera },
	m_inputManager{ inputManager },
	m_scissorRect{ 0, 0, static_cast<LONG>(windowManager.GetWidth()), static_cast<LONG>(windowManager.GetHeight()) },
	m_viewport{ 0.0f, 0.0f, static_cast<float>(windowManager.GetWidth()), static_cast<float>(windowManager.GetHeight()) , 0.0f, 1.0f },
	m_dxgiFactoryFlags{ 0 },
	m_rtvDescriptorSize{ 0 },
	m_dsvDescriptorSize{ 0 },
	m_cbvDescriptorSize{ 0 },
	m_frameIndex{ 0 }
{
}

void Engine::Initialize()
{
	m_windowManager.CreateMainWindow();
	InitializeD3D12();

	// Set up the resource manager
	m_resourceManager = ResourceManager(m_commandList.Get(), m_device.Get());

	// Initialize system (CPU) timer
	SystemTime().Initialize();
	m_cpuTimer = CpuTimer();
	m_cpuTimer.Reset();

	// Prepare geometry
	BuildConstantBuffers();
	BuildGeometry();
	BuildRootSignatures();
	BuildShadersAndInputLayouts();
	BuildPSO();

	// Setup the camera and input
	m_inputManager.CaptureMouseInputs();

	m_camera.SetFrustum(0.25f * 3.14f, (float)m_windowManager.GetWidth() / m_windowManager.GetHeight(), 1.0f, 1000);
	m_camera.LookAt(
		DirectX::XMVectorSet(5.0f, -5.0f, -5.0f, 1.0f),
		DirectX::XMVectorZero(),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.f, 0.0f));

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	WaitForPreviousFrame();
}

void Engine::Update()
{
	// Update time
	m_cpuTimer.Stop();
	double currTime = m_cpuTimer.GetTime();
	double deltaTime = currTime - m_currTime;
	m_currTime = currTime;
	m_cpuTimer.Start();

	//std::string dts = "Current time is: " + std::to_string(deltaTime*1000) + "ms" + "\n";
	//::OutputDebugStringA(dts.c_str());

	// Update camera
	m_inputManager.OnKeyboardInput(deltaTime, m_camera);
	auto xs = std::to_string(m_camera.GetPosition3f().x);
	auto ys = std::to_string(m_camera.GetPosition3f().y);
	auto zs = std::to_string(m_camera.GetPosition3f().z) + "\n";

	auto ps = xs + " | " + ys + " | " + zs;

	//::OutputDebugStringA(ps.c_str());

	auto dt = "DeltaTime: " + std::to_string(deltaTime) + "\n";
	auto ct = "CurrTime: " + std::to_string(m_currTime) + "\n";
	//::OutputDebugStringA(dt.c_str());
	::OutputDebugStringA(ct.c_str());

	m_camera.UpdateViewMatrix();

	auto proj = m_camera.GetProj();
	auto view = m_camera.GetView();
	auto viewProj = XMMatrixMultiply(view, proj);


	PassConstants passConstants;
	passConstants.DeltaTime = deltaTime;
	passConstants.TotalTime = currTime;
	DirectX::XMStoreFloat4x4(&passConstants.ViewProj, XMMatrixTranspose(viewProj));
	m_resourceManager.UpdateConstantBuffer<PassConstants>("PassConstants", 0, passConstants);

	// Update geometry
}

void Engine::Render()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_PSO.Get()));

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Change current front buffer to be the render target/back buffer
	auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChainRenderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &resourceBarrier);


	// Clear the back buffer and depth buffer.
	auto renderTargetView = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize);
	m_commandList->ClearRenderTargetView(renderTargetView,
		DirectX::Colors::LightSteelBlue, 0, nullptr);

	auto depthStencilView = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
		0,
		m_dsvDescriptorSize);
	m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_commandList->OMSetRenderTargets(1, &renderTargetView, true, &depthStencilView);

	// Draw box
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->SetGraphicsRootDescriptorTable(1, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	auto meshes = m_resourceManager.GetAllMeshes();
	for (auto meshPair : meshes) {
		auto mesh = meshPair.second;

		auto vertexBufferView = mesh.VertexBufferView();
		auto indexBufferView = mesh.IndexBufferView();

		m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_commandList->IASetIndexBuffer(&indexBufferView);
		m_commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(mesh.cbPerObjectIndex+1, m_cbvDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(0, cbvHandle);

		m_commandList->DrawIndexedInstanced(
			mesh.DrawArgs[mesh.Name].IndexCount,
			1, 0, 0, 0);
	}

	// Indicate a state transition on the resource usage.
	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChainRenderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &resourceBarrier);

	// Done recording commands.
	ThrowIfFailed(m_commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(m_swapChain->Present(0, 0));
	WaitForPreviousFrame();
	m_frameIndex = (m_frameIndex + 1) % m_frameCount;
}

void Engine::Destroy()
{
}

void Engine::InitializeD3D12()
{

#if defined(_DEBUG) 
	EnableDebugLayer();
#endif
	BuildDXGIFactory();
	Build3DDevice();
	BuildSyncObjects();
	BuildCommandObjects();
	BuildDescriptorHeaps();
	SetDescriptorSizes();
	BuildSwapChain();
}

void Engine::EnableDebugLayer()
{
	// Enable the D3D12 debug layer.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		// Enable additional debug layers.
		m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
}

void Engine::BuildDXGIFactory()
{
	ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
}

void Engine::Build3DDevice()
{
	// Because we pass nullptr the first hardware adapter found by factory::EnumAdapters is used
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
}

void Engine::BuildSyncObjects()
{
	// Create a fence to help synchronize the CPU and GPU
	ThrowIfFailed(m_device->CreateFence(
		0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
}

void Engine::BuildCommandObjects()
{
	// DX12 command are submitted to a CommandQueue through CommandLists.
	// A CommandList has an associated  CommandAllocator which stores the commands.
	// The CommandList can be thought of as a list of pointers to the actual commands in the CommandAllocator.
	// Once the CommandList has been submitted to the CommandQueue it can be Reset. But the CommandAllocator
	// can not be reset until after the GPU has finished executing the commands contained in it.

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	ThrowIfFailed(m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));

	// Notice how we associate a CommandAllocator with the CommandList.
	ThrowIfFailed(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(m_commandList.GetAddressOf())));
}

void Engine::BuildDescriptorHeaps()
{
	// Build a heap for storing our 2 render target views
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_frameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

	// Create a heap for storing the depth-stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

	// Create a heap for storing the constant buffer views
	// 1 per pass descriptor and 2 descriptors for, one for each box.
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 4;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(
		&cbvHeapDesc,
		IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));
}

void Engine::SetDescriptorSizes()
{
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_cbvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Engine::BuildSwapChain()
{
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_frameCount;
	swapChainDesc.Width = m_windowManager.GetWidth();
	swapChainDesc.Height = m_windowManager.GetHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_windowManager.GetMainWindow(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// The CreateSwapChain method required an IDXGISwapChain1 object but we need a IDXGISwapChain3 object
	// to use the GetCurrentBackBufferIndex function
	ThrowIfFailed(swapChain.As(&m_swapChain));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };

	// Create a RTV for each frame in the swapchain.
	for (UINT n = 0; n < m_frameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_swapChainRenderTargets[n])));
		m_device->CreateRenderTargetView(m_swapChainRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	// Create the DSV view and resource

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	auto defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto heapResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, m_windowManager.GetWidth(), m_windowManager.GetHeight(), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&heapResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_dsBuffer)
	));

	m_device->CreateDepthStencilView(m_dsBuffer.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Engine::BuildConstantBuffers()
{
	m_resourceManager.AddConstantBuffer("PassConstants", sizeof(MeshConstants), 1, m_cbvHeap.Get(), m_cbvDescriptorSize);
	m_resourceManager.AddConstantBuffer("MeshConstants", sizeof(MeshConstants), 3, m_cbvHeap.Get(), m_cbvDescriptorSize);
}

void Engine::BuildGeometry()
{
	Mesh unitBox1 = MeshGenerator().GenerateUnitBox("unitBox1");
	unitBox1.cbPerObjectIndex = 0;

	auto unitBox2World = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(2.0f, 2.0f, 2.0f));
	Mesh unitBox2 = MeshGenerator().GenerateUnitBox("unitBox2");
	unitBox2.cbPerObjectIndex = 1;
	DirectX::XMStoreFloat4x4(&unitBox2.World, unitBox2World);

	auto unitBox3World = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(-2.0f, -2.0f, -2.0f));
	Mesh unitBox3 = MeshGenerator().GenerateUnitBox("unitBox3");
	unitBox3.cbPerObjectIndex = 2;
	DirectX::XMStoreFloat4x4(&unitBox3.World, unitBox3World);

	Mesh grid = MeshGenerator().GenerateGrid("grid", 20, 25);
	grid.cbPerObjectIndex = 2;

	m_resourceManager.AddMesh(unitBox1);
	m_resourceManager.AddMesh(unitBox2);
	//m_resourceManager.AddMesh(unitBox3);
	m_resourceManager.AddMesh(grid);

	MeshConstants meshConstants;
	meshConstants.World = unitBox1.World;
	m_resourceManager.UpdateConstantBuffer<MeshConstants>("MeshConstants", 0, meshConstants);

	meshConstants.World = unitBox2.World;
	m_resourceManager.UpdateConstantBuffer<MeshConstants>("MeshConstants", 1, meshConstants);

	//meshConstants.World = unitBox3.World;
	//m_resourceManager.UpdateConstantBuffer<MeshConstants>("MeshConstants", 2, meshConstants);

	meshConstants.World = grid.World;
	m_resourceManager.UpdateConstantBuffer<MeshConstants>("MeshConstants", 2, meshConstants);
}

void Engine::BuildRootSignatures()
{
	//For now we only need to bind a ConstantBuffer which will store the projection matrix.
	//Create a root signature consisting of a descriptor table with a single CBV.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];

	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void Engine::BuildShadersAndInputLayouts()
{
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &m_vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &m_pixelShader, nullptr));

	// Define the vertex input layout.
	m_inputLayout =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void Engine::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	auto rasterizerStateDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerStateDesc.FrontCounterClockwise = false;
	rasterizerStateDesc.FillMode = D3D12_FILL_MODE_SOLID;

	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(m_vertexShader->GetBufferPointer()), m_vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(m_pixelShader->GetBufferPointer()), m_pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = rasterizerStateDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.StencilEnable = TRUE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO)));

}

void Engine::WaitForPreviousFrame() {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
// This is code implemented as such for simplicity. More advanced samples 
// illustrate how to use fences for efficient resource usage.

	m_fenceValue++;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));

	auto completedValue = m_fence->GetCompletedValue();
	// Wait until the previous frame is finished.
	if (completedValue < m_fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}


