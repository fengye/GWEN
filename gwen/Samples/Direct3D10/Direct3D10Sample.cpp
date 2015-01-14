#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Renderers/DirectX10.h"
#include <dxerr.h>

#if defined(_MSC_VER)
	#pragma comment(lib, "d3d10")
	#pragma comment(lib, "DXGI")
	#pragma comment( lib, "winmm.lib" )
	#pragma comment( lib, "dxerr.lib" )
	#if defined(_DEBUG) | defined (DEBUG)
		#pragma comment(lib, "d3dx10d")
	#else 
		#pragma comment(lib, "d3dx10")
	#endif

#endif


HWND					g_pHWND = NULL;
ID3D10Device*		g_Device = NULL;
IDXGISwapChain* SwapChain =0;
ID3D10RenderTargetView	*BackBufferRTV=0;
ID3D10Texture2D			*BackBuffer=0;
RECT ClientRect;
//
// Windows bullshit to create a Window to render to.
//
HWND CreateGameWindow( void )
{
	WNDCLASS	wc;
	ZeroMemory( &wc, sizeof( wc ) );

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= DefWindowProc;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"GWENWindow";
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );

	RegisterClass( &wc );

	HWND hWindow = CreateWindowEx( (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) , wc.lpszClassName, L"GWEN - Direct 3D Sample", (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME), 400, 400, 1024, 768, NULL, NULL, GetModuleHandle(NULL), NULL );

	ShowWindow( hWindow, SW_SHOW );
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );

	return hWindow;
}


//
// Typical DirectX stuff to create a D3D device
//
void ResetD3DDevice()
{
	
}
#define USEPERFHUD 1
void CreateD3DDevice()
{
	// this has to be the client rect, not the window rect
	GetClientRect( g_pHWND, &ClientRect );

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = ClientRect.right;
	sd.BufferDesc.Height = ClientRect.bottom;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count   = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = g_pHWND;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;
	// for using perfhud, 
#if defined(USEPERFHUD)
	IDXGIFactory *pDXGIFactory;

	HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pDXGIFactory));
	// Search for a PerfHUD adapter.
	UINT nAdapter = 0;
	IDXGIAdapter* adapter = NULL;
	IDXGIAdapter* selectedAdapter = NULL;
	D3D10_DRIVER_TYPE driverType = D3D10_DRIVER_TYPE_HARDWARE;

	while (pDXGIFactory->EnumAdapters(nAdapter, &adapter) !=DXGI_ERROR_NOT_FOUND) {
		if (adapter) {
	
			DXGI_ADAPTER_DESC adaptDesc;
			if (SUCCEEDED(adapter->GetDesc(&adaptDesc))) {
				bool isPerfHUD = true;

				isPerfHUD = wcscmp(adaptDesc.Description, L"NVIDIA PerfHUD") == 0;

				// Select the first adapter in normal circumstances or the PerfHUD one if it exists.
				if(nAdapter == 0 || isPerfHUD) selectedAdapter = adapter;
				if(isPerfHUD) driverType = D3D10_DRIVER_TYPE_REFERENCE;
			}
		}
		++nAdapter;
	}
	UINT flags =0;
#if defined(DEBUG) | defined (_DEBUG)
	flags = D3D10_CREATE_DEVICE_DEBUG;
#endif
	HR(D3D10CreateDevice( selectedAdapter, driverType, NULL, flags, D3D10_SDK_VERSION, &g_Device));
	HR(pDXGIFactory->CreateSwapChain(g_Device, &sd, &SwapChain));
	pDXGIFactory->Release();
	selectedAdapter->Release();
#else
	UINT flags =0;
#if defined(DEBUG) | defined (_DEBUG)
	flags = D3D10_CREATE_DEVICE_DEBUG;
#endif
	HR(D3D10CreateDeviceAndSwapChain(0, D3D10_DRIVER_TYPE_HARDWARE, 0, flags, D3D10_SDK_VERSION, &sd, &SwapChain, &g_Device));
#endif
	
	HR(SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**) &BackBuffer));
	HR(g_Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRTV));
	BackBuffer->Release();
	BackBuffer=0;
	g_Device->OMSetRenderTargets(1, &BackBufferRTV, 0);
	// Setup the viewport
	D3D10_VIEWPORT viewport;
	// this should be the window rect, not the client rect...
	RECT temp;
	GetWindowRect( g_pHWND, &temp );

	viewport.Width  = temp.right;
	viewport.Height = temp.bottom;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY =  0;
	g_Device->RSSetViewports(1, &viewport);

}

//
// Program starts here
//
int main(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {	

	g_pHWND = CreateGameWindow();
	CreateD3DDevice();	

	//
	// Create a GWEN DirectX renderer
	//
	Gwen::Renderer::DirectX10* pRenderer = new Gwen::Renderer::DirectX10( g_Device );
	RECT temp;
	GetWindowRect( g_pHWND, &temp );
	pRenderer->SetScreenSize(temp.right, temp.bottom);
	//
	// Create a GWEN skin
	//
	Gwen::Skin::TexturedBase skin(pRenderer);
	skin.Init( "DefaultSkin.png" );

	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( 1000, 700 );
	pCanvas->SetDrawBackground( true );
	pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );

	//
	// Create our unittest control (which is a Window with controls in it)
	//
	UnitTest* pUnit = new UnitTest( pCanvas );

	pUnit->SetPos( 10, 10 );

	//
	// Create a Windows Control helper 
	// (Processes Windows MSG's and fires input at GWEN)
	//
	Gwen::Input::Windows GwenInput;
	GwenInput.Initialize( pCanvas );

	//
	// Begin the main game loop
	//
	MSG msg;
	while( true )
	{
		// Skip out if the window is closed
		if ( !IsWindowVisible( g_pHWND ) )
			break;

		// If we have a message from windows..
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			// .. give it to the input handler to process
			GwenInput.ProcessMessage( msg );

			// if it's QUIT then quit..
			if ( msg.message == WM_QUIT )
				break;

			// Handle the regular window stuff..
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Normal DirectX rendering loop
	
				float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
				
				g_Device->ClearRenderTargetView( BackBufferRTV, ClearColor );
	
				// This is how easy it is to render GWEN!
				pCanvas->RenderCanvas();

				SwapChain->Present(0, 0);

		}
	}
	BackBufferRTV->Release();
	BackBufferRTV=0;
	if ( g_Device )
	{
		g_Device->Release();
		g_Device = NULL;
	}
	

}