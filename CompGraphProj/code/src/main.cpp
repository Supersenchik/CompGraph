#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

using u8 = uint8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using f32 = float;
using f64 = double;

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

struct ConstantBuffer {
	DirectX::XMMATRIX modelMatrix;
};

ID3D11Device*           d3d11Device = nullptr;
ID3D11DeviceContext*    d3d11DeviceContext = nullptr;
IDXGISwapChain*         swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

ID3D11Buffer*           vertexBuffer = nullptr;
ID3D11InputLayout*      vertexLayout = nullptr;
ID3D11VertexShader*     vertexShader = nullptr;
ID3D11PixelShader*      pixelShader = nullptr;

ID3D11Buffer*           indexBuffer = nullptr;
ID3D11Buffer*           constantBuffer = nullptr;

u32						renderWidth = 0;
u32						renderHeight = 0;
f32                     angle = 0.0f;

HRESULT InitializeD3D11( HWND hwnd, u32 width, u32 height );
void ReleaseD3D11();
void RenderScene();
HRESULT CreateObject();
void Rotate();

//
i32 CALLBACK WinMain( HINSTANCE /*hInstance*/, HINSTANCE, LPSTR /*lpCmdLine*/, i32 /*nCmdShow*/ ) {
	// the window we'll be rendering to
	SDL_Window* window = nullptr;

	// initialize SDL
	i32 result = SDL_Init( SDL_INIT_VIDEO );
	if( result < 0 ) {
		//printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// create window

	/*SDL_DisplayMode displayMode;
	SDL_GetDesktopDisplayMode(0, &displayMode);*/

	const u32 Width = 1366;
	const u32 Height = 768;

	window = SDL_CreateWindow(
		"D3D11 Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		Width, Height,
		SDL_WINDOW_SHOWN |
		SDL_WINDOW_FULLSCREEN_DESKTOP );
	if( window == nullptr ) {
		//printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION( &wmInfo.version );
	SDL_GetWindowWMInfo( window, &wmInfo );
	if( FAILED( InitializeD3D11( wmInfo.info.win.window, Width, Height ) ) )
		return EXIT_FAILURE;

	if( FAILED( CreateObject() ) )
		return EXIT_FAILURE;

	bool quit = false;

	// while application is running
	while( !quit ) {
		SDL_Event e;

		// handle events on queue
		while( SDL_PollEvent( &e ) ) {
			switch( e.type ) {
				case SDL_KEYUP:
					switch( e.key.keysym.sym ) {
						case SDLK_F11:
							if( SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN_DESKTOP )
								SDL_SetWindowFullscreen( window, 0 );
							else
								SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN_DESKTOP );
							break;
						case SDLK_ESCAPE:
							quit = true;
							break;
					}
					break;
				case SDL_QUIT:
					quit = true;
					break;
			}
		}
		Rotate();
		RenderScene();
	}

	// destroy window
	ReleaseD3D11();
	SDL_DestroyWindow( window );

	// quit SDL subsystems
	SDL_Quit();

	return EXIT_SUCCESS;
}


//
HRESULT InitializeD3D11( HWND hwnd, u32 width, u32 height ) {
	u32 deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// describe our Buffer
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset( &swapChainDesc, 0, sizeof( swapChainDesc ) );
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;

	// create our SwapChain
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&d3d11Device,
		nullptr,
		&d3d11DeviceContext );
	if( FAILED( result ) )
		return result;

	// create our BackBuffer
	ID3D11Texture2D* backBuffer = nullptr;
	result = swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( void** )&backBuffer );
	if( FAILED( result ) )
		return result;

	// create our Render Target
	result = d3d11Device->CreateRenderTargetView( backBuffer, nullptr, &renderTargetView );
	backBuffer->Release();
	if( FAILED( result ) )
		return result;

	// set our Render Target
	d3d11DeviceContext->OMSetRenderTargets( 1, &renderTargetView, nullptr );

	D3D11_VIEWPORT vp;
	vp.Width = ( f32 )width;
	vp.Height = ( f32 )height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	d3d11DeviceContext->RSSetViewports( 1, &vp );

	renderWidth = width;
	renderHeight = height;

	return S_OK;
}


//
void RenderScene() {
	const f32 ClearColor[ 4 ] = { 0.337f, 0.627f, 0.827f, 1.0f };
	d3d11DeviceContext->ClearRenderTargetView( renderTargetView, ClearColor );
	d3d11DeviceContext->IASetInputLayout( vertexLayout );
	u32 stride = sizeof( Vertex );
	u32 offset = 0;
	d3d11DeviceContext->IASetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );
	d3d11DeviceContext->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	d3d11DeviceContext->VSSetShader( vertexShader, nullptr, 0 );
	d3d11DeviceContext->PSSetShader( pixelShader, nullptr, 0 );
	d3d11DeviceContext->VSSetConstantBuffers( 0, 1, &constantBuffer );
	d3d11DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	d3d11DeviceContext->DrawIndexed( 36, 0, 0 );
	swapChain->Present( 0, 0 );
}


//
template< class T >
void SafeRelease( T*& o ) {
	if( o ) {
		o->Release();
		o = nullptr;
	}
}


//
void ReleaseD3D11() {
	// release the COM objects we created
	if( d3d11DeviceContext )
		d3d11DeviceContext->ClearState();

	SafeRelease( constantBuffer );
	SafeRelease( indexBuffer );
	SafeRelease( vertexBuffer );
	SafeRelease( vertexLayout );
	SafeRelease( vertexShader );
	SafeRelease( pixelShader );
	SafeRelease( renderTargetView );
	SafeRelease( swapChain );
	SafeRelease( d3d11DeviceContext );
	SafeRelease( d3d11Device );
}


//
HRESULT CreateObject() {
	u32 compileFlags = 0;;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif

	// компил€ци€ вершинного шейдера
	ID3DBlob* blob = nullptr;
	HRESULT result = D3DCompileFromFile( L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &blob, nullptr );
	if( FAILED( result ) )
		return result;

	// создание вершинного шейдера
	result = d3d11Device->CreateVertexShader( blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader );
	if( FAILED( result ) )
		return result;

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,							  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	u32 numElements = ARRAYSIZE( layout );
	result = d3d11Device->CreateInputLayout( layout, numElements, blob->GetBufferPointer(), blob->GetBufferSize(), &vertexLayout );
	blob->Release();
	if( FAILED( result ) )
		return result;

	// компил€ци€ пиксельного шейдера
	result = D3DCompileFromFile( L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &blob, nullptr );
	if( FAILED( result ) )
		return result;

	// создание пиксельного шейдера
	result = d3d11Device->CreatePixelShader( blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pixelShader );
	blob->Release();
	if( FAILED( result ) )
		return result;

	using namespace DirectX;

	// создание вершинного буфера
	const Vertex vertices[] = {
		{ { -1.0f, 1.0f,  -1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f,  1.0f,  -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f,  1.0f,  1.0f  }, { 0.0f, 1.0f } },
		{ { -1.0f, 1.0f,  1.0f  }, { 1.0f, 1.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f,  -1.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f,  -1.0f, 1.0f  }, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 1.0f  }, { 0.0f, 1.0f } },

		{ { -1.0f, -1.0f, 1.0f  }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, 1.0f,  -1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, 1.0f,  1.0f  }, { 0.0f, 0.0f } },

		{ { 1.0f,  -1.0f, 1.0f  }, { 1.0f, 1.0f } },
		{ { 1.0f,  -1.0f, -1.0f }, { 0.0f, 1.0f } },
		{ { 1.0f,  1.0f,  -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f,  1.0f,  1.0f  }, { 1.0f, 0.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } },
		{ { 1.0f,  -1.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { 1.0f,  1.0f,  -1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, 1.0f,  -1.0f }, { 0.0f, 0.0f } },

		{ { -1.0f, -1.0f, 1.0f  }, { 1.0f, 1.0f } },
		{ { 1.0f,  -1.0f, 1.0f  }, { 0.0f, 1.0f } },
		{ { 1.0f,  1.0f,  1.0f  }, { 0.0f, 0.0f } },
		{ { -1.0f, 1.0f,  1.0f  }, { 1.0f, 0.0f } },
	};
	D3D11_BUFFER_DESC bd;
	memset( &bd, 0, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof( vertices );
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.StructureByteStride = sizeof( Vertex );

	D3D11_SUBRESOURCE_DATA subData;
	memset( &subData, 0, sizeof( subData ) );
	subData.pSysMem = vertices;
	result = d3d11Device->CreateBuffer( &bd, &subData, &vertexBuffer );
	if( FAILED( result ) )
		return result;

	// создание буфера индексов
	// indices in clock-wise order
	const u16 indices[] = {
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
	};
	memset( &bd, 0, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof( indices );
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.StructureByteStride = sizeof( u16 );
	subData.pSysMem = indices;
	result = d3d11Device->CreateBuffer( &bd, &subData, &indexBuffer );
	if( FAILED( result ) )
		return result;

	// создание константного буфера
	memset( &bd, 0, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( ConstantBuffer );
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.StructureByteStride = sizeof( ConstantBuffer );
	result = d3d11Device->CreateBuffer( &bd, nullptr, &constantBuffer );
	if( FAILED( result ) )
		return result;

	return S_OK;
}


//
inline f32 ToRad( f32 deg ) {
	const float PI = 3.141592654f;
	return deg * PI / 180.0f;
}


//
void Rotate() {
	angle += 0.0002f;

	using namespace DirectX;

	XMFLOAT3 pos( 1, 1, 1 );
	XMVECTOR normal = DirectX::XMLoadFloat3( &pos );
	XMMATRIX modelViewProjection = DirectX::XMMatrixRotationAxis( normal, angle );

	const XMVECTOR Eye = { 0, 0, -5 };
	const XMVECTOR Focus = { 0, 0, 1 };
	const XMVECTOR Up = { 0, 1, 0 };
	modelViewProjection *= XMMatrixLookAtLH( Eye, Focus, Up );

	const f32 FovY = ToRad( 80.0f );
	const f32 Aspect = ( f32 )renderWidth / renderHeight;
	modelViewProjection *= XMMatrixPerspectiveFovLH( FovY, Aspect, 0.1f, 100.0f );

	ConstantBuffer cb;
	cb.modelMatrix = XMMatrixTranspose( modelViewProjection );

	D3D11_MAPPED_SUBRESOURCE mr;
	d3d11DeviceContext->Map( constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mr );
	memcpy( mr.pData, &cb, sizeof( cb ) );
	d3d11DeviceContext->Unmap( constantBuffer, 0 );
}
