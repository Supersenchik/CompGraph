#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>

using u8 = uint8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using f32 = float;
using f64 = double;


ID3D11Device*           d3d11Device = nullptr;
ID3D11DeviceContext*    d3d11DeviceContext = nullptr;
IDXGISwapChain*         swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;


HRESULT InitializeD3D11( HWND hwnd, u32 width, u32 height );
void ReleaseD3D11();
void RenderScene();


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
	swapChainDesc.Windowed = TRUE;

	// Create our SwapChain
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

	return S_OK;
}


//
void RenderScene() {
	// clear our backbuffer to the updated color
	f32 ClearColor[ 4 ] = { 0.337f, 0.627f, 0.827f, 1.0f };
	d3d11DeviceContext->ClearRenderTargetView( renderTargetView, ClearColor );

	// present the backbuffer to the screen
	swapChain->Present( 0, 0 );
}


//
void ReleaseD3D11() {
	// release the COM objects we created
	if( d3d11DeviceContext )
		d3d11DeviceContext->ClearState();
	if( renderTargetView )
		renderTargetView->Release();
	if( swapChain )
		swapChain->Release();
	if( d3d11DeviceContext )
		d3d11DeviceContext->Release();
	if( d3d11Device )
		d3d11Device->Release();
}
