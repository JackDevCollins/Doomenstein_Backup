#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/App.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Eventually, we'll remove most OpenGL references out of Main_Win32.cpp
// Both of the following lines will eventually move to the top of Engine/Renderer/Renderer.cpp
//
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#include <Engine/Math/Vec2.hpp>
#include <Engine/Core/Rgba8.hpp>
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Later we will move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will eventually go away once we add a Window engine class later on.
// 


//----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will become  App::Render() const
//
// Some simple OpenGL example drawing code.
// This is the graphical equivalent of printing "Hello, world."
// #SD1ToDo: Move this function to Game/App.cpp and rename it to  App::Render() const
// #SD1ToDo: Move *ALL* OpenGL code to Renderer.cpp (only).
//
// Ultimately this function (App::Render) will only call methods on Renderer (like Renderer::DrawVertexArray)
//	to draw things, never calling OpenGL (nor DirectX) functions directly.
//
//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED(applicationInstanceHandle );
	UNUSED( commandLineString );

	//CreateOSWindow( applicationInstanceHandle, CLIENT_ASPECT );

	g_app = new App();
	/////////App_Constructor( applicationInstanceHandle, commandLineString ); // This will get replaced with:
	
	while (!g_app->IsQuitting())
	{	
		g_app->RunFrame();
	}

	delete g_app;
	g_app = nullptr;
	return 0;
}