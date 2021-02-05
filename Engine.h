#ifndef ENGINE_H_ 
#define ENGINE_H_

#include "EventManager.h"
#include "WindowManager.h"
#include "ResourceManager.h"

#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include "SystemTime.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "CameraManager.h"
#include "ICamera.h"
#include <memory>
#include "InputManager.h"

using Microsoft::WRL::ComPtr;

class Engine
{
public:
	Engine(WindowManager& windowManager, ICamera& camera, InputManager& inputManager);

	void Initialize();
	void Update();
	void Render();
	void Destroy();

private:
	static const UINT m_frameCount{ 2 };
	CpuTimer m_cpuTimer;
	double m_currTime{ 0 };

	WindowManager& m_windowManager;
	InputManager& m_inputManager;

	ResourceManager m_resourceManager;
	ICamera& m_camera;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	UINT m_dxgiFactoryFlags;
	ComPtr<IDXGIFactory4> m_factory;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue = 0;
	ComPtr<IDXGISwapChain3> m_swapChain;
	UINT m_frameIndex;
	ComPtr<ID3D12Resource> m_swapChainRenderTargets[m_frameCount];
	ComPtr<ID3D12Resource> m_dsBuffer;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_dsvDescriptorSize;
	UINT m_cbvDescriptorSize;

	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> m_PSO = nullptr;


	// Initialization functions
	void InitializeD3D12();
	void EnableDebugLayer();
	void BuildDXGIFactory();
	void Build3DDevice();
	void BuildSyncObjects();
	void BuildCommandObjects();
	void BuildDescriptorHeaps();
	void SetDescriptorSizes();
	void BuildSwapChain();

	void BuildConstantBuffers();
	void BuildGeometry();
	void BuildRootSignatures();
	void BuildShadersAndInputLayouts();
	void BuildPSO();
	void WaitForPreviousFrame();
};

#endif
