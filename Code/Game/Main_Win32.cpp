#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Math/Vector2.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include <time.h>

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);



//-----------------------------------------------------------------------------------------------
void Initialize( HINSTANCE applicationInstanceHandle )
{
	UNUSED(applicationInstanceHandle);

	SetProcessDPIAware();
	g_theApp = new App();
}


//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete g_theApp;
	g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	Initialize( applicationInstanceHandle );

	while( !g_theApp->IsQuitting() )
	{
		Sleep(1);
		g_theApp->RunFrame();
	}

	Shutdown();
	return 0;
}


