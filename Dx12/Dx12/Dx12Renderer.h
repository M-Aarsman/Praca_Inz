#pragma once
#include <d3d12.h>
#include "d3dx12.h"
//#include "Mesh.h"
#include <dxgi1_4.h>

#include <vector>
#include "Shared.h"
#include <unordered_map>
#include  <memory>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

struct TranslateData {
	TranslateData()
		: X(0.0f),
		Y(0.0f),
		Z(0.0f) {};

	float X;
	float Y;
	float Z;
};

static DirectX::XMFLOAT4X4 Identity4x4() {
	static DirectX::XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

enum Key {
	KEY_UP,
	KEY_DOWN,

	KEY_NUM
};

struct ModelViewProjectionConstantBuffer {
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 rotation;
};

class Dx12Renderer {
public:
	Dx12Renderer(int meshNum);

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void OnKeyDown(UINT8 key);
	void OnKeyUp(UINT8 key);

	void CameraUpdate();
	void ZoomIn(float n);
	void ZoomOut(float n);

	void SetWindowWidth(int width) {
		_windowWidth = width;
	}

	void SetWindowHeight(int height) {
		_windowHeight = height;
	}

	void SetHWND(HWND hwnd) {
		_hwnd = hwnd;
	}

private:
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
	void loadVertices(char* fileName);

private:
	static const UINT FrameCount = 2;
	static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

	// Pipeline objects.
	D3D12_VIEWPORT _viewport;
	D3D12_RECT _scissorRect;
	IDXGISwapChain3* _swapChain;
	ID3D12Device* _device;
	ID3D12Resource* _renderTargets [FrameCount];
	ID3D12Resource* _depthStencil;
	ID3D12CommandAllocator* _commandAllocator;
	ID3D12CommandQueue* _commandQueue;
	ID3D12RootSignature* _rootSignature;
	ID3D12DescriptorHeap* _rtvHeap;
	ID3D12DescriptorHeap* _cbvHeap;
	ID3D12DescriptorHeap* _dsvHeap;
	ID3D12PipelineState* _pipelineState;
	ID3D12GraphicsCommandList* _commandList;
	UINT _rtvDescriptorSize;
	UINT _cbvDescriptorSize;
	UINT8* _mappedConstantBuffer;

	// App resources.
	ID3D12Resource* _vertexBuffer;
	ID3D12Resource* _indexBuffer;
	ID3D12Resource* _constantBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	// Synchronization objects.
	UINT _frameIndex;
	HANDLE _fenceEvent;
	ID3D12Fence* _fence;
	UINT64 _fenceValue;

	float _aspectRatio;

	bool _useWarpDevice;

	ModelViewProjectionConstantBuffer _constantBufferData;

	//camera value
	float _cameraPositionX = 0.0f;
	float _cameraPositionY = 0.0f;
	float _cameraPositionZ = 0.0f;

	float _atPositionX = 0.0f;
	float _atPositionY = 0.0f;
	float _atPositionZ = 0.0f;

	bool KEYS [KEY_NUM] = { 0 };

	HANDLE _swapChainEvent;

	std::vector<Vertex> _meshesVertices;
	std::vector<unsigned short> _cubeIndices;

	std::vector<TranslateData> _translateValues;

	unsigned int _vertexPerMesh = 0;
	unsigned int _meshNum;

	ID3D12Resource* _vertexBufferUpload;
	
	float _begin = 0;

	int _windowWidth = 0;
	int _windowHeight = 0;
	HWND _hwnd = nullptr;
};

