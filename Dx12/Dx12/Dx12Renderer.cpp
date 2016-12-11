#include "Dx12Renderer.h"
#include "Win32Application.h"
#include <D3DCompiler.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#pragma comment(lib, "d3dcompiler.lib")

Dx12Renderer::Dx12Renderer(int meshNum)
	: _viewport(),
	_scissorRect(),
	_swapChain(nullptr),
	_device(nullptr),
	_commandAllocator(nullptr),
	_commandQueue(nullptr),
	_rootSignature(nullptr),
	_rtvHeap(nullptr),
	_cbvHeap(nullptr),
	_dsvHeap(nullptr),
	_pipelineState(nullptr),
	_commandList(nullptr),
	_rtvDescriptorSize(0),
	_cbvDescriptorSize(0),
	_vertexBuffer(nullptr),
	_frameIndex(0),
	_fenceEvent(nullptr),
	_fence(nullptr),
	_fenceValue(0.0),
	_useWarpDevice(false),
	_meshNum(meshNum) {}

void Dx12Renderer::OnInit() {

	if(_windowHeight == 0 || _windowWidth == 0) {
		assert(0 && "width and height is not set!");
	}

	_renderTargets [FrameCount] = 0;

	_viewport = { 0.0f, 0.0f, static_cast<float>(_windowWidth), static_cast<float>(_windowHeight), 0.0f, 1.0f };
	_aspectRatio = static_cast<float>(_windowWidth) / static_cast<float>(_windowHeight);

	_scissorRect = { 0, 0, static_cast<LONG>(_viewport.Width), static_cast<LONG>(_viewport.Height) };

	LoadPipeline();
	LoadAssets();
}

void Dx12Renderer::OnUpdate() {
	CameraUpdate();
}

void Dx12Renderer::OnRender() {
	HRESULT result = S_OK;
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists [] = { _commandList };
	_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	result = _swapChain->Present(1, 0);
	if(result != S_OK) {
		assert(0 && "Present failed!");
		exit(-1);
	}


	WaitForPreviousFrame();
}

void Dx12Renderer::OnDestroy() {
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(_fenceEvent);
}

void Dx12Renderer::OnKeyDown(UINT8 key) {
	if(key == VK_UP) {
		KEYS [KEY_UP] = true;
	}
	if(key == VK_DOWN) {
		KEYS [KEY_DOWN] = true;
	}
}

void Dx12Renderer::OnKeyUp(UINT8 key) {
	if(key == VK_UP) {
		KEYS [KEY_UP] = false;
	}
	if(key == VK_DOWN) {
		KEYS [KEY_DOWN] = false;
	}
}

void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) {
	IDXGIAdapter1* adapter;
	*ppAdapter = nullptr;

	for(UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if(SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			break;
		}
	}

	*ppAdapter = adapter;
}

void Dx12Renderer::CameraUpdate() {
	bool changed = false;
	//check which key is pushed
	if(KEYS [KEY_UP]) {
		ZoomIn(0.1);
		changed = true;
	}
	if(KEYS [KEY_DOWN]) {
		ZoomOut(0.1);
		changed = true;
	}
	if(changed) {
		const XMVECTORF32 eye = { _cameraPositionX, _cameraPositionY, _cameraPositionZ, 0.0f };
		const XMVECTORF32 at = { _atPositionX, _atPositionY, _atPositionZ, 0.0f };
		const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&_constantBufferData.view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(eye, at, up)));

		for(int i = 0; i < _meshNum; i++) {

			XMStoreFloat4x4(&_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, _translateValues [i].X, _translateValues [i].Y)));

			UINT8* destination = _mappedConstantBuffer + i * c_alignedConstantBufferSize;
			memcpy(destination, &_constantBufferData, sizeof(_constantBufferData));
		}
	}
}

void Dx12Renderer::ZoomIn(float n) {
	_cameraPositionX += n;
}

void Dx12Renderer::ZoomOut(float n) {
	_cameraPositionX -= n;
}

void Dx12Renderer::LoadPipeline() {
	HRESULT result = S_OK;

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* debugController;
		if(SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**) &debugController))) {
			debugController->EnableDebugLayer();
		}
	}
#endif

	IDXGIFactory4* factory;
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**) (&factory));
	if(result != S_OK) {
		assert(0 && "createDXGIFactory1 failed!");
		exit(-1);
	}

	if(_useWarpDevice) {
		IDXGIAdapter* warpAdapter;
		result = factory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**) (&warpAdapter));
		if(result != S_OK) {
			assert(0 && "EnumWarpAdapter failed!");
			exit(-1);
		}

		result = D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**) (&_device));
		if(result != S_OK) {
			assert(0 && "D3D12CreateDevice failed!");
			exit(-1);
		}
	}
	else {
		IDXGIAdapter1* hardwareAdapter;
		GetHardwareAdapter(factory, &hardwareAdapter);

		result = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**) (&_device));
		if(result != S_OK) {
			assert(0 && "D3D12CreateDevice failed!");
			exit(-1);
		}
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = _device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**) (&_commandQueue));
	if(result != S_OK) {
		assert(0 && "CreateCommandQueue failed!");
		exit(-1);
	}

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = _windowWidth;
	swapChainDesc.Height = _windowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	if(!_hwnd) {
		assert(0 && "HWND is not set");
	}

	IDXGISwapChain1* swapChain;
	result = factory->CreateSwapChainForHwnd(
		_commandQueue,		// Swap chain needs the queue so that it can force a flush on it.
		_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);
	if(result != S_OK) {
		assert(0 && "CreateSwapChainForHwnd failed!");
		exit(-1);
	}

	result = factory->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER);
	if(result != S_OK) {
		assert(0 && "MakeWindowAssociation failed!");
		exit(-1);
	}

	_swapChain = (IDXGISwapChain3*) swapChain;
	_frameIndex = _swapChain->GetCurrentBackBufferIndex();

	// Create a root signature with a single constant buffer slot.
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;

		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

		ID3DBlob* pSignature;
		ID3DBlob* pError;

		result = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
		if(result != S_OK) {
			assert(0 && "D3D12SerializeRootSignature failed!");
			exit(-1);
		}

		result = _device->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
											   __uuidof(ID3D12RootSignature), (void**) (&_rootSignature));
		if(result != S_OK) {
			assert(0 && "CreateRootSignature failed!");
			exit(-1);
		}
	}

	result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**) (&_commandAllocator));
	if(result != S_OK) {
		assert(0 && "CreateCommandAllocator failed!");
		exit(-1);
	}
}

void Dx12Renderer::LoadAssets() {
	HRESULT result = S_OK;

	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	result = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
	if(result != S_OK) {
		assert(0 && "veretxShader failed!");
		exit(-1);
	}

	result = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);
	if(result != S_OK) {
		assert(0 && "pixelShader failed!");
		exit(-1);
	}

	// Create the pipeline state once the shaders are loaded.
	{
		static const D3D12_INPUT_ELEMENT_DESC inputLayout [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { inputLayout, _countof(inputLayout) };
		state.pRootSignature = _rootSignature;
		state.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
		state.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
		state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		state.NumRenderTargets = 1;
		state.RTVFormats [0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		state.SampleDesc.Count = 1;

		result = _device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&_pipelineState));
		if(result != S_OK) {
			assert(0 && "CreateGraphicsPipelineState failed!");
			exit(-1);
		}
	}


	// Create the command list.
	result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, _pipelineState,
										 __uuidof(ID3D12CommandList), (void**) (&_commandList));
	if(result != S_OK) {
		assert(0 && "CreateCommandList failed!");
		exit(-1);
	}

	loadVertices("cubeVertices.txt");
	
	const UINT vertexBufferSize = _meshesVertices.size() * sizeof(Vertex);

	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	result = _device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**) (&_vertexBuffer));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	result = _device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**) (&_vertexBufferUpload));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	// Upload the vertex buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(_meshesVertices.data());
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(_commandList, _vertexBuffer, _vertexBufferUpload, 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}

	const UINT indexBufferSize = _cubeIndices.size() * sizeof(unsigned short);

	// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	ID3D12Resource* indexBufferUpload;

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	result = _device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**) (&_indexBuffer));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	result = _device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**) (&indexBufferUpload));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	// Upload the index buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(_cubeIndices.data());
		indexData.RowPitch = indexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources(_commandList, _indexBuffer, indexBufferUpload, 0, 0, 1, &indexData);

		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	}

	// Create a descriptor heap for the constant buffers.
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		heapDesc.NumDescriptors = FrameCount * _meshNum;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		result = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_cbvHeap));
		if(result != S_OK) {
			assert(0 && "CreateDescriptorHeap failed!");
			exit(-1);
		}

		_cbvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create descriptor heaps for render target views and depth stencil views.
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		result = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap));
		if(result != S_OK) {
			assert(0 && "CreateDescriptorHeap failed!");
			exit(-1);
		}

		_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for(UINT n = 0; n < FrameCount; n++) {
			result = _swapChain->GetBuffer(n, __uuidof(ID3D12Resource), (void**) (&_renderTargets [n]));
			if(result != S_OK) {
				assert(0 && "GetBuffer failed!");
				exit(-1);
			}
			_device->CreateRenderTargetView(_renderTargets [n], nullptr, rtvHandle);
			rtvHandle.Offset(1, _rtvDescriptorSize);
		
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		result = _device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap));
		if(result != S_OK) {
			assert(0 && "CreateDescriptorHeap failed!");
			exit(-1);
		}
	}

	// Create a depth stencil and view.
	{
		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, _windowWidth, _windowHeight, 1, 1);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

		result = _device->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&_depthStencil)
		);
		if(result != S_OK) {
			assert(0 && "CreateCommittedResource failed!");
			exit(-1);
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		_device->CreateDepthStencilView(_depthStencil, &dsvDesc, _dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(FrameCount *_meshNum * c_alignedConstantBufferSize);
	result = _device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**) (&_constantBuffer));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	// Create constant buffer views to access the upload buffer.
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = _constantBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(_cbvHeap->GetCPUDescriptorHandleForHeapStart());

	for(int n = 0; n < FrameCount; n++) {
		for(int i = 0; i < _meshNum; i++) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = c_alignedConstantBufferSize;
			_device->CreateConstantBufferView(&desc, cbvCpuHandle);

			cbvGpuAddress += desc.SizeInBytes;
			cbvCpuHandle.Offset(_cbvDescriptorSize);
		}
	}

	// Map the constant buffers.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	result = _constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_mappedConstantBuffer));
	if(result != S_OK) {
		assert(0 && "CreateCommittedResource failed!");
		exit(-1);
	}

	memcpy(_mappedConstantBuffer, _constantBuffer, sizeof(_constantBuffer));
	//ZeroMemory(m_mappedConstantBuffer, FrameCount * c_alignedConstantBufferSize);
	// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

	// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
	result = (_commandList->Close());
	if(result != S_OK) {
		assert(0 && "Close CommandList failed!");
		exit(-1);
	}

	ID3D12CommandList* ppCommandLists [] = { _commandList };
	_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create vertex/index buffer views.
	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(Vertex);
	_vertexBufferView.SizeInBytes = _meshesVertices.size() * sizeof(Vertex);

	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = _cubeIndices.size() * sizeof(unsigned short);
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	float fovAngleY = 45.0f * XM_PI / 180.0f;

	if(_aspectRatio < 1.0f) {
		fovAngleY *= 2.0f;
	}

	XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovRH(
		fovAngleY,
		_aspectRatio,
		0.01f,
		100.0f
	);

	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	XMFLOAT4X4 orientation = Rotation0;
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&_constantBufferData.projection,
		DirectX::XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	_cameraPositionX = -10.0f;
	_cameraPositionY = 2.0f;
	_cameraPositionZ = 3.5f;

	_atPositionX = 0.0f;
	_atPositionY = 0.0f;
	_atPositionZ = 0.0f;

	// Eye is at (0,2,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { _cameraPositionX, _cameraPositionY, _cameraPositionZ, 0.0f };
	static const XMVECTORF32 at = { _atPositionX, _atPositionY, _atPositionZ, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&_constantBufferData.view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(eye, at, up)));


	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		result = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**) (&_fence));
		if(result != S_OK) {
			assert(0 && "CreateFence failed!");
			exit(-1);
		}

		_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if(_fenceEvent == nullptr) {
			result = HRESULT_FROM_WIN32(GetLastError());
			if(result != S_OK) {
				assert(0 && "CreateEvent failed!");
				exit(-1);
			}
		}

		_vertexPerMesh = _meshesVertices.size();
		int indicesPerMesh = _cubeIndices.size();

		// I want to put mesh togeter in some kind of square

		unsigned int rowNum = (unsigned int) (sqrt(_meshNum));
		unsigned int meshPerRow = _meshNum / rowNum;

		unsigned int centerIndex = meshPerRow / 2; // the cube index (i,j) which will be around 0,0

		//TODO: !!! calculate object width + object height

		int objectWidth = 4;
		int objectHeight = 4;

		//create Meshes

		for(int i = 0; i <= rowNum; i++) {  // +/- flip
			for(int j = 0; j <= meshPerRow; j++) {

				int currentIndex = i*meshPerRow + j;
				if(currentIndex >= _meshNum) {
					break;
				}

				float traslateValueY = (float) (((int) centerIndex - i) * objectWidth);
				float traslateValueX = (float) (((int) centerIndex - j) * objectHeight);

				TranslateData trans;
				trans.X = traslateValueX;
				trans.Y = traslateValueY;

				_translateValues.push_back(trans);
			}
		}

		for(int i = 0; i < _meshNum; i++) {

			XMStoreFloat4x4(&_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, _translateValues [i].X, _translateValues [i].Y)));

			UINT8* destination = _mappedConstantBuffer + i * c_alignedConstantBufferSize;
			memcpy(destination, &_constantBufferData, sizeof(_constantBufferData));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}

}

void Dx12Renderer::PopulateCommandList() {
	//UINT8* destination = _mappedConstantBuffer + (_frameIndex * c_alignedConstantBufferSize);
	//memcpy(destination, &_constantBufferData, sizeof(_constantBufferData));
	bool rotate = false;
	static float angle = 0.0f;
	HRESULT result = S_OK;
	result = _commandAllocator->Reset();
	if(result != S_OK) {
		assert(0 && "Reset failed!");
		exit(-1);
	}

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	result = _commandList->Reset(_commandAllocator, _pipelineState);
	if(result != S_OK) {
		assert(0 && "Reset failed!");
		exit(-1);
	}

	//Draw the cube"
	{
		if(_begin == 0) {
			_begin = clock();
			rotate = true;
			angle += 3;
		}

		float end = clock();

		if(double(end - _begin) / CLOCKS_PER_SEC > 0.05) {
			_begin = clock();
			
		}

		const float clearColor [] = { 0.0f, 0.2f, 0.0f, 1.0f }; _frameIndex = _swapChain->GetCurrentBackBufferIndex();

		// Set the graphics root signature and descriptor heaps to be used by this frame.
		_commandList->SetGraphicsRootSignature(_rootSignature);
		ID3D12DescriptorHeap* ppHeaps [] = { _cbvHeap };
		_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Bind the current frame's constant buffer to the pipeline.

		// Set the viewport and scissor rectangle.
		_commandList->RSSetViewports(1, &_viewport);
		_commandList->RSSetScissorRects(1, &_scissorRect);

		// Indicate this resource will be in use as a render target.
		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets [_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		
		const UINT vertexBufferSize = _meshesVertices.size() * sizeof(Vertex);

		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(_meshesVertices.data());
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer,
									   D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources<1>(_commandList, _vertexBuffer, _vertexBufferUpload, 0, 0, 1, &vertexData);
		_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer,
									   D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		// Record drawing commands.
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetView(_rtvHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilView(_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		_commandList->ClearRenderTargetView(renderTargetView, clearColor, 0, nullptr);
		_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);
		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
		_commandList->IASetIndexBuffer(&_indexBufferView);

		for(int i = 0; i < _meshNum; i++) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_cbvHeap->GetGPUDescriptorHandleForHeapStart(),  i, _cbvDescriptorSize);
			_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
			if(rotate) {
				//TODO
				//XMStoreFloat4x4(&_constantBufferData.model, XMMatrixTranspose(XROtate * XMMatrixTranslation(0.0f, _translateValues [i].X, _translateValues [i].Y)));

				//UINT8* destination = _mappedConstantBuffer + i * c_alignedConstantBufferSize;
				//memcpy(destination, &_constantBufferData, sizeof(_constantBufferData));
			}
			_commandList->DrawIndexedInstanced(_cubeIndices.size(), 1, 0 , 0, 0);
		}

		// Indicate that the render target will now be used to present when the command list is done executing.
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets [_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		_commandList->ResourceBarrier(1, &presentResourceBarrier);
	}

	result = _commandList->Close();
	if(result != S_OK) {
		assert(0 && "Close failed!");
		exit(-1);
	}

	// Record commands.
	// Indicate that the back buffer will now be used to present.

}

void Dx12Renderer::WaitForPreviousFrame() {
	HRESULT result = S_OK;

	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = _fenceValue;
	result = _commandQueue->Signal(_fence, fence);
	if(result != S_OK) {
		assert(0 && "Signal failed!");
		exit(-1);
	}

	_fenceValue++;

	// Wait until the previous frame is finished.
	if(_fence->GetCompletedValue() < fence) {
		result = _fence->SetEventOnCompletion(fence, _fenceEvent);
		if(result != S_OK) {
			assert(0 && "SetEventOnCompletion failed!");
			exit(-1);
		}

		WaitForSingleObject(_fenceEvent, INFINITE);
	}

	_frameIndex = _swapChain->GetCurrentBackBufferIndex();
}

void Dx12Renderer::loadVertices(char* fileName) {
	FILE* fp;

	if(!(fp = fopen(fileName, "r"))) {
		assert(0 && "Can not open file!");
	}

	int size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	if(!size) {
		assert(0 && "file is empty!");
	}

	fseek(fp, 0, SEEK_SET);

	char c;
	bool comment = false;
	bool face = false;
	bool vertex = false;
	bool color = false;


	char buffer [32] = { 0 };

	Vertex currentVertex;

	uint8_t valueConuter = 0;
	uint8_t charCounter = 0;

	float value [4] = { 0 };

	while((c = getc(fp)) != EOF) {
		if(c == '#') {
			comment = true;
			continue;
		}

		if(c == '\n') {
			if(!face && !vertex && !color) {
				comment = false;
				continue;
			}

			if(buffer [0] != '\0') {
				if(!color) {
					if(valueConuter > 2) {
						assert(0 && "Invalid data in file");
					}
				} else {
					if(valueConuter > 3) {
						assert(0 && "Invalid data in file");
					}
				}

				value [valueConuter] = atof(buffer);
				valueConuter++;

				//clear buffer
				for(int i = 0; i < 32; i++) {
					buffer [i] = '\0';
				}

				charCounter = 0;
			}
			if(!color) {
				if(valueConuter != 3) { // too much or less than shuld be 
					assert(0 && "Invalid data in file");
				}
			} else {
				if(valueConuter != 4) { // too much or less than shuld be 
					assert(0 && "Invalid data in file");
				}
			}

			if(color) {
				if(!vertex) {
					assert(0 && "Invalid data in file, color without vertex");
				}

				currentVertex.color = { value [0], value [1], value [2], value[3] };
				_meshesVertices.push_back(currentVertex);

			} else if(face) {
				_cubeIndices.push_back((unsigned short) value [0]);
				_cubeIndices.push_back((unsigned short) value [1]);
				_cubeIndices.push_back((unsigned short) value [2]);
			}

			comment = false;
			face = false;
			vertex = false;
			color = false;
			valueConuter = 0;
			continue;
		}

		if(comment) {
			continue;
		}

		if(c == 'v') {
			comment = false;
			face = false;
			vertex = true;
			color = false;
			continue;
		}

		if(c == 'c') {
			comment = false;
			face = false;
			color = true;

			if(valueConuter != 3) { // too much or less than shuld be 
				assert(0 && "Invalid data in file");
			}

			currentVertex.position = { value [0], value [1], value [2] };
			valueConuter = 0;

			continue;
		}

		if(c == 'f') {
			comment = false;
			face = true;
			vertex = false;
			color = false;
			continue;
		}

		if(c == ' ' || c == '\t') {
			//flush value
			if(buffer [0] == '\0') {// smotething wrong meybe 2 separators?
				continue;
			}

			if(!color) {
				if(valueConuter > 2) {
					assert(0 && "Invalid data in file");
				}
			}
			else {
				if(valueConuter > 3) {
					assert(0 && "Invalid data in file");
				}
			}

			value [valueConuter] = atof(buffer);
			valueConuter++;

			//clear buffer
			for(int i = 0; i < 32; i++) {
				buffer [i] = '\0';
			}

			charCounter = 0;
			continue;
		}

		buffer [charCounter] = c;
		charCounter++;
	}

	if(valueConuter != 0) {
		if(buffer [0] != '\0') {
			if(!color) {
				if(valueConuter > 2) {
					assert(0 && "Invalid data in file");
				}
			}
			else {
				if(valueConuter > 3) {
					assert(0 && "Invalid data in file");
				}
			}

			value [valueConuter] = atof(buffer);
			valueConuter++;

			//clear buffer
			for(int i = 0; i < 32; i++) {
				buffer [i] = '\0';
			}

			charCounter = 0;
		}
		if(!color) {
			if(valueConuter != 3) { // too much or less than shuld be 
				assert(0 && "Invalid data in file");
			}
		}
		else {
			if(valueConuter != 4) { // too much or less than shuld be 
				assert(0 && "Invalid data in file");
			}
		}

		if(color) {
			if(!vertex) {
				assert(0 && "Invalid data in file, color without vertex");
			}

			currentVertex.color = { value [0], value [1], value [2], value [3] };
			_meshesVertices.push_back(currentVertex);

		}
		else if(face) {
			_cubeIndices.push_back((unsigned short) value [0]);
			_cubeIndices.push_back((unsigned short) value [1]);
			_cubeIndices.push_back((unsigned short) value [2]);
		}
	}
}