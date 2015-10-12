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

ID3D11Device*           d3d11Device = NULL;
ID3D11DeviceContext*    d3d11DeviceContext = NULL;
IDXGISwapChain*         swapChain = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;

u32 width = 1366;
u32 height = 768;

int InitializeD3D11(HWND hwnd);
void ReleaseD3D11();
void RenderScene();

int main(i32 argc, char* args[])
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//Initialize SDL
	i32 result = 0;
	if ((result = SDL_Init(SDL_INIT_VIDEO)) < 0)
	{
		//printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return result;
	}
	
	//Create window

	/*SDL_DisplayMode displayMode;
	SDL_GetDesktopDisplayMode(0, &displayMode);*/

	window = SDL_CreateWindow(	"D3D11 Test",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								width, height,
								SDL_WINDOW_SHOWN |
								SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (window == NULL)
	{
		//printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	if ((result = InitializeD3D11(wmInfo.info.win.window)) < 0)
		return result;

	bool quit = false;
	SDL_Event e;

	// While application is running
	while (!quit)
	{
		// Handle events on queue
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
				case SDLK_F11:
					if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
						SDL_SetWindowFullscreen(window, SDL_FALSE);
					else
						SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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

	// Destroy window
	ReleaseD3D11();
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

int InitializeD3D11(HWND hwnd)
{
	i32 result;

	u32 createDeviceFlags = NULL;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Describe our Buffer
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));		
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
	result = D3D11CreateDeviceAndSwapChain(	NULL,
											D3D_DRIVER_TYPE_HARDWARE,
											NULL,
											createDeviceFlags,
											NULL,
											NULL,
											D3D11_SDK_VERSION,
											&swapChainDesc,
											&swapChain,
											&d3d11Device,
											NULL,
											&d3d11DeviceContext);
	if (result < 0)
		return result;

	// Create our BackBuffer
	ID3D11Texture2D* backBuffer = NULL;
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (result < 0)
		return result;

	// Create our Render Target
	result = d3d11Device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
	backBuffer->Release();
	if (result < 0)
		return result;

	//Set our Render Target
	d3d11DeviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

	return result;
}

void RenderScene()
{
	// Clear our backbuffer to the updated color
	f32 ClearColor[4] = { 0.337f, 0.627f, 0.827f, 1.0f };

	d3d11DeviceContext->ClearRenderTargetView(renderTargetView, ClearColor);

	//Present the backbuffer to the screen
	swapChain->Present(0, 0);
}

void ReleaseD3D11()
{
	// Release the COM Objects we created
	if (d3d11DeviceContext)
		d3d11DeviceContext->ClearState();
	if (renderTargetView)
		renderTargetView->Release();
	if (swapChain)
		swapChain->Release();
	if (d3d11DeviceContext)
		d3d11DeviceContext->Release();
	if (d3d11Device)
		d3d11Device->Release();
}