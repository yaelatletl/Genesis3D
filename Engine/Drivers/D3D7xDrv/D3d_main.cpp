/****************************************************************************************/
/*  D3D_Main.cpp                                                                        */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: DD/D3D wrapper                                                         */
/*   TODO: Gamma table OK but is done is software not using dx7,                        */
/*         implement gamma/color correction in dx7                                      */
/*   TODO: Understand lightmap generation a little better                               */
/*   TODO: a lot of createthandle calls were made for a level with 1 texture... why?    */
/*         check and make sure textures are only sent once...                           */
/*   TODO: For the those wonderful people who are are having refresh rate problems...   */
/*         wait for vertical blank...                                                   */
/*   TODO: These are obsolete, I need to find out how to get the mipmaps working....    */
/*   TODO: It looks like newer cards will support 32-bit bumpmapping support... right   */
/*         now stick with 16-bit and 24-bit since dx7 examples only these formats       */
/*   02/21/2004 Wendell Buckner                                                         */
/*    DOT3 BUMPMAPPING                                                                  */
/*   01/05/2004 Wendell Buckner                                                         */   
/*    CONFIG DRIVER - Display the drivers capibilities in the the driver log            */
/*   01/03/2004 Wendell Buckner                                                         */   
/*    CONFIG DRIVER - Display the drivers capibilities in the the driver log            */
/*   12/27/2003 Wendell Buckner                                                         */
/*    CONFIG DRIVER - Make the driver configurable by "ini" file settings               */
/*   12/20/2003 Wendell Buckner                                                         */  
/*    CONFIG DRIVER - Informational: Display the zbuffer depth                          */
/*   12/20/2003 Wendell Buckner                                                         */   
/*    COMPRESSED TEXTURES - Display the four CC codes                                   */          
/*   04/28/2003 Wendell Buckner                                                         */
/*    BUMPMAPPING                                                                       */
/*   03/25/2003 Wendell Buckner                                                         */
/*    BUMPMAPPING                                                                       */
/*   03/17/2003 Wendell Buckner                                                         */
/*    Fog under dx6 would clear use the viewport interface which would automatically    */
/*    use the material color to clear the target                                        */
/*    dx7 doesn't support this... you must now clear the target using clear and passing */
/*    the color you want to clear it to.                                                */
/*   01/13/2003 Wendell Buckner                                                         */
/*     Use the geneis d3d api functions else there be problems                          */ 
/*   01/11/2003 Wendell Buckner                                                         */
/*    Force to bpp to desktop bpp...                                                    */
/*   01/11/2003 Wendell Buckner                                                         */
/*    The driver did not take into account the ZBufferD and allways created a 16-bit    */
/*    z-buffer                                                                          */
/*   01/05/2003 Wendell Buckner
/*    Set the memory to nulls so we can check against it later to see if it's available... */	
/*   01/04/2003 Wendell Buckner                                                         */
/*    This variable was never initialized?, I know I've done complete recompiles why    */
/*    hasn't this shown up before?                                                      */
/*   01/02/2003 Wendell Buckner                                                         */
/*    Allow z buffer depths of 32 bits                                                  */
/*   12/28/2002 Wendell Buckner                                                         */
/*    16-bit for 16bit and ect... right now always assumes 8-bit                        */
/*          32-bit alpha.                                                               */
/*    Allow/make 32-bit (ARGB) mode the default mode...                                 */ 
/*    This surface will be FORCED to 32-bit (ARGB) at the end of this routine if it is  */
/*    available...                                                                      */
/*    Give out the true 24-bit surface as well...                                       */
/*   03/01/2002 Wendell Buckner	                                                        */
/*    This wasn't commented in the original code but it was in my genesis 1.0 converted */
/*    code...not sure I should do this.		                                            */ 
/*   12/06/2001 Wendell Buckner                                                         */
/*    ENHANCEMENT - Allow triple buffering                                              */
/*	 01/24/2002 Wendell Buckner                                                         */
/*    Change flags for speed...		                                                    */
/*   12/06/2001 Wendell Buckner                                                         */
/*    ENHANCEMENT - Allow full-scene anti-aliasing                                      */
/*   02/28/2001 Wendell Buckner                                                         */  
/*	  Lighting is on by default so turn it off...                                       */
/*    Optimization from GeForce_Optimization2.doc                                       */
/*     9. Do not du  plicate render state commands.  Worse is useless renderstates.  Do   */
/*        not set a renderstate unless it is needed.                                    */
/*    Optimization from GeForce_Optimization2.doc                                       */ 
/*     Lights                                                                           */
/*     Apps should be sure to turn off per-vertex color material support when           */
/*     appropriate for increased performance.                                           */
/*    Optimization from GeForce_Optimization2.doc                                       */
/*     Lights                                                                           */
/*	  Also turning D3DRENDERSTATE_LOCALVIEWER to false saves transform cycles when exact*/ 
/*    specular highlights are not necessary.                                            */
/*   07/16/2000 Wendell Buckner                                                         */
/*    Convert to Directx7...                                                            */
/*                                                                                    */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include <Windows.h>
#include <Assert.h>
#include <stdio.h>
#include <DDraw.h>
#include <D3D.h>
#include <Math.h>

#include "D3D_Main.h"
#include "D3D_Err.h"
#include "D3D_fx.h"
#include "d3dcache.h"

#include "Render.h"
#include "D3DCache.h"
#include "THandle.h"
#include "PCache.h"

#undef ATTEMPT
#define ATTEMPT(x) if (!(x)) goto exit_with_error

#undef RELEASE
#define RELEASE(x) if (x) { x->Release(); x = NULL; }

static BOOL				bInitDone	=FALSE;
#ifdef STRICT
WNDPROC					pOldWndProc;
#else
FARPROC					pOldWndProc;
#endif

//================================================================================
//	Globals
//================================================================================
App_Info	AppInfo;				// Our global structure that knows all... (once initialized)

// start 32 bit changes
int BPP32 = 16; // Our bpp variable
int ZbufferD = 16; // our Z buffer depths
FILE *stream; // The variable we open our config file to
int gWidth, gHeight; // Global variables for our width and height
// end 32 bit changes

/* 12/27/2003 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
int BBufferCount = 2;
int FSAntiAliasing = 0;
int NoVsync = 1;
int Async = 1;
int DoNotWait = 1;
int ExtraTextures = 1;
int CompressTextures = 0;

DWORD FlipFlags = DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC;
DWORD BltRtFlags  = DDBLT_DONOTWAIT | DDBLT_ASYNC;
DWORD BltSurfFlags  = DDBLTFAST_DONOTWAIT;

#define MAX_DRIVERS		64

typedef struct
{
	geBoolean	IsPrimary;
	GUID		Guid;
	char		Name[MAX_DRIVER_NAME];
} D3D_DRIVER;

typedef struct
{
	int32		NumDrivers;
	D3D_DRIVER	*Drivers;
} DDEnumInfo;

//================================================================================
//	Local static functions
//================================================================================
static BOOL D3DMain_CreateD3D(void);
static BOOL D3DMain_EnumDevices(void);
static BOOL D3DMain_CreateViewPort(int w, int h);
static BOOL D3DMain_ClearBuffers(void);
static BOOL OutputDriverInfo(const char *Filename, DDMain_D3DDriver *Driver);
static BOOL D3DMain_RememberOldMode(HWND hWnd);
static BOOL D3DMain_SetDisplayMode(HWND hWnd, int w, int h, int bpp, BOOL FullScreen);
static BOOL D3DMain_PickDevice(void);
static BOOL D3DMain_CreateDevice(void);
static BOOL D3DMain_CreateBuffers(void);
static void D3DMain_DestroyBuffers(void);
static BOOL D3DMain_CreateZBuffer(void);
static void D3DMain_DestroyZBuffer(void);
static BOOL D3DMain_RestoreDisplayMode(void);
static BOOL D3DMain_CreateDDFromName(const char *DriverName);
static geBoolean CreateDDFromDriver(D3D_DRIVER *pDriver);
static geBoolean CreateDDFromName(const char *DriverName, const DDEnumInfo *Info);

BOOL D3DMain_RestoreAllSurfaces(void)
{
	HRESULT	ddrval;

#ifdef _DEBUG
	OutputDebugString("--- D3DMain_RestoreAllSurfaces ---\n");
#endif
	
	if (AppInfo.lpDD)
	{
		if (!D3DMain_SetDisplayMode(AppInfo.hWnd, AppInfo.CurrentWidth, AppInfo.CurrentHeight, AppInfo.CurrentBpp, AppInfo.FullScreen))
			return FALSE;

		// Restore all the surfaces
		ddrval = AppInfo.lpDD->RestoreAllSurfaces();

		if(ddrval!=DD_OK)
		{
			D3DMain_Log("D3DMain_RestoreAllSurfaces: AppInfo.lpDD->RestoreAllSurfaces() failed:\n  %s\n", D3DErrorToString(ddrval));
			return	FALSE;
		}
	}

	// Force an update in the cache system
	if (TextureCache)
		if (!D3DCache_EvictAllSurfaces(TextureCache))
			return FALSE;

	if (LMapCache)
		if (!D3DCache_EvictAllSurfaces(LMapCache))
			return FALSE;

	return TRUE;
}

//================================================================================
//	BPPToDDBD
//	Convert an integer bit per pixel number to a DirectDraw bit depth flag
//================================================================================
static DWORD BPPToDDBD(int bpp)
{
	switch(bpp) 
	{
		case 1:
			return DDBD_1;
		case 2:
			return DDBD_2;
		case 4:
			return DDBD_4;
		case 8:
			return DDBD_8;
		case 16:
			return DDBD_16;
		case 24:
			return DDBD_24;
		case 32:
			return DDBD_32;
		default:
			assert(!"BOGUS bpp");
	}	

	return DDBD_1;		// Shutup compiler warning
}

//================================================================================
//	D3DMain_InitD3D
//	Does all what is needed to get an app ready to go at a specified with height
//	NOTE - It only makes 16 bit modes availible
//================================================================================
BOOL D3DMain_InitD3D(HWND hWnd, const char *DriverName, int32 Width, int32 Height)
{
	HRESULT			LastError;
	SYSTEMTIME		Time;

	memset(&AppInfo, 0, sizeof(App_Info));
	
	GetSystemTime(&Time);
		
	unlink(D3DMAIN_LOG_FILENAME);

	D3DMain_Log("=================================================================\n");
	D3DMain_Log(" D3DDrv v%d.%d\n", DRV_VERSION_MAJOR, DRV_VERSION_MINOR);
	D3DMain_Log(" Build Date: "__DATE__", Time: "__TIME__"\n");
	D3DMain_Log("=================================================================\n\n");

	D3DMain_Log("Current Date: %02d-%02d-%4d\n", Time.wMonth, Time.wDay, Time.wYear);
	D3DMain_Log("Current Time: %02d:%02d:%02d\n", Time.wHour, Time.wMinute, Time.wSecond);
	D3DMain_Log("\n ** D3D Driver Initializing **\n\n");

	AppInfo.hWnd = hWnd;

	// Create DD
	ATTEMPT(D3DMain_CreateDDFromName(DriverName));
	
	ATTEMPT(D3DMain_GetTextureMemory());

	// We must do this after the DD object is created!!!
	ATTEMPT(D3DMain_RememberOldMode(hWnd));		// Store old mode

	// Get available fullscreen display modes
	ATTEMPT(D3DMain_EnumDisplayModes());

	// Create D3D, and enum it's devices
	ATTEMPT(D3DMain_CreateD3D());
	ATTEMPT(D3DMain_EnumDevices());

	// start 32 bit changes
	// Open the config file and read the bpp variable
	stream = fopen("D3D24.ini","r");
	if(stream)
	{
		fscanf(stream,"%d",&BPP32);
		fscanf(stream,"%d",&ZbufferD);
		fclose(stream);

/* 12/27/2003 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
		{
		 char lpRetStr[25];
		 DWORD dwRetCount;

		 dwRetCount = GetPrivateProfileString("D3D24", "BPP","16", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 if ( dwRetCount ) BPP32 = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "ZBufferD","16", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 if ( dwRetCount ) ZbufferD = atoi(lpRetStr);

         dwRetCount = GetPrivateProfileString("D3D24", "BBufferCount","2", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 BBufferCount = atoi(lpRetStr);
         if (BBufferCount < 1) BBufferCount = 1;

		 dwRetCount = GetPrivateProfileString("D3D24", "CompressTextures","0", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 CompressTextures = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "FSAntiAliasing","0", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 FSAntiAliasing = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "ExtraTextures","1", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 ExtraTextures = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "NoVsync","1", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 NoVsync = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "Async","1", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 Async = atoi(lpRetStr);

		 dwRetCount = GetPrivateProfileString("D3D24", "DoNotWait","1", (LPTSTR) &lpRetStr,sizeof(lpRetStr), ".\\D3D24.INI");
		 DoNotWait = atoi(lpRetStr);

         if (!NoVsync) FlipFlags &= ~DDFLIP_NOVSYNC;
		 if (!Async) BltRtFlags  &= ~DDBLT_ASYNC;

		 if (!DoNotWait)
		 {
          FlipFlags &= ~DDFLIP_DONOTWAIT;
		  BltRtFlags  &= ~DDBLT_DONOTWAIT;
          BltSurfFlags  &= ~DDBLTFAST_DONOTWAIT;

          FlipFlags |= DDFLIP_WAIT;
          BltRtFlags |= DDBLT_WAIT;
		  BltSurfFlags  |= DDBLTFAST_WAIT; 
		 }

		}

	}
	else
	{
		BPP32 = 16;
		ZbufferD = 16;
	}

	// Set our global width and height to the real width and height
	gWidth = Width;
	gHeight = Height;

	if (Width == -1 && Height == -1)		// Window Mode
	{
		// Force Width/Height to client window area size
		Width = AppInfo.OldWindowWidth;
		Height = AppInfo.OldWindowHeight;

//IT APPEARS YOU MUST KEEP THE DESKTOP BPP INSTEAD OF TRYING TO SET IT TO SOMETHING ELSE...		
		ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, AppInfo.OldBpp, FALSE));

/* 01/11/2003 Wendell Buckner
    Force to bpp to desktop bpp... */
        BPP32 =	AppInfo.OldBpp;

	}
	else
	{
		ATTEMPT(D3DMain_SetDisplayMode(hWnd, Width, Height, BPP32, TRUE));
	}
// end 32 bit changes


	// Pick a device we will be happy with
	ATTEMPT(D3DMain_PickDevice());

	// Create front/back buffer
	ATTEMPT(D3DMain_CreateBuffers());		
	
	// For some reason, we have to create the zbuffer BEFORE the device??? Why???
	ATTEMPT(D3DMain_CreateZBuffer());

	// Create the device and viewport
	ATTEMPT(D3DMain_CreateDevice());
	ATTEMPT(D3DMain_CreateViewPort(Width, Height));

	// Get the surface formats for textures, and 2d surfaces
	ATTEMPT(D3DMain_GetSurfaceFormats());

#if 0		// For selective debugging
	AppInfo.CanDoMultiTexture = GE_FALSE;
#else
	AppInfo.CanDoMultiTexture = (AppInfo.Drivers[AppInfo.CurrentDriver].MaxSimultaneousTextures > 1) ? GE_TRUE : GE_FALSE;
#endif

	D3DMain_Log("--- D3DMain_SetRenderState --- \n");
    
	// Set some defaults render states
	LastError = AppInfo.lpD3DDevice->BeginScene();

	if (LastError != D3D_OK)
	{
		D3DMain_Log("D3DMain_InitD3D:  BeginScene failed.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}

/*   07/16/2000 Wendell Buckner
/*    Convert to Directx7...    
	LastError = AppInfo.lpD3DDevice->SetCurrentViewport(AppInfo.lpD3DViewport);*/
	LastError = AppInfo.lpD3DDevice->SetViewport(AppInfo.lpD3DViewport);
	
	
	if (LastError != D3D_OK)
	{
		D3DMain_Log("D3DMain_InitD3D:  SetViewport failed.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}

	//D3DMain_SetFogEnable(GE_TRUE, 255.0f, 0.0f, 0.0f, 500.0f, 1500.0f);
	D3DMain_SetFogEnable(GE_FALSE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

/* 02/28/2001 Wendell Buckner
	Lighting is on by default so turn it off...*/
    AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

/*	02/08/2002 Wendell Buckner
    Optimization from GeForce_Optimization2.doc
    9.	Do not duplicate render state commands.  Worse is useless renderstates.  Do not set a renderstate unless it is needed. */
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING,FALSE);
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_EXTENTS,FALSE); 

/*	02/08/2002 Wendell Buckner
    Optimization from GeForce_Optimization2.doc
    Lights
    Apps should be sure to turn off per-vertex color material support when appropriate for increased performance.             */
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_COLORVERTEX,FALSE); 

/*	02/08/2002 Wendell Buckner
    Optimization from GeForce_Optimization2.doc
    Lights
	Also turning D3DRENDERSTATE_LOCALVIEWER to false saves transform cycles when exact specular highlights are not necessary.*/
    AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_LOCALVIEWER,FALSE); 

/*  01/13/2003 Wendell Buckner
     Use the geneis d3d api functions else there be problems 
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_GREATEREQUAL);*/
    D3DZFunc (D3DCMP_LESSEQUAL);
    D3DZFunc (D3DCMP_GREATEREQUAL);

// changed QD Shadows
	D3DStencilFunc (D3DCMP_LESSEQUAL);
	D3DStencilFunc (D3DCMP_GREATEREQUAL);
// end change 

	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
    AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);

	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);

/* 12/27/2003 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/* 12/06/2001 Wendell Buckner
    ENHANCEMENT - Allow full-scene anti-aliasing *
    if ( AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dpcLineCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT ) 
     AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_SORTINDEPENDENT ); */
    if ( (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dpcLineCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) &&  FSAntiAliasing )      
     D3DFullSceneAntiAliasing ( TRUE );

#if 0
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, TRUE);
#endif
	
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
    AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);

/* 01/30/2003 Wendell Buckner
    TODO: These are obsolete, I need to find out how to get the mipmaps working....*/
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPNEAREST);
	AppInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR); 

	LastError = AppInfo.lpD3DDevice->EndScene();

	if (LastError != D3D_OK)
	{
		D3DMain_Log("D3DMain_InitD3D:  EndScene failed.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}

	AppInfo.RenderingIsOK = TRUE;

	ATTEMPT(D3DMain_ClearBuffers());
	ATTEMPT(D3DMain_GetTextureMemory());

	if (!THandle_Startup())
		return GE_FALSE;

	D3DViewport (0, 0, Width, Height);
	D3DDepthRange (0.0f, 1.0f);

	D3DMain_Log("\n ** Initialization was successful **\n\n");

	return TRUE;

	exit_with_error:;
		D3DMain_Log(" ** Initialization was NOT successful **\n");
		D3DMain_ShutdownD3D();
		return FALSE;
}

//================================================================================
//	D3DMain_ShutdownD3D
//================================================================================
BOOL D3DMain_ShutdownD3D(void)
{
	D3DMain_Log("\n--- D3DMain_ShutdownD3D ---\n");

	THandle_Shutdown();

	// Destroys all objects including Direct Draw.
	AppInfo.RenderingIsOK = FALSE;

	if (AppInfo.lpD3DViewport)
	{
		assert(AppInfo.lpD3DDevice);

/*  07/16/2000 Wendell Buckner
    Convert to Directx7...    
		AppInfo.lpD3DDevice->DeleteViewport(AppInfo.lpD3DViewport); */
		AppInfo.lpD3DDevice->Clear(0,0,0,0,0,0);

//03/01/2002 Wendell Buckner	
//This wasn't commented in the original code but it was in my genesis 1.0 converted code...
//not sure I should do this.		
//		RELEASE(AppInfo.lpD3DViewport);
	}

	RELEASE(AppInfo.lpD3DDevice);

/* 03/09/2002 Wendell Buckner
   The BackgroundMaterial variable is a copy of lpD3Device, so don't release it just set it to NULL
	RELEASE(AppInfo.BackgroundMaterial);                                                            */
    AppInfo.BackgroundMaterial = NULL;                                                            

	if (AppInfo.lpZBuffer)
	{
		assert(AppInfo.lpBackBuffer);
		AppInfo.lpBackBuffer->DeleteAttachedSurface(0, AppInfo.lpZBuffer);
		RELEASE(AppInfo.lpZBuffer);
	}

	if (AppInfo.lpFrontBuffer)
		AppInfo.lpFrontBuffer->SetClipper(NULL);

	RELEASE(AppInfo.lpClipper);
	RELEASE(AppInfo.lpBackBuffer);
	RELEASE(AppInfo.lpFrontBuffer);

	D3DMain_RestoreDisplayMode();

	RELEASE(AppInfo.lpD3D);
	RELEASE(AppInfo.lpDD);

	memset(&AppInfo, 0, sizeof(App_Info));

	D3DMain_Log("  Shutdown was successful...\n\n");

	return TRUE;
}

extern uint32 CurrentLRU;
//================================================================================
//================================================================================
geBoolean D3DMain_Reset(void)
{
	THandle_Shutdown();
	PCache_Reset();

	if (!THandle_Startup())
		return GE_FALSE;

	CurrentLRU = 0;

	return GE_TRUE;
}

//================================================================================
//	D3DMain_Log
//================================================================================
void D3DMain_Log(LPSTR Str, ... )
{
	char	Buffer[2048];
	FILE	*f;

	wvsprintf(Buffer, Str, (char*)(&Str+1));

	f = fopen(D3DMAIN_LOG_FILENAME, "a+t");

	if (!f)
		return;

	fprintf(f, "%s", Buffer);

	fclose(f);
}

//================================================================================
//	CompareModes
//================================================================================
static int CompareModes(const void* element1, const void* element2) 
{
	App_Mode *lpMode1, *lpMode2;
  
	lpMode1 = (App_Mode*)element1;
	lpMode2 = (App_Mode*)element2;
  
	if (lpMode1->Bpp > lpMode2->Bpp)
		return -1;
	else if (lpMode2->Bpp > lpMode1->Bpp)
		return 1;
	else if (lpMode1->Width > lpMode2->Width)
		return -1;
	else if (lpMode2->Width > lpMode1->Width)
		return 1;
	else if (lpMode1->Height > lpMode2->Height)
		return -1;
	else if (lpMode2->Height > lpMode1->Height)
		return 1;
	else
		return 0;
}

//================================================================================
//	EnumDisplayModesCallback
//================================================================================
static HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC2 pddsd, LPVOID lpContext)
{
	App_Mode		*pMode;

	if (!pddsd)
		return DDENUMRET_OK;
		
	if (pddsd->dwWidth > 1280 || pddsd->dwHeight > 1024)
		return DDENUMRET_OK;

	if (AppInfo.NumModes >= MAX_APP_MODES)
		return DDENUMRET_CANCEL;

	pMode = &AppInfo.Modes[AppInfo.NumModes++];

	// Save this mode at the end of the mode array and increment mode count
	pMode->Width = pddsd->dwWidth;
	pMode->Height = pddsd->dwHeight;
	pMode->Bpp = pddsd->ddpfPixelFormat.dwRGBBitCount;
	pMode->ThisDriverCanDo = FALSE;

	return DDENUMRET_OK;
}

//================================================================================
//	D3DMain_EnumDisplayModes
//================================================================================
BOOL D3DMain_EnumDisplayModes(void)
{
	HRESULT		LastError;

	D3DMain_Log("--- D3DMain_EnumDisplayModes ---\n");

	// Get a list of available display modes from DirectDraw
	AppInfo.NumModes = 0;
  
	LastError = AppInfo.lpDD->EnumDisplayModes(0, NULL, 0, EnumDisplayModesCallback);

	if(LastError != DD_OK ) 
	{
		D3DMain_Log("EnumDisplayModes failed.\n  %s\n", D3DErrorToString(LastError));
		AppInfo.NumModes = 0;
		return FALSE;
	}

	// Sort the list of display modes
	qsort((void *)&AppInfo.Modes[0], (size_t)AppInfo.NumModes, sizeof(App_Mode), CompareModes);
	
	return TRUE;
}



//================================================================================
//	D3DMain_CreateD3D
//================================================================================
static BOOL D3DMain_CreateD3D(void)
{
	HRESULT		LastError;
	
	assert(AppInfo.lpDD);

	D3DMain_Log("--- D3DMain_CreateD3D ---\n");

/*   07/16/2000 Wendell Buckner                                                         
/*    Convert to Directx7...                                                            
	LastError = AppInfo.lpDD->QueryInterface(IID_IDirect3D3, (LPVOID*)&AppInfo.lpD3D);  */
    LastError = AppInfo.lpDD->QueryInterface(IID_IDirect3D7, (LPVOID*)&AppInfo.lpD3D);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Creation of IDirect3D failed.\n  %s\n", D3DErrorToString(LastError));
		goto exit_with_error;
	}

	return TRUE;

	exit_with_error:
		return FALSE;
}
/*
#define MUST_BLEND (D3DPBLENDCAPS_BOTHINVSRCALPHA | \
					D3DPBLENDCAPS_BOTHSRCALPHA | \
					D3DPBLENDCAPS_DESTALPHA | \
					D3DPBLENDCAPS_DESTCOLOR | \
					D3DPBLENDCAPS_INVDESTALPHA | \
					D3DPBLENDCAPS_INVDESTCOLOR | \
					D3DPBLENDCAPS_INVSRCALPHA | \
					D3DPBLENDCAPS_INVSRCCOLOR | \
					D3DPBLENDCAPS_ONE | \
					D3DPBLENDCAPS_SRCALPHA | \
					D3DPBLENDCAPS_SRCALPHASAT | \
					D3DPBLENDCAPS_SRCCOLOR | \
					D3DPBLENDCAPS_ZERO)
*/
#if 0
#define MUST_BLEND_SRC	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_DESTCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#define MUST_BLEND_DEST	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_SRCCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#else
#define MUST_BLEND_SRC	(D3DPBLENDCAPS_SRCALPHA | \
						D3DPBLENDCAPS_DESTCOLOR | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)

#define MUST_BLEND_DEST	(D3DPBLENDCAPS_INVSRCALPHA | \
						D3DPBLENDCAPS_ONE | \
						D3DPBLENDCAPS_ZERO)
#endif

//================================================================================
//	EnumDeviceFunc
//================================================================================

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
static HRESULT WINAPI EnumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription,
                      LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc,
                      LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)*/
static HRESULT WINAPI EnumDeviceFunc(LPSTR lpDeviceDescription,
                      LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpHWDesc,
                      LPVOID lpContext)
{
	DDMain_D3DDriver	*Driver;	
	BOOL				Good;

// D3DMain_Log("%s\n",lpDeviceDescription);
// D3DMain_Log("%s\n",lpDeviceName);

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	if (!lpGuid|| !lpDeviceDescription || !lpDeviceName || !lpHWDesc || !lpHELDesc)
		return (D3DENUMRET_OK);*/
	if (!lpDeviceDescription || !lpDeviceName || !lpHWDesc)
		return (D3DENUMRET_OK);

	if (strlen(lpDeviceDescription) >= MAX_DRIVER_NAME)
		return (D3DENUMRET_OK);

	if (strlen(lpDeviceName) >= MAX_DRIVER_NAME)
		return (D3DENUMRET_OK);

//	D3DMain_Log("--- EnumDeviceFunc ---\n");

	lpContext = lpContext;

	Good = TRUE;

	AppInfo.CurrentDriver = AppInfo.NumDrivers;

	Driver = &AppInfo.Drivers[AppInfo.NumDrivers];
	
	// Record the D3D driver's inforamation
	
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Guid, lpGuid, sizeof(GUID));*/
	memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Guid, &lpHWDesc->deviceGUID, sizeof(GUID));
	lstrcpy(AppInfo.Drivers[AppInfo.NumDrivers].About, lpDeviceDescription);
	lstrcpy(AppInfo.Drivers[AppInfo.NumDrivers].Name, lpDeviceName);

	// Is this a hardware device or software emulation?  Checking the color
	// model for a valid model works.

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	if (lpHWDesc->dcmColorModel)                         */ 
    if (lpHWDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) 
	{
		AppInfo.Drivers[AppInfo.NumDrivers].IsHardware = TRUE;
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
		memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Desc, lpHWDesc, sizeof(D3DDEVICEDESC));*/
		memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Desc, lpHWDesc, sizeof(D3DDEVICEDESC7));
	}
	else	
	{
		// Skip if this is not a hardware driver
		AppInfo.Drivers[AppInfo.NumDrivers].IsHardware = FALSE;

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
		memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Desc, lpHWDec, sizeof(D3DDEVICEDESC));*/
		memcpy(&AppInfo.Drivers[AppInfo.NumDrivers].Desc, lpHWDesc, sizeof(D3DDEVICEDESC7));
		Good = FALSE;
	}

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	LPD3DDEVICEDESC Desc = &AppInfo.Drivers[AppInfo.NumDrivers].Desc; */
	LPD3DDEVICEDESC7 Desc = &AppInfo.Drivers[AppInfo.NumDrivers].Desc;
	

    Driver->MaxTextureBlendStages = Desc->wMaxTextureBlendStages;
    Driver->MaxSimultaneousTextures = Desc->wMaxSimultaneousTextures;	

	if (!(Desc->dwDeviceZBufferBitDepth))
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesZBuffer = TRUE;

	if (!(Desc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE))
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesTextures = TRUE;

	// Skip if it does not support alpha blending
	if (!(Desc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA))
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesAlpha = TRUE;

	if (!(Desc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY))
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesTransparency = TRUE;
		
	//if (!(lpHWDesc->dpcTriCaps.dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP))
	//	Good = FALSE;
	//else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesClamping = TRUE;

	if ((Desc->dpcTriCaps.dwSrcBlendCaps & MUST_BLEND_SRC) != MUST_BLEND_SRC)
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesSrcBlending = TRUE;

	if ((Desc->dpcTriCaps.dwDestBlendCaps & MUST_BLEND_DEST) != MUST_BLEND_DEST)
		Good = FALSE;
	else
		AppInfo.Drivers[AppInfo.NumDrivers].DoesDestBlending = TRUE;

	// Stop as soon as we find a driver that can render into a window
	if ((Desc->dwDeviceRenderBitDepth & BPPToDDBD(AppInfo.OldBpp)) && AppInfo.IsPrimary && Good)
	{
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoWindow = TRUE;
		AppInfo.CanDoWindow = TRUE;
	}
	else
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoWindow = FALSE;

/* 03/25/2003 Wendell Buckner
    BUMPMAPPING */
	if ( (Desc->dwTextureOpCaps & (D3DTEXOPCAPS_BUMPENVMAP | D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)) && (Desc->wMaxTextureBlendStages > 2) )
	{
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoBumpMapping = TRUE;
		AppInfo.CanDoBumpMapping = TRUE;
	}
	else
	{
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoBumpMapping = FALSE;	
		AppInfo.CanDoBumpMapping = FALSE;
	}

/* 02/21/2004 Wendell Buckner
    DOT3 BUMPMAPPING */
	if ( Desc->dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3 )
	{
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoDot3 = TRUE;
	}
	else
	{
		AppInfo.Drivers[AppInfo.NumDrivers].CanDoDot3 = FALSE;	
	}

	// Store if we can use this driver
	AppInfo.Drivers[AppInfo.NumDrivers].CanUse = Good;

	#if 0
		if (AppInfo.LogToFile)
			OutputDriverInfo(D3DMAIN_LOG_FILENAME, &AppInfo.Drivers[AppInfo.NumDrivers]);
	#endif

	if (!Good)
		return (D3DENUMRET_OK);

	// Tell global structure that we found a good device
	AppInfo.FoundGoodDevice = TRUE;
	
	// If all was good, increment the number of drivers
	AppInfo.NumDrivers++;

	if (AppInfo.NumDrivers+1 >= DDMAIN_MAX_D3D_DRIVERS)
		return (D3DENUMRET_CANCEL);

	return (D3DENUMRET_OK);
}

//================================================================================
//	EnumDevices
//================================================================================
static BOOL D3DMain_EnumDevices(void)
{
	HRESULT		LastError;

	D3DMain_Log("--- D3DMain_EnumDevices ---\n");

	AppInfo.NumDrivers = 0;

/*   07/16/2000 Wendell Buckner                                                         
/*    Convert to Directx7...                                                            
	LastError = AppInfo.lpD3D->EnumDevices(EnumDeviceFunc, NULL);                              */
	LastError = AppInfo.lpD3D->EnumDevices( (LPD3DENUMDEVICESCALLBACK7) EnumDeviceFunc, NULL);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Enumeration of drivers failed.\n  %s\n", D3DErrorToString(LastError));
		return FALSE;
	}

	AppInfo.CurrentDriver = 0;

	return TRUE;
}

//================================================================================
//	CreateSurface
//================================================================================
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
static HRESULT CreateSurface(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE4 FAR *lpDDSurface) */
static HRESULT CreateSurface(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE7 FAR *lpDDSurface) 
{
	HRESULT Result;
	
	//if (AppInfo.OnlySystemMemory)
	//	lpDDSurfDesc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	Result = AppInfo.lpDD->CreateSurface(lpDDSurfDesc, lpDDSurface, NULL);
	
	return Result;
}

//================================================================================
//	GetSurfDesc
//================================================================================

/*   07/16/2000 Wendell Buckner                                                         
/*    Convert to Directx7...                                                            
static HRESULT GetSurfDesc(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE4 lpDDSurf)*/
static HRESULT GetSurfDesc(LPDDSURFACEDESC2 lpDDSurfDesc, LPDIRECTDRAWSURFACE7 lpDDSurf)
{
	HRESULT Result;
	
	memset(lpDDSurfDesc, 0, sizeof(DDSURFACEDESC2));
	
	lpDDSurfDesc->dwSize = sizeof(DDSURFACEDESC2);
	
	Result = lpDDSurf->GetSurfaceDesc(lpDDSurfDesc);
	
	return Result;
}

//================================================================================
//	D3DMain_CreateViewPort
//================================================================================

/* 07/16/2000 Wendell Buckner
    I assume a local viewport will go out of scope so use a project static/global one... */
D3DVIEWPORT7 viewData;

static BOOL D3DMain_CreateViewPort(int w, int h)
{
/* 07/16/2000 Wendell Buckner
    There is no CreateViewport and the following calls to setviewport will fail under DX7!
    Convert to Directx7...    	
	LPDIRECT3DVIEWPORT3	lpD3DViewport;*/
	
	HRESULT				rval;
  
	D3DMain_Log("--- D3DMain_CreateViewPort ---\n");

/* 07/16/2000 Wendell Buckner
    There is no CreateViewport and the following calls to setviewport will fail under DX7!
    Convert to Directx7...    

	// Create the D3D viewport object
	rval = AppInfo.lpD3D->CreateViewport(&lpD3DViewport, NULL);

	if (rval != D3D_OK) 
	{
		D3DMain_Log("Create D3D viewport failed.\n  %s\n", D3DErrorToString(rval));
		return FALSE;
	}
	
	// Add the viewport to the D3D device
	rval = AppInfo.lpD3DDevice->AddViewport(lpD3DViewport);
	if (rval != D3D_OK) 
	{
		D3DMain_Log("Add D3D viewport failed.\n  %s\n", D3DErrorToString(rval));
		return FALSE;
	}
	
	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT2 viewData;*/

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	memset(&viewData, 0, sizeof(D3DVIEWPORT2)); */
	memset(&viewData, 0, sizeof(D3DVIEWPORT7));

//	viewData.dwSize = sizeof(D3DVIEWPORT2);
	viewData.dwX = viewData.dwY = 0;
	viewData.dwWidth = w;
	viewData.dwHeight = h;
//	viewData.dvClipX = -1.0f;
//	viewData.dvClipWidth = 2.0f;
//	viewData.dvClipHeight = h * 2.0f / w;
//	viewData.dvClipY = viewData.dvClipHeight / 2.0f;
	viewData.dvMinZ = 0.0f;
	viewData.dvMaxZ = 1.0f;

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	rval = lpD3DViewport->SetViewport2(&viewData);      */
	rval = AppInfo.lpD3DDevice->SetViewport(&viewData);	
	
	if (rval != D3D_OK) 
	{
		D3DMain_Log("SetViewport failed.\n  %s\n", D3DErrorToString(rval));
		return FALSE;
	}

/* I assume a local viewport will go out of scope so use a project static/global one... 
	AppInfo.lpD3DViewport = lpD3DViewport;*/
	AppInfo.lpD3DViewport =(LPD3DVIEWPORT7) &viewData;

/*  // Create the material that will be used for the background
	{
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...           
		D3DMATERIAL			Material;
		D3DMATERIALHANDLE	MatHandle; */
        D3DMATERIAL7		Material;


		// Create the material

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...           
		rval = AppInfo.lpD3D->CreateMaterial(&AppInfo.BackgroundMaterial, NULL );       

		if (rval != D3D_OK)
		{
			D3DMain_Log("D3DMain_CreateViewPort: CreateMaterial failed.\n  %s\n", D3DErrorToString(rval));
			return FALSE;
		} */
        AppInfo.BackgroundMaterial = AppInfo.lpD3DDevice;


		// Fill in the material with the data

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...           
		memset(&Material, 0, sizeof(D3DMATERIAL));
		Material.dwSize       = sizeof(D3DMATERIAL); */
		memset(&Material, 0, sizeof(D3DMATERIAL7));

		Material.dcvDiffuse.r = Material.dcvAmbient.r = 0.0f;
		Material.dcvDiffuse.g = Material.dcvAmbient.g = 0.0f;
		Material.dcvDiffuse.b = Material.dcvAmbient.b = 0.0f;

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...           		
	    Material.dwRampSize   = 16L; // A default ramp size */

	    AppInfo.BackgroundMaterial->SetMaterial(&Material);

		// Attach the material to the viewport

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...           
		AppInfo.BackgroundMaterial->GetHandle( AppInfo.lpD3DDevice, &MatHandle); 
	    AppInfo.lpD3DViewport->SetBackground(MatHandle);                         *
	}*/

	return TRUE;
}

//================================================================================
//	EnumTextureFormatsCallback
//	Record information about each texture format the current D3D driver can
//	support. Choose one as the default format and return it through lpContext.
//================================================================================
static HRESULT CALLBACK EnumTextureFormatsCallback(LPDDPIXELFORMAT lpddpfPixelFormat, LPVOID lpContext)
{
	DDMain_SurfFormat	*pTexFormat;


	if(!lpddpfPixelFormat)
		return DDENUMRET_OK;

/* TODO: It looks like newer cards will support 32-bit bumpmapping support... right now stick with 16-bit and 24-bit since dx7
         examples only these formats */

/* 12/20/2003 Wendell Buckner
    COMPRESSED TEXTURES - Display the four CC codes
/* 04/28/2003 Wendell Buckner
    BUMPMAPPING
	D3DMain_Log("EnumTextureFormatsCallback: %i, A:%x, R:%x, G:%x, B:%x Texture Support Found\n",	lpddpfPixelFormat->dwRGBBitCount, 
																									lpddpfPixelFormat->dwRGBAlphaBitMask, 
																									lpddpfPixelFormat->dwRBitMask, 
																									lpddpfPixelFormat->dwGBitMask, 
																									lpddpfPixelFormat->dwBBitMask); */
/*	D3DMain_Log("EnumTextureFormatsCallback: %i, A:%x, R:%x, G:%x, B:%x / %i, U:%x, V:%x, L:%x / 4CC:%x Texture Support Found\n",	
		                                                                                            lpddpfPixelFormat->dwRGBBitCount, 
																									lpddpfPixelFormat->dwRGBAlphaBitMask, 
																									lpddpfPixelFormat->dwRBitMask, 
																									lpddpfPixelFormat->dwGBitMask, 
																									lpddpfPixelFormat->dwBBitMask,

                                                                                                    lpddpfPixelFormat->dwBumpBitCount,
                                                                                                    lpddpfPixelFormat->dwBumpDuBitMask,
																									lpddpfPixelFormat->dwBumpDvBitMask,
			                                                                                        lpddpfPixelFormat->dwBumpLuminanceBitMask);*/
	D3DMain_Log("EnumTextureFormatsCallback: %i, A:%x, R:%x, G:%x, B:%x / %i, U:%x, V:%x, L:%x / 4CC:%x Texture Support Found\n",	
		                                                                                            lpddpfPixelFormat->dwRGBBitCount, 
																									lpddpfPixelFormat->dwRGBAlphaBitMask, 
																									lpddpfPixelFormat->dwRBitMask, 
																									lpddpfPixelFormat->dwGBitMask, 
																									lpddpfPixelFormat->dwBBitMask,

                                                                                                    lpddpfPixelFormat->dwBumpBitCount,
                                                                                                    lpddpfPixelFormat->dwBumpDuBitMask,
																									lpddpfPixelFormat->dwBumpDvBitMask,
			                                                                                        lpddpfPixelFormat->dwBumpLuminanceBitMask,

																									lpddpfPixelFormat->dwFourCC);



	if (AppInfo.NumTextureFormats+1 >= DDMAIN_MAX_TEXTURE_FORMATS )
	{
		D3DMain_Log("EnumTextureFormatsCallback:  Can't enumerate any more texture formats\n");	
		return DDENUMRET_CANCEL;
	}

	pTexFormat = &AppInfo.TextureFormats[AppInfo.NumTextureFormats];

	// Clear out this texture format slot
	memset(pTexFormat, 0, sizeof(DDMain_SurfFormat));

/* 12/20/2003 Wendell Buckner
    COMPRESSED TEXTURES - Display the four CC codes
    if( lpddpfPixelFormat->dwFlags & DDPF_BUMPDUDV ) 
/* 03/25/2003 Wendell Buckner
    BUMPMAPPING *
	if(lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS) */
	if( lpddpfPixelFormat->dwRGBBitCount == 0 ) 
	{
		switch( lpddpfPixelFormat->dwFourCC )
		{
		 case 0:
			return DDENUMRET_OK;
			break;

		 case FOURCC_DXT1:
			pTexFormat->HasOneBitAlpha = TRUE;
			pTexFormat->HasFourBitAlpha = FALSE;
			pTexFormat->HasEightBitAlpha = FALSE;
			break;

		 case FOURCC_DXT2:
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = TRUE;
			pTexFormat->HasEightBitAlpha = FALSE;
			break;

		 case FOURCC_DXT3:
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = TRUE;
			pTexFormat->HasEightBitAlpha = FALSE;
			break;

		 case FOURCC_DXT4:
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = FALSE;
			pTexFormat->HasEightBitAlpha = TRUE;
            break;

		 case FOURCC_DXT5:
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = FALSE;
			pTexFormat->HasEightBitAlpha = TRUE;
			break;

         default:
            return DDENUMRET_OK; 
			break;
		}
	}
    else if( lpddpfPixelFormat->dwFlags & DDPF_BUMPDUDV ) 
    {

	 if(lpddpfPixelFormat->dwBumpBitCount == 16)
	 {
        if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x000000ff) && 
             (lpddpfPixelFormat->dwBumpDvBitMask        == 0x0000ff00) &&
			 (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x00000000) )
			 {
			  pTexFormat->HasBumpMapSupportNoLuminance = TRUE;
	          pTexFormat->HasBumpMapSupportSixBitLuminance = FALSE;
	          pTexFormat->HasBumpMapSupportEightBitLuminance = FALSE;
			 }
        else if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x0000001f) && 
                  (lpddpfPixelFormat->dwBumpDvBitMask        == 0x000003e0) &&
			      (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x0000fc00) )
             {
              pTexFormat->HasBumpMapSupportNoLuminance = FALSE;
	          pTexFormat->HasBumpMapSupportSixBitLuminance = TRUE;
	          pTexFormat->HasBumpMapSupportEightBitLuminance = FALSE;
		     }
        else return DDENUMRET_OK;
	 }
	 else if(lpddpfPixelFormat->dwBumpBitCount == 24) 
	 {
      if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x000000ff) && 
           (lpddpfPixelFormat->dwBumpDvBitMask        == 0x0000ff00) &&
	       (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x00ff0000) )
           {
	        pTexFormat->HasBumpMapSupportSixBitLuminance = FALSE;
	        pTexFormat->HasBumpMapSupportEightBitLuminance = FALSE;
	        pTexFormat->HasBumpMapSupportEightBitLuminance = TRUE;
		   }
      else return DDENUMRET_OK;
	 }
     else return DDENUMRET_OK;
	}
    else if(lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS) 
	{

/*  04/28/2003 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode... 
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000)     */
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xff000000) 
		{
			if(lpddpfPixelFormat->dwRBitMask != 0xff0000 || 
				lpddpfPixelFormat->dwGBitMask != 0xff00 ||
				lpddpfPixelFormat->dwBBitMask != 0xff)
					return DDENUMRET_OK;
				
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = FALSE;
			pTexFormat->HasEightBitAlpha = TRUE;
		}		

		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000) 
		{
			if(lpddpfPixelFormat->dwRBitMask != 0x7c00 || 
				lpddpfPixelFormat->dwGBitMask != 0x3e0 ||
				lpddpfPixelFormat->dwBBitMask != 0x1f)
					return DDENUMRET_OK;
				
			pTexFormat->HasOneBitAlpha = TRUE;
			pTexFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pTexFormat->HasEightBitAlpha = FALSE;

		}

		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xf000) 
		{
			if(lpddpfPixelFormat->dwRBitMask != 0xf00 || 
				lpddpfPixelFormat->dwGBitMask != 0xf0 ||
				lpddpfPixelFormat->dwBBitMask != 0xf)
					return DDENUMRET_OK;

			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = TRUE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pTexFormat->HasEightBitAlpha = FALSE;

		}

		else
		{
			pTexFormat->HasOneBitAlpha = FALSE;
			pTexFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pTexFormat->HasEightBitAlpha = FALSE;

		}
	}
	else 
	{
		if(!(lpddpfPixelFormat->dwFlags & DDPF_RGB))
			return DDENUMRET_OK;
		
		#if 0
		if(lpddpfPixelFormat->dwRGBBitCount != 16) 
			return DDENUMRET_OK;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xf800 && lpddpfPixelFormat->dwRBitMask != 0x7c00) ||
			(lpddpfPixelFormat->dwGBitMask != 0x7e0 && lpddpfPixelFormat->dwGBitMask != 0x3e0) ||
			(lpddpfPixelFormat->dwBBitMask != 0x1f))
				return DDENUMRET_OK;
		#endif

		pTexFormat->HasOneBitAlpha = FALSE;
		pTexFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
		pTexFormat->HasEightBitAlpha = FALSE;

	}

	// Record the PixelFormat of this texture
	memcpy(&pTexFormat->ddsd.ddpfPixelFormat, lpddpfPixelFormat, sizeof(DDPIXELFORMAT));

	AppInfo.NumTextureFormats++;

	return DDENUMRET_OK;
}

//================================================================================
//	Main_EnumTextureFormats
//	Get a list of available texture map formats from the Direct3D driver by
//	enumeration.  Choose a default format.
//================================================================================
BOOL Main_EnumTextureFormats(void)
{
	HRESULT	LastError;

	assert(AppInfo.lpD3DDevice);

	AppInfo.NumTextureFormats = 0;

	LastError = AppInfo.lpD3DDevice->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
	
	if (LastError != DD_OK) 
	{
		D3DMain_Log("Main_EnumTextureFormats:  Enumeration of texture formats failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}


//================================================================================
//	EnumSurfaceFormatsCallback
//================================================================================
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
HRESULT WINAPI EnumSurfaceFormatsCallback(LPDIRECTDRAWSURFACE4 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext) */
HRESULT WINAPI EnumSurfaceFormatsCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)

{
	LPDDPIXELFORMAT		lpddpfPixelFormat;
	DDMain_SurfFormat	*pSurfFormat;

	// Don't need this.
	RELEASE(lpDDSurface);
	
	lpddpfPixelFormat = &lpDDSurfaceDesc->ddpfPixelFormat;

	if(!lpddpfPixelFormat)
		return DDENUMRET_OK;

	D3DMain_Log("EnumSurfaceFormatsCallback: %i, A:%x, R:%x, G:%x, B:%x Texture Support Found\n",	lpddpfPixelFormat->dwRGBBitCount, 
																									lpddpfPixelFormat->dwRGBAlphaBitMask, 
																									lpddpfPixelFormat->dwRBitMask, 
																									lpddpfPixelFormat->dwGBitMask, 
																									lpddpfPixelFormat->dwBBitMask);




	if (AppInfo.NumSurfFormats+1 >= DDMAIN_MAX_SURFACE_FORMATS )
	{
	 D3DMain_Log("EnumSurfaceFormatsCallback:  Can't enumerate any more surface formats\n");	
	 return DDENUMRET_CANCEL;
    }

	pSurfFormat = &AppInfo.SurfFormats[AppInfo.NumSurfFormats];

	// Clear out this texture format slot
	memset(pSurfFormat, 0, sizeof(DDMain_SurfFormat));

	if(lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS) 
	{

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode... 
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000)      */
		if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xff000000) 
		{
			//8888
			if(lpddpfPixelFormat->dwRBitMask != 0xff0000 || 
				lpddpfPixelFormat->dwGBitMask != 0xff00 ||
				lpddpfPixelFormat->dwBBitMask != 0xff)
					return DDENUMRET_OK;
				
			pSurfFormat->HasOneBitAlpha = FALSE;
			pSurfFormat->HasFourBitAlpha = FALSE;
			pSurfFormat->HasEightBitAlpha = TRUE;
		}	
		
		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0x8000) 
		{
			// 1555
			if(lpddpfPixelFormat->dwRBitMask != 0x7c00 || 
				lpddpfPixelFormat->dwGBitMask != 0x3e0 ||
				lpddpfPixelFormat->dwBBitMask != 0x1f)
					return DDENUMRET_OK;
				
			pSurfFormat->HasOneBitAlpha = TRUE;
			pSurfFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pSurfFormat->HasEightBitAlpha = FALSE;
		}

		else if(lpddpfPixelFormat->dwRGBAlphaBitMask == 0xf000) 
		{
			// 4444
			if(lpddpfPixelFormat->dwRBitMask != 0xf00 || 
				lpddpfPixelFormat->dwGBitMask != 0xf0 ||
				lpddpfPixelFormat->dwBBitMask != 0xf)
					return DDENUMRET_OK;

			pSurfFormat->HasOneBitAlpha = FALSE;
			pSurfFormat->HasFourBitAlpha = TRUE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pSurfFormat->HasEightBitAlpha = FALSE;
		}

		else
		{
			pSurfFormat->HasOneBitAlpha = FALSE;
			pSurfFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
			pSurfFormat->HasEightBitAlpha = FALSE;
		}
	}
	else 
	{
		if(!(lpddpfPixelFormat->dwFlags & DDPF_RGB))
			return DDENUMRET_OK;

		pSurfFormat->HasOneBitAlpha = FALSE;
		pSurfFormat->HasFourBitAlpha = FALSE;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
		pSurfFormat->HasEightBitAlpha = FALSE;
	}

	// Record the PixelFormat of this texture
	memcpy(&pSurfFormat->ddsd.ddpfPixelFormat, lpddpfPixelFormat,sizeof(DDPIXELFORMAT));

	AppInfo.NumSurfFormats++;

	return DDENUMRET_OK;
}
  
//================================================================================
//	Main_EnumSurfaceFormats
//================================================================================
BOOL Main_EnumSurfaceFormats(void)
{
	HRESULT		LastError;

	assert(AppInfo.lpDD);

	AppInfo.NumSurfFormats = 0;

	LastError = AppInfo.lpDD->EnumSurfaces(DDENUMSURFACES_DOESEXIST|DDENUMSURFACES_ALL,
		NULL, NULL, EnumSurfaceFormatsCallback);
	
	if (LastError != DD_OK) 
	{
		D3DMain_Log("Main_EnumSurfaceFormats:  Enumeration of texture formats failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
// Name: EnumZBufferFormatsCallback()
// Desc: Enumeration function to report valid pixel formats for z-buffers.
//-----------------------------------------------------------------------------
static HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf,
                                                  VOID* pddpfDesired )
{
	unsigned long UserZBufferD = ZbufferD;

    if( NULL==pddpf || NULL==pddpfDesired )
        return D3DENUMRET_CANCEL;

    // If the current pixel format's match the desired ones (DDPF_ZBUFFER and
    // possibly DDPF_STENCILBUFFER), lets copy it and return. This function is
    // not choosy...it accepts the first valid format that comes along.
    if( pddpf->dwFlags == ((DDPIXELFORMAT*)pddpfDesired)->dwFlags )
    {
        memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );

// We're happy with a 16-bit z-buffer. Otherwise, keep looking.

/* 01/11/2003 Wendell Buckner
    The driver did not take into account the ZBufferD and allways created a 16-bit z- buffer 
		if( pddpf->dwZBufferBitDepth == 16 )
			return D3DENUMRET_CANCEL;        */

// changed QD Shadows
		if(pddpf->dwStencilBitDepth>=8)
		{
			AppInfo.Drivers[AppInfo.CurrentDriver].CanDoStencil = TRUE;
			D3DMain_Log("EnumZBufferFormats:  StencilBitDepth>=8... can do 8bit stencil buffer\n");
		}
		else
		{
			AppInfo.Drivers[AppInfo.CurrentDriver].CanDoStencil = FALSE;
			D3DMain_Log("EnumZBufferFormats:  StencilBitDepth<8... not enough stencil bits\n");
		}
// end change

		if( pddpf->dwZBufferBitDepth == UserZBufferD )
		{

			return D3DENUMRET_CANCEL;
		}
    }

    return D3DENUMRET_OK;
}

//================================================================================
//	D3DMain_ClearBuffers
//================================================================================
static BOOL D3DMain_ClearBuffers(void)
{
	DDSURFACEDESC2	ddsd;
	RECT			dst;
	DDBLTFX			ddbltfx;
	HRESULT			LastError;

	// Find the width and height of the front buffer by getting its
	// DDSURFACEDESC2
	if (AppInfo.lpFrontBuffer) 
	{
		LastError = GetSurfDesc(&ddsd, AppInfo.lpFrontBuffer);
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_ClearBuffers:  Failure getting the surface description of the front buffer before clearing.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
    
		// Clear the front buffer to black
		memset(&ddbltfx, 0, sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(DDBLTFX);
		SetRect(&dst, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

/* 01/03/2004 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/*	01/24/2002 Wendell Buckner
    Change flags for speed...		
		LastError = AppInfo.lpFrontBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, 
							&ddbltfx);*
		LastError = AppInfo.lpFrontBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_DONOTWAIT | DDBLT_ASYNC, 
							&ddbltfx);		*/
		LastError = AppInfo.lpFrontBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | BltRtFlags, 
							&ddbltfx);		
    
		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("D3DMain_ClearBuffers:  D3DMain_RestoreAllSurfaces failed...\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("D3DMain_ClearBuffers:  Clearing the front buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}
	
	if (AppInfo.lpBackBuffer) 
	{
		// Find the width and height of the back buffer by getting its
		// DDSURFACEDESC2
		
		LastError = GetSurfDesc(&ddsd, AppInfo.lpBackBuffer);
		
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_ClearBuffers:  Failure while getting the surface description of the back buffer before clearing.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
		
		// Clear the back buffer to black
		memset(&ddbltfx, 0, sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(DDBLTFX);
		SetRect(&dst, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

/* 01/03/2004 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/*	01/24/2002 Wendell Buckner
    Change flags for speed...
		LastError = AppInfo.lpBackBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
					&ddbltfx);                                                                *
		LastError = AppInfo.lpBackBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_DONOTWAIT | DDBLT_ASYNC,
					&ddbltfx); */
		LastError = AppInfo.lpBackBuffer->Blt(&dst, NULL, NULL, DDBLT_COLORFILL | BltRtFlags,
					&ddbltfx);

		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("D3DMain_ClearBuffers:  D3DMain_RestoreAllSurfaces failed.\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("D3DMain_ClearBuffers:  Clearing the back buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}
	
	return TRUE;
}

//================================================================================
//	Main_ShowBackBuffer
//================================================================================
BOOL Main_ShowBackBuffer(void)
{
	HRESULT		LastError;

	if (!AppInfo.RenderingIsOK) 
		return TRUE;
  
	if (AppInfo.FullScreen) 
	{  
		// Flip the back and front buffers
		#if 1

/* 01/03/2004 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/*	01/24/2002 Wendell Buckner
    Change flags for speed...		
			LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, DDFLIP_WAIT);*
			LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC );*/
            LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, FlipFlags );

		#else

/* 01/03/2004 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/*	01/24/2002 Wendell Buckner
    Change flags for speed...		
			LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, DDFLIP_NOVSYNC);*
			LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC ); */
			LastError = AppInfo.lpFrontBuffer->Flip(AppInfo.lpBackBuffer, FlipFlags );

		#endif
		
		if (LastError == DDERR_SURFACELOST) 
		{
			D3DMain_RestoreAllSurfaces();
			//AppInfo.lpFrontBuffer->Restore();
			//AppInfo.lpBackBuffer->Restore();
			
			D3DMain_ClearBuffers();

		} 
		else if (LastError == DDERR_WASSTILLDRAWING)
		{
		}
		else if (LastError != DD_OK) 
		{
			D3DMain_Log("Flipping complex display surface failed.\n  %s\n", D3DErrorToString(LastError));
			return FALSE;
		}
	} 
	else 
	{
		RECT	FRect, BRect;

		FRect.left = AppInfo.WindowXOffset;
		FRect.right = FRect.left + AppInfo.CurrentWidth;
		FRect.top = AppInfo.WindowYOffset;
		FRect.bottom = FRect.top + AppInfo.CurrentHeight;

		BRect.left = 0;
		BRect.right = AppInfo.CurrentWidth;
		BRect.top = 0;
		BRect.bottom = AppInfo.CurrentHeight;

/* 01/03/2004 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
/*  03/10/2002 Wendell Buckner
     Optimization from GeForce_Optimization2.doc                                        
     Procedural Textures
     Also, Load is even faster than BLT under Dx7.
 *	01/24/2002 Wendell Buckner
     Change flags for speed...
		LastError = AppInfo.lpFrontBuffer->Blt(&FRect, AppInfo.lpBackBuffer,
				&BRect, DDBLT_WAIT, NULL);                                   *
		LastError = AppInfo.lpFrontBuffer->Blt(&FRect, AppInfo.lpBackBuffer,
				&BRect, DDBLT_DONOTWAIT | DDBLT_ASYNC, NULL);                */
		LastError = AppInfo.lpFrontBuffer->Blt(&FRect, AppInfo.lpBackBuffer,
				&BRect, BltRtFlags, NULL);                
  
		if (LastError != DD_OK) 
		{
			if(LastError==DDERR_SURFACELOST)
			{
				if (!D3DMain_RestoreAllSurfaces())
				{
					D3DMain_Log("Main_ShowBackBuffer:  D3DMain_RestoreAllSurfaces.\n");
					return FALSE;
				}
			}
			else
			{
				D3DMain_Log("Main_ShowBackBuffer:  Blt of back buffer to front buffer failed.\n  %s\n", D3DErrorToString(LastError));
				return FALSE;
			}
		}
	}

	return TRUE;
}

__inline DWORD F2DW(float f)
{
   DWORD            retval = 0;

   _asm {
      fld            f
      lea            eax, [retval]
      fistp         dword ptr[eax]
   }

   return retval;
}


//================================================================================
//	Main_ClearBackBuffer
//================================================================================
// changed QD Shadows
//BOOL Main_ClearBackBuffer(BOOL Clear, BOOL ClearZ)
BOOL Main_ClearBackBuffer(BOOL Clear, BOOL ClearZ, BOOL ClearStencil)
{
    int			ClearFlags;
    D3DRECT		Dummy;
	HRESULT		LastError;

	if (!AppInfo.RenderingIsOK) 
		return TRUE;
	
    // Default to clear nothing
	ClearFlags = 0;

	// Then set in what callers wants to clear
	if (Clear)
		ClearFlags |= D3DCLEAR_TARGET;
	
	if (ClearZ)
		ClearFlags |= D3DCLEAR_ZBUFFER;

// changed QD Shadows
	if (ClearStencil)
		ClearFlags |= D3DCLEAR_STENCIL;
// end change

    Dummy.x1 = Dummy.y1 = 0;
    Dummy.x2 = AppInfo.CurrentWidth;
    Dummy.y2 = AppInfo.CurrentHeight;
 
/* 03/17/2003 Wendell Buckner
	Fog under dx6 would clear use the viewport interface which would automatically use the material color to clear the target
	dx7 doesn't support this... you must now clear the target using clear and passing the color you want to clear it to. 
/* 07/16/2000 Wendell Buckner
	Convert to Directx7... /
	LastError = AppInfo.lpD3DViewport->Clear(1, &Dummy, ClearFlags);

	POTENTIAL GOTCHA 
	Need to understand this value => 1.0f better, the original code above has it set to 0 but my
	sample code won't work with this value set to 0 *
	LastError = AppInfo.lpD3DDevice->Clear( 1, &Dummy, ClearFlags, 0, 1.0f, 0L ); */
	if (AppInfo.FogEnable) 
		LastError = AppInfo.lpD3DDevice->Clear( 1, &Dummy, ClearFlags, (F2DW(AppInfo.FogR)<<16)|(F2DW(AppInfo.FogG)<<8)|F2DW(AppInfo.FogB), 1.0f, 0L );
	else
		LastError = AppInfo.lpD3DDevice->Clear( 1, &Dummy, ClearFlags, (F2DW(AppInfo.ClearR)<<16)|(F2DW(AppInfo.ClearG)<<8)|F2DW(AppInfo.ClearB), 1.0f, 0L ); 


	if (LastError != D3D_OK) 
	{
		D3DMain_Log("Viewport clear failed.\n  %s\n",
			D3DErrorToString(LastError));
		return FALSE;
	}

	return TRUE;
}

//================================================================================
//	Surface manipulation
//================================================================================

typedef struct
{
	unsigned char r, g, b;
} MY_D3D_RGB;

typedef struct
{
	DWORD	R_Shift;
	DWORD	G_Shift;
	DWORD	B_Shift;
	DWORD	A_Shift;

	DWORD	R_Mask;
	DWORD	G_Mask;
	DWORD	B_Mask;
	DWORD	A_Mask;

	DWORD	R_Width;
	DWORD	G_Width;
	DWORD	B_Width;
	DWORD	A_Width;
} D3D_PixelMask;

//================================================================================
//	GetSurfacePixelMask
//================================================================================
static void GetSurfacePixelMask(DDSURFACEDESC2 *ddsd, D3D_PixelMask *PixelMask)
{
	DWORD	red_mask, grn_mask, blu_mask, a_mask;
	DWORD	red_shift, grn_shift, blu_shift, a_shift;
	DWORD	red_width, grn_width, blu_width, a_width;
	int		i;

	red_mask = ddsd->ddpfPixelFormat.dwRBitMask;
	grn_mask = ddsd->ddpfPixelFormat.dwGBitMask;
	blu_mask = ddsd->ddpfPixelFormat.dwBBitMask;
	a_mask = ddsd->ddpfPixelFormat.dwRGBAlphaBitMask;

	//
	// Derive shift, width values from masks
	//

	for (i=31; i >= 0; i--)
	{
		if (red_mask & (1 << i))
			red_shift = i;

		if (grn_mask & (1 << i))
			grn_shift = i;

		if (blu_mask & (1 << i))
			blu_shift = i;

		if (a_mask & (1 << i))
			a_shift = i;
	}

	for (i=0; i <= 31; i++)
	{
		if (red_mask & (1 << i))
			red_width = i - red_shift + 1;

		if (grn_mask & (1 << i))
			grn_width = i - grn_shift + 1;

		if (blu_mask & (1 << i))
			blu_width = i - blu_shift + 1;

		if (a_mask & (1 << i))
			a_width = i - a_shift + 1;
	}
	//
	// Pass all requested values back to the caller
	//

	PixelMask->R_Shift = red_shift;
	PixelMask->G_Shift = grn_shift;
	PixelMask->B_Shift = blu_shift;
	PixelMask->A_Shift = a_shift;

	PixelMask->R_Mask  = red_mask;
	PixelMask->G_Mask  = grn_mask;
	PixelMask->B_Mask  = blu_mask;
	PixelMask->A_Mask  = a_mask;

	PixelMask->R_Width = red_width;
	PixelMask->G_Width = grn_width;
	PixelMask->B_Width = blu_width;
	PixelMask->A_Width = a_width;
}

//================================================================================
//	MyRGB
//================================================================================
static unsigned int MyRGB(DWORD R, DWORD G, DWORD B, D3D_PixelMask *PixelMask)
{
   DWORD       R_Left, G_Left, B_Left;
   DWORD       R_Right, G_Right, B_Right;
   

   // Get shift constants for current video mode
   R_Left = PixelMask->R_Shift;
   G_Left = PixelMask->G_Shift;
   B_Left = PixelMask->B_Shift;

   R_Right = 8 - PixelMask->R_Width;
   G_Right = 8 - PixelMask->G_Width;
   B_Right = 8 - PixelMask->B_Width;
   // Shift R,G, and B into one value
   return( 
       (((((unsigned int) R) >> R_Right) << R_Left) & PixelMask->R_Mask) |
       (((((unsigned int) G) >> G_Right) << G_Left) & PixelMask->G_Mask) |
       (((((unsigned int) B) >> B_Right) << B_Left) & PixelMask->B_Mask)
   );
}

//==========================================================================================
//	D3DMain_GetSurfaceFormats
//==========================================================================================
BOOL D3DMain_GetSurfaceFormats(void)
{
	int32		i;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
	BOOL HasEightBitAlpha = FALSE;

    D3DMain_Log("--- D3DMain_GetSurfaceFormats ---\n");
	
	if (!Main_EnumTextureFormats())
	{
        D3DMain_Log("D3DMain_GetSurfaceFormats:  Main_EnumTextureFormats failed.\n");
		return FALSE;
    }
	
	if (!Main_EnumSurfaceFormats())
	{
        D3DMain_Log("D3DMain_GetSurfaceFormats:  Main_EnumSurfaceFormats failed.\n");
		return FALSE;
	}

/* 01/05/2003 Wendell Buckner
    Set the memory to nulls so we can check against it later to see if it's available... */	
    memset(&AppInfo.ddTexFormat24, 0, sizeof(DDSURFACEDESC2));
	memset(&AppInfo.ddTexFormat32, 0, sizeof(DDSURFACEDESC2));

#if 1
	for(i = 0; i < AppInfo.NumSurfFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

		lpddpfPixelFormat = &AppInfo.SurfFormats[i].ddsd.ddpfPixelFormat;

		if(lpddpfPixelFormat->dwRGBBitCount != AppInfo.ddsd.ddpfPixelFormat.dwRGBBitCount)
			continue;

        if (lpddpfPixelFormat->dwRGBAlphaBitMask != AppInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
			continue;
		if (lpddpfPixelFormat->dwRBitMask != AppInfo.ddsd.ddpfPixelFormat.dwRBitMask)
			continue;
        if (lpddpfPixelFormat->dwGBitMask != AppInfo.ddsd.ddpfPixelFormat.dwGBitMask)
			continue;
        if (lpddpfPixelFormat->dwBBitMask != AppInfo.ddsd.ddpfPixelFormat.dwBBitMask)
			continue;

	#if 0	// For debugging (This is the surface it is going to use for 2d decals)
		D3DMain_Log("Bits: %i, A:%x, R:%x, G:%x, B:%x\n",	AppInfo.ddsd.ddpfPixelFormat.dwRGBBitCount, 
															AppInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwRBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwGBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwBBitMask);
		return FALSE;
	#endif

/* 12/28/2002 Wendell Buckner
    A 2-d surface is different than a 3d-surface, some cards don't support the same surfaces in 3-d as in 2-d, so don't force the 2-d
    surface to be like the 3-d one*/
		AppInfo.ddSurfFormat = AppInfo.SurfFormats[i].ddsd;

		break;
	}

    if(i == AppInfo.NumSurfFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a 2d surface format that matches current bit depth.\n");
        return FALSE;
    }

#else
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

		lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		if(lpddpfPixelFormat->dwRGBBitCount != AppInfo.ddsd.ddpfPixelFormat.dwRGBBitCount)
			continue;

        if (lpddpfPixelFormat->dwRGBAlphaBitMask != AppInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
			continue;
		if (lpddpfPixelFormat->dwRBitMask != AppInfo.ddsd.ddpfPixelFormat.dwRBitMask)
			continue;
        if (lpddpfPixelFormat->dwGBitMask != AppInfo.ddsd.ddpfPixelFormat.dwGBitMask)
			continue;
        if (lpddpfPixelFormat->dwBBitMask != AppInfo.ddsd.ddpfPixelFormat.dwBBitMask)
			continue;

	#if 0	// For debugging (This is the surface it is going to use for 2d decals)
		D3DMain_Log("Bits: %i, A:%x, R:%x, G:%x, B:%x\n",	AppInfo.ddsd.ddpfPixelFormat.dwRGBBitCount, 
															AppInfo.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwRBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwGBitMask, 
															AppInfo.ddsd.ddpfPixelFormat.dwBBitMask);
		return FALSE;
	#endif

		
		AppInfo.ddSurfFormat = AppInfo.TextureFormats[i].ddsd;

		break;
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a 2d surface format that matches current bit depth.\n");
        return FALSE;
    }
#endif

	// Now get the 3d surface formats

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
    // Get 8888
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
        if(AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) 
		{
            AppInfo.ddEightBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...   */
			AppInfo.ddTexFormat32  = AppInfo.TextureFormats[i].ddsd;
			HasEightBitAlpha = TRUE;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 8888 (32-bit) texture support.\n");

/* 01/05/2003 Wendell Buckner 
    Only abort if the base 16-bit texture/surface support doesn't exist...
        return FALSE;*/
    }

	// Get 1555
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
        if(AppInfo.TextureFormats[i].HasOneBitAlpha == TRUE) 
		{
            AppInfo.ddOneBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 1555 (16-bit) texture support.\n");
        return FALSE;
    }
    
    // Get 4444
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
        if(AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
		{
            AppInfo.ddFourBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 4444 (16-bit) texture support.\n");
        return FALSE;
    }

/* 12/28/2002 Wendell Buckner
    Give out the true 24-bit surface as well...  */
	// Get a 888
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

/* 01/04/2003 Wendell Buckner
    This variable was never initialized?, I know I've done complete recompiles why hasn't this shown up before? */
        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if(AppInfo.TextureFormats[i].HasOneBitAlpha == TRUE)
			continue;

		if (AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
			continue;

		if (AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) 
			continue;

		// For now, force 3d textures with RGB only info to be either 565 or 555
		// We could enum all formats and let the caller pick between several different RGB formats...
		if (lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS)
			continue;		// We don't want any surface that has alpha, just pure RGB...

		if(lpddpfPixelFormat->dwRGBBitCount != 24) 
			continue;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xff0000) ||
			(lpddpfPixelFormat->dwGBitMask != 0xff00) ||
			(lpddpfPixelFormat->dwBBitMask != 0xff))
				continue;

		AppInfo.ddTexFormat24 = AppInfo.TextureFormats[i].ddsd;

		break;
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a 888 (24-bit) texture support.\n");

/* 01/05/2003 Wendell Buckner 
    Only abort if the base 16-bit texture/surface support doesn't exist...
		return FALSE;*/
    }

/* 12/28/2002 Wendell Buckner
    Give out the 24-bit surface as well...  (this is really 32-bit mode) */
	// Get a X888
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

/* 01/04/2003 Wendell Buckner
    This variable was never initialized?, I know I've done complete recompiles why hasn't this shown up before? */
        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if(AppInfo.TextureFormats[i].HasOneBitAlpha == TRUE)
			continue;

		if (AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
			continue;

		if (AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) 
			continue;

		// For now, force 3d textures with RGB only info to be either 565 or 555
		// We could enum all formats and let the caller pick between several different RGB formats...
		if (lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS)
			continue;		// We don't want any surface that has alpha, just pure RGB...

		if(lpddpfPixelFormat->dwRGBBitCount != 32) 
			continue;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xff0000) ||
			(lpddpfPixelFormat->dwGBitMask != 0xff00) ||
			(lpddpfPixelFormat->dwBBitMask != 0xff) )
				continue;

		AppInfo.ddTexFormat24 = AppInfo.TextureFormats[i].ddsd;

		break;
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find a X888 (32-bit) texture support.\n");

/* 01/05/2003 Wendell Buckner 
    Only abort if the base 16-bit texture/surface support doesn't exist...
		return FALSE;*/
    }

	// Get either 555, or 565.
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

/* 01/04/2003 Wendell Buckner
    This variable was never initialized?, I know I've done complete recompiles why hasn't this shown up before? */
        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if(AppInfo.TextureFormats[i].HasOneBitAlpha == TRUE)
			continue;

		if (AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
			continue;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
		if (AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) 
			continue;

		// For now, force 3d textures with RGB only info to be either 565 or 555
		// We could enum all formats and let the caller pick between several different RGB formats...
		if (lpddpfPixelFormat->dwFlags & DDPF_ALPHAPIXELS)
			continue;		// We don't want any surface that has alpha, just pure RGB...

		if(lpddpfPixelFormat->dwRGBBitCount != 16) 
			continue;

		if(	(lpddpfPixelFormat->dwRBitMask != 0xf800 && lpddpfPixelFormat->dwRBitMask != 0x7c00) ||
			(lpddpfPixelFormat->dwGBitMask != 0x7e0 && lpddpfPixelFormat->dwGBitMask != 0x3e0) ||
			(lpddpfPixelFormat->dwBBitMask != 0x1f))
				continue;

/* 12/28/2002 Wendell Buckner
    This surface will be FORCED to 32-bit (ARGB) at the end of this routine if it is available...  */
		// This is it
		AppInfo.ddTexFormat = AppInfo.TextureFormats[i].ddsd;

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
		AppInfo.ddTexFormat16 = AppInfo.TextureFormats[i].ddsd;

		break;
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 555 or 565 (16-bit) texture support.\n");
		return FALSE;
    }

/* 12/28/2002 Wendell Buckner
    BUMPMAPPING */
    memset(&AppInfo.ddBumpMapNoLuminance , 0, sizeof(DDSURFACEDESC2));
	memset(&AppInfo.ddBumpMapSixBitLuminance, 0, sizeof(DDSURFACEDESC2));
	memset(&AppInfo.ddBumpMapEightBitLuminance, 0, sizeof(DDSURFACEDESC2));

	// Get all bump map surfaces: 88, 556, & 888 surfaces.

	// Get bump map surface: 88
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		if( (lpddpfPixelFormat->dwBumpBitCount != 16)) 
			continue;

		if ( (AppInfo.TextureFormats[i].HasBumpMapSupportNoLuminance == FALSE))
			continue;

        if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x000000ff) && 
             (lpddpfPixelFormat->dwBumpDvBitMask        == 0x0000ff00) &&
			 (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x00000000) )
		{
         AppInfo.ddBumpMapNoLuminance = AppInfo.TextureFormats[i].ddsd;    
		 break;
		} 
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 88 (16-bit) bump map support.\n");
    } 


	// Get bump map surface: 556
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		if( (lpddpfPixelFormat->dwBumpBitCount != 16)) 
			continue;

		if ( (AppInfo.TextureFormats[i].HasBumpMapSupportSixBitLuminance == FALSE) )
			continue;

        if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x0000001f) && 
             (lpddpfPixelFormat->dwBumpDvBitMask        == 0x000003e0) &&
			 (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x0000fc00) )               
		{
         AppInfo.ddBumpMapSixBitLuminance = AppInfo.TextureFormats[i].ddsd;    
		 break;
		} 
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 556 (16-bit) bump map support.\n");
    } 


	// Get bump map surface: 888
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

		if( (lpddpfPixelFormat->dwBumpBitCount != 24)) 
			continue;

		if ( (AppInfo.TextureFormats[i].HasBumpMapSupportEightBitLuminance == FALSE) )
			continue;

        if ( (lpddpfPixelFormat->dwBumpDuBitMask        == 0x000000ff) && 
             (lpddpfPixelFormat->dwBumpDvBitMask        == 0x0000ff00) &&
			 (lpddpfPixelFormat->dwBumpLuminanceBitMask == 0x00ff0000) )
		{
         AppInfo.ddBumpMapEightBitLuminance = AppInfo.TextureFormats[i].ddsd;    
		 break;
		} 
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 888 (24-bit) bump map support.\n");
    } 

/*   TODO: It looks like newer cards will support 32-bit bumpmapping support... right   
           now stick with 16-bit and 24-bit since dx7 examples only these formats       */

/* 12/28/2002 Wendell Buckner
    A 2-d surface is different than a 3d-surface, some cards don't support the same surfaces in 3-d as in 2-d, so don't force the 2-d
    surface to be like the 3-d one*/

/* 12/28/2002 Wendell Buckner
    Allow/make 32-bit (ARGB) mode the default mode...  */
	if ( HasEightBitAlpha && (BPP32 == 32) ) AppInfo.ddTexFormat =  AppInfo.ddEightBitAlphaSurfFormat;

/* 12/21/2003 Wendell Buckner
    COMPRESSED TEXTURES - Get the compressed surface formats */
    memset(&AppInfo.ddCompressedOneBitAlphaSurfFormat, 0, sizeof(DDSURFACEDESC2));
	memset(&AppInfo.ddCompressedFourBitAlphaSurfFormat, 0, sizeof(DDSURFACEDESC2));
	memset(&AppInfo.ddCompressedEightBitAlphaSurfFormat, 0, sizeof(DDSURFACEDESC2));

	// Get 1555 Compressed
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if (lpddpfPixelFormat->dwRGBBitCount != 0)
			continue;

        if(AppInfo.TextureFormats[i].HasOneBitAlpha == TRUE) 
		{
            AppInfo.ddCompressedOneBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 1555 (16-bit) compressed texture support.\n");
    }
    
    // Get 4444 Compressed
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if (lpddpfPixelFormat->dwRGBBitCount != 0)
			continue;

        if(AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) 
		{
            AppInfo.ddCompressedFourBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 4444 (16-bit) compressed texture support.\n");
    }

    // Get 4444 Compressed - favor DXT3
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if (lpddpfPixelFormat->dwRGBBitCount != 0)
			continue;

        if( (AppInfo.TextureFormats[i].HasFourBitAlpha == TRUE) && (lpddpfPixelFormat->dwFourCC == FOURCC_DXT3) ) 
		{
            AppInfo.ddCompressedFourBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}

    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 4444 (16-bit) compressed (DXT3) texture support.\n");
    }

	// Get 8888 Compressed
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if (lpddpfPixelFormat->dwRGBBitCount != 0)
			continue;

        if(AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) 
		{
            AppInfo.ddCompressedEightBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}
    
    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 8888 (32-bit) compressed texture support.\n");
    }
	
	// Get 8888 Compressed - Favor DXT5
	for(i = 0; i < AppInfo.NumTextureFormats; i++)
	{
		LPDDPIXELFORMAT lpddpfPixelFormat;

        lpddpfPixelFormat = &AppInfo.TextureFormats[i].ddsd.ddpfPixelFormat;

        if (lpddpfPixelFormat->dwRGBBitCount != 0)
			continue;

        if( (AppInfo.TextureFormats[i].HasEightBitAlpha == TRUE) && (lpddpfPixelFormat->dwFourCC == FOURCC_DXT5) ) 
		{
            AppInfo.ddCompressedEightBitAlphaSurfFormat = AppInfo.TextureFormats[i].ddsd;
            break;
        }
	}
    
    if(i == AppInfo.NumTextureFormats) 
	{
		D3DMain_Log("D3DMain_GetSurfaceFormats:  Unable to find 8888 (32-bit) compressed (DXT5) texture support.\n");
    }
	
	Main_BuildRGBGammaTables(1.0f);
	
	return TRUE;
}

//==========================================================================================
//	Main_CheckDD
//	Checks to see if current DD driver has any usable D3D Devices...
//==========================================================================================
BOOL Main_CheckDD(void)
{
	AppInfo.NumDrivers = 0;
	AppInfo.CurrentDriver = 0;
	AppInfo.FoundGoodDevice = FALSE;
	AppInfo.CanDoWindow = FALSE;

	assert(AppInfo.lpDD);
	
	if (!D3DMain_RememberOldMode(GetDesktopWindow()))
		return FALSE;

	memset(AppInfo.Drivers, 0, sizeof(DDMain_D3DDriver)*DDMAIN_MAX_D3D_DRIVERS);

	if (!D3DMain_CreateD3D())
		return FALSE;

	if (!D3DMain_EnumDevices())			// See if we can enumerate at least one good device for this DD Driver
		return FALSE;

	if (!AppInfo.FoundGoodDevice)		// Return FALSE if not...
		return FALSE;

	return TRUE;	// Found at least one!!!
}

//==========================================================================================
//	OutputDriverInfo
//==========================================================================================
static BOOL OutputDriverInfo(const char *FileName, DDMain_D3DDriver *Driver)
{
	FILE			*f;
	SYSTEMTIME		Time;
	char			YesNo[2][10];

	f = fopen(FileName, "a+t");
      
	if (!f)
		return FALSE;

	GetSystemTime(&Time);

	strcpy(YesNo[0], "No\n");
	strcpy(YesNo[1], "Yes\n");
	
	fprintf(f,"=================================================================\n");
	fprintf(f,"Time: %2i:%2i:%2i\n", Time.wHour, Time.wMinute, Time.wSecond);
	fprintf(f,"Date: %2i-%2i-%4i\n", Time.wMonth, Time.wDay, Time.wYear);

	fprintf(f, "DirectDraw Name: \n");
	fprintf(f, "    %s\n", AppInfo.DDName);

	fprintf(f, "D3D Driver Name: \n");
	fprintf(f, "    %s\n", Driver->Name);

	fprintf(f, "D3D Driver Description: \n");
	fprintf(f, "    %s\n", Driver->About);

	fprintf(f, "3D Acceleration        : %s", YesNo[Driver->IsHardware]);
	fprintf(f, "Texture Support        : %s", YesNo[Driver->DoesTextures]);
	fprintf(f, "Transparency Support   : %s", YesNo[Driver->DoesTransparency]);
	fprintf(f, "Alpha Support          : %s", YesNo[Driver->DoesAlpha]);
	fprintf(f, "UV Clamping Support    : %s", YesNo[Driver->DoesClamping]);
	fprintf(f, "Src Blending Support   : %s", YesNo[Driver->DoesSrcBlending]);
	fprintf(f, "Dest Blending Support  : %s", YesNo[Driver->DoesDestBlending]);
	fprintf(f, "Window Support         : %s", YesNo[Driver->CanDoWindow]);
	fprintf(f, "Can Use                : %s", YesNo[Driver->CanUse]);

	fclose(f);

	return TRUE;
}

//==========================================================================================
//	D3DMain_GetTextureMemory
//==========================================================================================
BOOL D3DMain_GetTextureMemory(void)
{

	DDSCAPS2		ddsCaps;
	DWORD			dwTotal;
	DWORD			dwFree;
	HRESULT			Error;

	D3DMain_Log("--- D3DMain_GetTextureMemory ---\n");

	memset(&ddsCaps, 0, sizeof(ddsCaps));

	//ddsCaps.dwSize = sizeof(DDSCAPS2);
	ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

	Error = AppInfo.lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);

	if(Error !=DD_OK)
	{
		D3DMain_Log("Getting DD capabilities failed while checking total video memory.\n  %s\n", D3DErrorToString(Error));
		return FALSE;
	}

	AppInfo.VidMemFree = dwFree;

	D3DMain_Log("  Ram free: %i\n", AppInfo.VidMemFree);

	return TRUE;
}


//==========================================================================================
//==========================================================================================
void Main_BuildRGBGammaTables(float Gamma)
{
	int32				i, Val;
	int32				GammaTable[256];
	D3D_PixelMask	PixelMask;
	DWORD			R_Left, G_Left, B_Left, A_Left;
	DWORD			R_Right, G_Right, B_Right, A_Right;
	

	AppInfo.Gamma = Gamma;

	if (Gamma == 1.0)
	{
		for (i=0 ; i<256 ; i++)
			GammaTable[i] = i;
	}
	else for (i=0 ; i<256 ; i++)
	{
		float Ratio = (i+0.5f)/255.5f;

		float RGB = (float)(255.0 * pow((double)Ratio, 1.0/(double)Gamma) + 0.5);
		
		if (RGB < 0.0f)
			RGB = 0.0f;
		if (RGB > 255.0f)
			RGB = 255.0f;

		GammaTable[i] = (int32)RGB;
	}

	GetSurfacePixelMask(&AppInfo.ddTexFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		AppInfo.Lut1.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		AppInfo.Lut1.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		AppInfo.Lut1.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		AppInfo.Lut1.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
	GetSurfacePixelMask(&AppInfo.ddFourBitAlphaSurfFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		AppInfo.Lut2.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		AppInfo.Lut2.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		AppInfo.Lut2.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		AppInfo.Lut2.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
	GetSurfacePixelMask(&AppInfo.ddOneBitAlphaSurfFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		AppInfo.Lut3.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		AppInfo.Lut3.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		AppInfo.Lut3.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		AppInfo.Lut3.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}

/* 01/10/2002 Wendell Buckner
     Add gamma table for true 32-bit alpha/color.... */
	GetSurfacePixelMask(&AppInfo.ddEightBitAlphaSurfFormat, &PixelMask);
	for (i=0; i< 256; i++)
	{
		// Get shift constants for current video mode/pixel format
		R_Left = PixelMask.R_Shift;
		G_Left = PixelMask.G_Shift;
		B_Left = PixelMask.B_Shift;
		A_Left = PixelMask.A_Shift;

		R_Right = 8 - PixelMask.R_Width;
		G_Right = 8 - PixelMask.G_Width;
		B_Right = 8 - PixelMask.B_Width;
		A_Right = 8 - PixelMask.A_Width;

		Val = GammaTable[i];

		AppInfo.Lut4.R[i] = (((uint32)Val >> R_Right) << R_Left) & PixelMask.R_Mask;
		AppInfo.Lut4.G[i] = (((uint32)Val >> G_Right) << G_Left) & PixelMask.G_Mask;
		AppInfo.Lut4.B[i] = (((uint32)Val >> B_Right) << B_Left) & PixelMask.B_Mask;
		AppInfo.Lut4.A[i] = (((uint32)  i >> A_Right) << A_Left) & PixelMask.A_Mask;
	}
}

//====================================================================================================
//	D3DMain_UpdateWindow
//====================================================================================================
geBoolean DRIVERCC D3DMain_UpdateWindow(void)
{
	D3DMain_GetClientWindowOffset(AppInfo.hWnd);
	return GE_TRUE;
}

//====================================================================================================
//	D3DMain_SetActive
//====================================================================================================
geBoolean DRIVERCC D3DMain_SetActive(geBoolean wParam)
{
	if (AppInfo.lpFrontBuffer)
		AppInfo.RenderingIsOK = wParam;

	if(AppInfo.RenderingIsOK)							// Regaining focus
	{
		HRESULT	Result;

		if (AppInfo.lpFrontBuffer)
		{
			Result = AppInfo.lpFrontBuffer->IsLost();

			if(Result == DDERR_SURFACELOST)
			{
				if(!D3DMain_RestoreAllSurfaces())
				{
					OutputDebugString("Couldn't restore surfaces!\n");
					return GE_FALSE;
				}
				
				OutputDebugString("D3DMain_SetActive: Regained Focus...\n");
		
				ShowWindow(AppInfo.hWnd, SW_SHOWNORMAL);		// Dx doesn't restore it
			}
			else
				OutputDebugString("D3DMain_SetActive: No surfaces lost...\n");
		}
	}

	return	GE_TRUE;
}

//========================================================================================================
//	D3DMain_SetFogEnable
//========================================================================================================
geBoolean DRIVERCC D3DMain_SetFogEnable(geBoolean Enable, float r, float g, float b, float Start, float End)
{
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	D3DMATERIAL			Material; */
    D3DMATERIAL7		Material;

	AppInfo.FogEnable = Enable;
	AppInfo.FogR = r;
	AppInfo.FogG = g;
	AppInfo.FogB = b;
	AppInfo.FogStart = Start;
	AppInfo.FogEnd = End;

	// Fill in the material with the data
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    	
	memset(&Material, 0, sizeof(D3DMATERIAL));
	Material.dwSize       = sizeof(D3DMATERIAL); */
	memset(&Material, 0, sizeof(D3DMATERIAL7));

	if (Enable)
	{
		Material.dcvDiffuse.r = Material.dcvAmbient.r = r/255.0f;
		Material.dcvDiffuse.g = Material.dcvAmbient.g = g/255.0f;
		Material.dcvDiffuse.b = Material.dcvAmbient.b = b/255.0f;
	}
	else
	{
		Material.dcvDiffuse.r = Material.dcvAmbient.r = AppInfo.ClearR/255.0f;
		Material.dcvDiffuse.g = Material.dcvAmbient.g = AppInfo.ClearG/255.0f;
		Material.dcvDiffuse.b = Material.dcvAmbient.b = AppInfo.ClearB/255.0f;
	}

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    		
	Material.dwRampSize = 16L; // A default ramp size */

	AppInfo.BackgroundMaterial->SetMaterial(&Material); 

	return GE_TRUE;
}


//========================================================================================================
//	D3DMain_SetClearColor
//========================================================================================================
geBoolean DRIVERCC D3DMain_SetClearColor(float r, float g, float b)
{
	D3DMATERIAL7		Material;

	AppInfo.ClearR = r;
	AppInfo.ClearG = g;
	AppInfo.ClearB = b;

	// Fill in the material with the data
	memset(&Material, 0, sizeof(D3DMATERIAL7));

	Material.dcvDiffuse.r = Material.dcvAmbient.r = r/255.0f;
	Material.dcvDiffuse.g = Material.dcvAmbient.g = g/255.0f;
	Material.dcvDiffuse.b = Material.dcvAmbient.b = b/255.0f;
	
	AppInfo.BackgroundMaterial->SetMaterial(&Material); 

	return GE_TRUE;
}

//================================================================================
//	D3DMain_GetClientWindowOffset
//================================================================================
BOOL D3DMain_GetClientWindowOffset(HWND hWnd)
{
	POINT			CPoint;

	CPoint.x = CPoint.y = 0;

	ClientToScreen(hWnd, &CPoint);

	AppInfo.WindowXOffset = CPoint.x;
	AppInfo.WindowYOffset = CPoint.y;

	return TRUE;
}

//================================================================================
//	D3DMain_RememberOldMode
//================================================================================
static BOOL D3DMain_RememberOldMode(HWND hWnd)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			LastError;
	RECT			CRect;
  
	D3DMain_Log("--- D3DMain_RememberOldMode ---\n");

	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	
	LastError = AppInfo.lpDD->GetDisplayMode(&ddsd);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("Getting the current display mode failed.\n  %s\n", D3DErrorToString(LastError));
		return FALSE;
	}
	
	GetClientRect(hWnd, &CRect);

	// Get old fulscreen width/height/bpp
	AppInfo.OldWidth = ddsd.dwWidth;
	AppInfo.OldHeight = ddsd.dwHeight;
	AppInfo.OldBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

	// Get old window width/pos
	AppInfo.OldWindowWidth = CRect.right;
	AppInfo.OldWindowHeight = CRect.bottom;

	GetWindowRect(hWnd, &CRect);
	AppInfo.OldWindowRect = CRect;
	
	AppInfo.OldGWL_STYLE = GetWindowLong(hWnd, GWL_STYLE);

	D3DMain_GetClientWindowOffset(hWnd);

	return TRUE;
}

//==========================================================================================
//	D3DMain_SetDisplayMode
//==========================================================================================
static BOOL D3DMain_SetDisplayMode(HWND hWnd, int w, int h, int bpp, BOOL FullScreen)
{
	HRESULT	LastError;
	int		DWidth, DHeight;
	char	YN[2][32];

	strcpy(YN[0], "NO");
	strcpy(YN[1], "YES");
  
	D3DMain_Log("--- D3DMain_SetDisplayMode ---\n");
	D3DMain_Log("  W: %i, H: %i, Bpp: %i, FullScreen: %s\n", w, h, bpp, YN[FullScreen]);

	if (FullScreen)
	{
		LastError = AppInfo.lpDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);// | DDSCL_ALLOWREBOOT);
		
		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to fullscreen failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}

		LastError = AppInfo.lpDD->SetDisplayMode(w, h, bpp,0,0);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetFullScreenDisplayMode:  Mode %dx%dx%d failed\n  %s\n", w, h, bpp, D3DErrorToString(LastError));
			return FALSE;
		}

		DWidth = GetSystemMetrics(SM_CXSCREEN);
		DHeight = GetSystemMetrics(SM_CYSCREEN);

		//
		// Set window boundaries to cover entire desktop, and show it
		//
		SetWindowLong(hWnd, GWL_STYLE, AppInfo.OldGWL_STYLE | WS_POPUP);
	
		SetWindowLong(hWnd, GWL_STYLE, AppInfo.OldGWL_STYLE & 
											  ~(WS_OVERLAPPED  | 
												WS_CAPTION     | 
                                                WS_SYSMENU     | 
                                                WS_MINIMIZEBOX | 
                                                WS_MAXIMIZEBOX | 
                                                WS_THICKFRAME));


		SetWindowPos(AppInfo.hWnd, 
		            HWND_TOP, 
		            0,
		            0,
		            DWidth,
		            DHeight,
		            SWP_NOCOPYBITS | SWP_NOZORDER);

		ShowWindow(AppInfo.hWnd, SW_SHOWNORMAL);
	}
	else
	{
		LastError = AppInfo.lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to normal failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	}

	AppInfo.hWnd = hWnd;
	AppInfo.CurrentWidth = w;
	AppInfo.CurrentHeight = h;
	AppInfo.CurrentBpp = bpp;
	AppInfo.FullScreen = FullScreen;

	AppInfo.ModeSet = GE_TRUE;

	return TRUE;
}

//================================================================================
//	D3DMain_PickDevice
//================================================================================

/* 02/21/2004 Wendell Buckner
    DOT3 BUMPMAPPING */
extern DRV_EngineSettings EngineSettings;

static BOOL D3DMain_PickDevice(void)
{
	int32	i;
	DWORD	Depths;

	D3DMain_Log("--- D3DMain_PickDevice ---\n");

	// Find a device with the same bpp as the mode set
	// start 32 bit change
	if (gWidth == -1 && gHeight == -1)
	{
		// Find a device with the same bpp as the mode set
		Depths = BPPToDDBD(AppInfo.CurrentBpp);
	}
	else
	{
		// Find a device with the same bpp as the mode set
		Depths = BPPToDDBD(BPP32);
	}

	//Depths = BPPToDDBD(AppInfo.CurrentBpp);
// end 32 bit change


	for (i = 0; i < AppInfo.NumDrivers; i++)
	{
		if (!(AppInfo.Drivers[i].IsHardware))		// ONLY hardware
			continue;

		// Only choose drivers that can support our draw buffer bpp
		if (!(AppInfo.Drivers[i].Desc.dwDeviceRenderBitDepth & Depths))
			continue;

		// Only choose drivers that can create the zbuffer we need
// start 32 bit changes

		// Get the Z buffer depth
/* 01/02/2003 Wendell Buckner
    Allow z buffer depths of 32 bits 
		if ( ZbufferD == 24)*/
		if ( ZbufferD == 32) 
		{
		// Only choose drivers that can create the zbuffer we need
			if (!(AppInfo.Drivers[i].Desc.dwDeviceZBufferBitDepth & DDBD_32))
				continue;
		}
		else if ( ZbufferD == 24)
		{
		// Only choose drivers that can create the zbuffer we need
			if (!(AppInfo.Drivers[i].Desc.dwDeviceZBufferBitDepth & DDBD_24))
				continue;
		}
		else
		{
		// Only choose drivers that can create the zbuffer we need
			if (!(AppInfo.Drivers[i].Desc.dwDeviceZBufferBitDepth & DDBD_16))
				continue;
		}
		//if (!(AppInfo.Drivers[i].Desc.dwDeviceZBufferBitDepth & DDBD_16))
			//continue;
// end 32 bit changes

/* 07/16/2000 Wendell Buckner
   POTENTIAL GOTCHA * DOCS SAY DCMCOLORMODEL IS SUPPORTED BUT IT IS NO LONGER AVAILABLE  
		if (!(AppInfo.Drivers[i].Desc.dcmColorModel & D3DCOLOR_RGB)) 
			continue;                                                */

		if (!AppInfo.Drivers[i].CanDoWindow && !AppInfo.FullScreen)
			continue;

		// Remember the current driver
		AppInfo.CurrentDriver = i;

/*   01/05/2004 Wendell Buckner                                                         
      CONFIG DRIVER - Display the drivers capibilities in the the driver log                            */
		// Remember the current driver
	    if ( AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP )
         D3DMain_Log("    D3DTEXOPCAPS_BUMPENVMAP      : YES\n");
	    else
         D3DMain_Log("    D3DTEXOPCAPS_BUMPENVMAP      : NO\n");

	    if ( AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE  )
         D3DMain_Log(" D3DTEXOPCAPS_BUMPENVMAPLUMINANCE: YES\n");
	    else
         D3DMain_Log(" D3DTEXOPCAPS_BUMPENVMAPLUMINANCE: NO\n");

/*	01/04/2004 Wendell Buckner
    DOT3 BUMPMAPPING */
	    if ( AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3  )
         D3DMain_Log("   D3DTEXOPCAPS_DOTPRODUCT3     : YES\n");
	    else
         D3DMain_Log("   D3DTEXOPCAPS_DOTPRODUCT3     : NO\n"); 

/* 02/21/2004 Wendell Buckner                                                         
    DOT3 BUMPMAPPING                                                                  */
		{
		 EngineSettings.CanSupportFlags = (DRV_SUPPORT_ALPHA | DRV_SUPPORT_COLORKEY);

         if ( AppInfo.Drivers[AppInfo.CurrentDriver].CanDoBumpMapping ) EngineSettings.CanSupportFlags |= DRV_SUPPORT_EMBM;
         if ( AppInfo.Drivers[AppInfo.CurrentDriver].CanDoDot3 ) EngineSettings.CanSupportFlags |= DRV_SUPPORT_DOT3;
        }

		return TRUE;
	}

	D3DMain_Log("    Could not find a %i-bit Z buffer \n",ZbufferD);
	return FALSE;
}

//================================================================================
//	D3DMain_CreateDevice
//================================================================================
static BOOL D3DMain_CreateDevice(void)
{
	HRESULT					Error;

	D3DMain_Log("--- D3DMain_CreateDevice ---\n");

	// Release old device
	RELEASE(AppInfo.lpD3DDevice);

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	Error = AppInfo.lpD3D->CreateDevice(AppInfo.Drivers[AppInfo.CurrentDriver].Guid, 
										AppInfo.lpBackBuffer, 
										&AppInfo.lpD3DDevice,NULL);                  */
	Error = AppInfo.lpD3D->CreateDevice(AppInfo.Drivers[AppInfo.CurrentDriver].Guid, 
										  AppInfo.lpBackBuffer, 
									      &AppInfo.lpD3DDevice);  


	if (Error != DD_OK) 
	{
		D3DMain_Log("D3DMain_CreateDevice:  lpD3D->CreateDevice failed:\n  %s\n", D3DErrorToString(Error));
		return FALSE;
	}

	#if 0
	{
		// Get some info from the device
		D3DDEVICEDESC hw, sw;

		hw.dwSize = sw.dwSize = D3DDEVICEDESCSIZE;
    
		AppInfo.lpD3DDevice->GetCaps(&hw, &sw);
	}
	#endif
	
	// Get Device Identifier
	Error = AppInfo.lpDD->GetDeviceIdentifier(&AppInfo.DeviceIdentifier, 0);

	if (Error != DD_OK) 
	{
		D3DMain_Log("D3DMain_CreateDevice:  lpDD->GetDeviceIdentifier failed:\n  %s\n", D3DErrorToString(Error));
		return FALSE;
	}

	// Print the debug ID
	D3DMain_Log("   Vender ID = %6i\n", AppInfo.DeviceIdentifier.dwVendorId);
	D3DMain_Log("   Device ID = %6i\n", AppInfo.DeviceIdentifier.dwDeviceId);

	return TRUE;
}

//================================================================================
//	D3DMain_CreateBuffers
//================================================================================
static BOOL D3DMain_CreateBuffers(void)
{
	DDSURFACEDESC2	ddsd;
	DDSCAPS2		ddscaps;
	HRESULT			LastError;
  
	D3DMain_Log("--- D3DMain_CreateBuffers ---\n");

	// Release any old objects that might be lying around.  This should have
	// already been taken care of, but just in case...
	RELEASE(AppInfo.lpClipper);
	RELEASE(AppInfo.lpBackBuffer);
	RELEASE(AppInfo.lpFrontBuffer);
  
	if (AppInfo.FullScreen) 
	{
		// Create a complex flipping surface for fullscreen mode with one
		// back buffer.
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
		DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX;

/* 12/27/2003 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings *
/ * 12/06/2001 Wendell Buckner
    ENHANCEMENT - Allow triple buffering 
		ddsd.dwBackBufferCount = 1;      *
		ddsd.dwBackBufferCount = 2; */
		ddsd.dwBackBufferCount = BBufferCount;

		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

/* 12/27/2003 Wendell Buckner
   CONFIG DRIVER -  Make the driver configurable by "ini" file settings *
/* 12/06/2001 Wendell Buckner
    ENHANCEMENT - Allow full-scene anti-aliasing  *
        if ( AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dpcLineCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT )*/
        if ( FSAntiAliasing && (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dpcLineCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) )
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING; 

		LastError = CreateSurface(&ddsd, &AppInfo.lpFrontBuffer);

		if(LastError != DD_OK) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  Please restart the program and try another fullscreen mode with less resolution or lower bit depth.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for fullscreen flipping surface failed.\n  %s\n",
						D3DErrorToString(LastError));
			}
			
			goto exit_with_error;
		}

		// Obtain a pointer to the back buffer surface created above so we
		// can use it later.  For now, just check to see if it ended up in
		// video memory (FYI).
        memset(&ddscaps,0,sizeof(DDSCAPS2));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		LastError = AppInfo.lpFrontBuffer->GetAttachedSurface(&ddscaps, &AppInfo.lpBackBuffer);
		
		if(LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  GetAttachedSurface failed to get back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
		
		LastError = GetSurfDesc(&ddsd, AppInfo.lpBackBuffer);
		
		if (LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  Failed to get surface description of back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
		
		AppInfo.BackBufferInVideo =
			(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;

		AppInfo.ddsd = ddsd;	// Save the format of the back buffer
	}
	else 
	{	
		// In the window case, create a front buffer which is the primary
		// surface and a back buffer which is an offscreen plane surface.

		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
		
		LastError = AppInfo.lpDD->CreateSurface(&ddsd, &AppInfo.lpFrontBuffer, NULL);

		if(LastError != DD_OK ) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  To run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for window front buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
			}
			goto exit_with_error;
		}
    
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = AppInfo.CurrentWidth;
		ddsd.dwHeight = AppInfo.CurrentHeight;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    
		LastError = CreateSurface(&ddsd, &AppInfo.lpBackBuffer);
	
		if (LastError != DD_OK) 
		{
			if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
			{
				D3DMain_Log("CreateBuffers:  There was not enough video memory to create the rendering surface.\n  To run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n");
			} 
			else 
			{
				D3DMain_Log("CreateBuffers:  CreateSurface for window back buffer failed.\n  %s\n",
					D3DErrorToString(LastError));
			}
			goto exit_with_error;
		}
    
		// Check to see if the back buffer is in video memory (FYI).
		LastError = GetSurfDesc(&ddsd, AppInfo.lpBackBuffer);

		if (LastError != DD_OK) 
		{
			D3DMain_Log("CreateBuffers:  Failed to get surface description for back buffer.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
    
		AppInfo.BackBufferInVideo =	(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;
    
		// Create the DirectDraw Clipper object and attach it to the window
		// and front buffer.
		LastError = AppInfo.lpDD->CreateClipper(0, &AppInfo.lpClipper, NULL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  CreateClipper failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
	
		LastError = AppInfo.lpClipper->SetHWnd(0, AppInfo.hWnd);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  Attaching clipper to window failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}
	
		
		LastError = AppInfo.lpFrontBuffer->SetClipper(AppInfo.lpClipper);
    
		if(LastError != DD_OK ) 
		{
			D3DMain_Log("CreateBuffers:  Attaching clipper to front buffer failed.\n  %s\n",
				D3DErrorToString(LastError));
			goto exit_with_error;
		}

		AppInfo.ddsd = ddsd;		// Save the format of the back buffer
	}	
  
	D3DMain_ClearBuffers();
  
/* 12/27/2003 Wendell Buckner
    CONFIG DRIVER - Make the driver configurable by "ini" file settings */
    D3DMain_Log("    Back Buffer Count = %i \n",BBufferCount);

	return TRUE;
  
	exit_with_error:
		
		RELEASE(AppInfo.lpFrontBuffer);
		RELEASE(AppInfo.lpBackBuffer);
		RELEASE(AppInfo.lpClipper);
		return FALSE;
}

//================================================================================
//	D3DMain_DestroyBuffers
//================================================================================
static void D3DMain_DestroyBuffers(void)
{
	RELEASE(AppInfo.lpClipper);
	RELEASE(AppInfo.lpBackBuffer);
	RELEASE(AppInfo.lpFrontBuffer);
}

//================================================================================
//	D3DMain_CreateZBuffer
//	Create a Z-Buffer of the appropriate depth and attach it to the back buffer.
//================================================================================
static BOOL D3DMain_CreateZBuffer(void)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			LastError;
	
	assert(AppInfo.lpBackBuffer);

	D3DMain_Log("--- D3DMain_CreateZBuffer ---\n");

	// Release any Z-Buffer that might be around just in case.
	RELEASE(AppInfo.lpZBuffer);
  
	memset(&ddsd, 0 ,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;//DDSD_ZBUFFERBITDEPTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = AppInfo.CurrentWidth;
	ddsd.dwHeight = AppInfo.CurrentHeight;
	
// changed QD Shadows	
//	ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
	ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER|DDPF_STENCILBUFFER;
// end change

    // Find a valid zbuffer, from the current device
    AppInfo.lpD3D->EnumZBufferFormats(AppInfo.Drivers[AppInfo.CurrentDriver].Guid, EnumZBufferFormatsCallback,
										(VOID*)&ddsd.ddpfPixelFormat);

// changed QD
	if(AppInfo.Drivers[AppInfo.CurrentDriver].CanDoStencil) EngineSettings.CanSupportFlags |= DRV_SUPPORT_STENCIL;
//end change


    
	if( sizeof(DDPIXELFORMAT) != ddsd.ddpfPixelFormat.dwSize )
    {
		D3DMain_Log("CreateZBuffer:  No zbuffer found for 3d device.\n");
		return FALSE;
    }

	LastError = AppInfo.lpDD->CreateSurface(&ddsd, &AppInfo.lpZBuffer, NULL);
	
	if(LastError != DD_OK) 
	{
		if (LastError == DDERR_OUTOFMEMORY || LastError == DDERR_OUTOFVIDEOMEMORY) 
		{
			if (AppInfo.FullScreen) 
			{
				D3DMain_Log("CreateZBuffer:  There was not enough video memory to create the Z-buffer surface.\n  Please try another mode with less resolution.\n");
			} 
			else 
			{
				D3DMain_Log("CreateZBuffer:  There was not enough video memory to create the Z-buffer surface.\n  Please try another mode with less resolution.\n");
			}
		} 
		else 
		{
			D3DMain_Log("CreateZBuffer:  CreateSurface for Z-buffer failed.\n  %s\n",
				D3DErrorToString(LastError));
		}
	
		goto exit_with_error;
	}
	
	// Attach the Z-buffer to the back buffer so D3D will find it
	LastError = AppInfo.lpBackBuffer->AddAttachedSurface(AppInfo.lpZBuffer);
	
	if(LastError != DD_OK) 
	{
		D3DMain_Log("CreateZBuffer:  AddAttachedBuffer failed for Z-Buffer.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}
	
	// Find out if it ended up in video memory.
	LastError = GetSurfDesc(&ddsd, AppInfo.lpZBuffer);

	if (LastError != DD_OK) 
	{
		D3DMain_Log("CreateZBuffer:  Failed to get surface description of Z buffer.\n  %s\n",
			D3DErrorToString(LastError));
		goto exit_with_error;
	}
	
	AppInfo.ZBufferInVideo = (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE;
  
/* 12/20/2003 Wendell Buckner 
    CONFIG DRIVER - Display the zbuffer depth*/
    {
	 char	YN[2][32];

	 strcpy(YN[0], "NO");
	 strcpy(YN[1], "YES");

	 D3DMain_Log("   ZBuffer Depth Available:");
     D3DMain_Log(" 16-bit (%s),", YN[ (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_16) ? 1 : 0] );
	 D3DMain_Log(" 24-bit (%s),", YN[ (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_24) ? 1 : 0] );
     D3DMain_Log(" 32-bit (%s),", YN[ (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_32) ? 1 : 0] );
     D3DMain_Log("\n");

//***

	 if ( (ZbufferD == 16) && (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_16))
      D3DMain_Log("   ZBuffer Depth: 16, ZBuffer in Video: %s\n",YN[AppInfo.ZBufferInVideo]);

	 if ( (ZbufferD == 24) && (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_24))
      D3DMain_Log("   ZBuffer Depth: 24, ZBuffer in Video: %s\n",YN[AppInfo.ZBufferInVideo]);

	 if ( (ZbufferD == 32) && (AppInfo.Drivers[AppInfo.CurrentDriver].Desc.dwDeviceZBufferBitDepth & DDBD_32))
      D3DMain_Log("   ZBuffer Depth: 32, ZBuffer in Video: %s\n",YN[AppInfo.ZBufferInVideo]);
    }

	return TRUE;
  
	exit_with_error:
		RELEASE(AppInfo.lpZBuffer);
		return FALSE;
}

//================================================================================
//	D3DMain_DestroyZBuffer
//================================================================================
static void D3DMain_DestroyZBuffer(void)
{
	RELEASE(AppInfo.lpZBuffer)
}

//================================================================================
//	D3DMain_RestoreDisplayMode
//	 Does nothing if mode has not been set yet
//================================================================================
static BOOL D3DMain_RestoreDisplayMode(void)
{
	HRESULT		LastError;

	if (!AppInfo.ModeSet)
		return TRUE;

	AppInfo.ModeSet = GE_FALSE;

	assert(AppInfo.lpDD);

	if (AppInfo.FullScreen) 
	{
		LastError = AppInfo.lpDD->RestoreDisplayMode();
	
		if (LastError != DD_OK) 
		{
			D3DMain_Log("D3DMain_RestoreDisplayMode:  RestoreDisplayMode failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	
		LastError = AppInfo.lpDD->SetCooperativeLevel(AppInfo.hWnd, DDSCL_NORMAL);

		if(LastError != DD_OK ) 
		{
			D3DMain_Log("SetCooperativeLevel to normal failed.\n  %s\n",
				D3DErrorToString(LastError));
			return FALSE;
		}
	}
	
	// Restore window width/height
	SetWindowLong(AppInfo.hWnd, GWL_STYLE, AppInfo.OldGWL_STYLE);

	SetWindowPos(AppInfo.hWnd, 
	            HWND_TOP, 
				AppInfo.OldWindowRect.left,
				AppInfo.OldWindowRect.top,
				(AppInfo.OldWindowRect.right - AppInfo.OldWindowRect.left),
				(AppInfo.OldWindowRect.bottom - AppInfo.OldWindowRect.top),
	            SWP_NOCOPYBITS | SWP_NOZORDER);

	ShowWindow(AppInfo.hWnd, SW_SHOWNORMAL);

	return TRUE;
}


//
//	Enum drivers
//

D3D_DRIVER	Drivers[MAX_DRIVERS];

//========================================================================================================
//	EnumDriversCB2
//========================================================================================================

/*   07/16/2000 Wendell Buckner                                                         
      Convert to Directx7...                                                            
BOOL FAR PASCAL EnumDriversCB2(GUID FAR *lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext)              */
BOOL FAR PASCAL EnumDriversCB2(GUID FAR *lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext, HMONITOR  hm)
{
	DDEnumInfo		*Info;
	D3D_DRIVER		*pDriver;

	if (!lpDriverDesc || !lpDriverName)
		return (D3DENUMRET_OK);

	if (strlen(lpDriverDesc) + 5 >= MAX_DRIVER_NAME)
		return DDENUMRET_OK;

	Info = (DDEnumInfo*)lpContext;

	if (Info->NumDrivers >= MAX_DRIVERS)
		return DDENUMRET_CANCEL;

	pDriver = &Info->Drivers[Info->NumDrivers++];

	if (lpGUID)
	{
		pDriver->IsPrimary = GE_FALSE;
		memcpy(&pDriver->Guid, lpGUID, sizeof(GUID));
	}
	else
	{
		pDriver->IsPrimary = GE_TRUE;
	}

	// Save name
	sprintf(pDriver->Name, "(D3D)%s", lpDriverDesc);

	return DDENUMRET_OK;
}

//========================================================================================================
//	EnumSubDrivers2
//========================================================================================================
BOOL DRIVERCC EnumSubDrivers2(DRV_ENUM_DRV_CB *Cb, void *Context)
{
	HRESULT		hr;
	int32		i;
	DDEnumInfo	Info;

	unlink(D3DMAIN_LOG_FILENAME);
	
	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

/*   07/16/2000 Wendell Buckner                                                         
      Convert to Directx7...                                                            
	hr = DirectDrawEnumerate(EnumDriversCB2, &Info);                                   */
    hr = DirectDrawEnumerateEx((LPDDENUMCALLBACKEX) EnumDriversCB2, &Info, 0);
	

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_EnumSubDrivers: DirectDrawEnumerate failed.\n");
		return FALSE;
    }

	for (i=0; i< Info.NumDrivers; i++)
	{
 		// Create the DD object for this driver
		if (!CreateDDFromDriver(&Info.Drivers[i]))
			return GE_FALSE;

		if (Main_CheckDD())
		{
			if (!Cb(i, Info.Drivers[i].Name, Context))
			{
				RELEASE(AppInfo.lpD3D);
				AppInfo.lpD3D = NULL;

				//D3DMain_ShutdownD3D();
				RELEASE(AppInfo.lpDD);
				AppInfo.lpDD = NULL;
				memset(&AppInfo, 0, sizeof(AppInfo));
				break;
			}
		}
		
		RELEASE(AppInfo.lpD3D);
		AppInfo.lpD3D = NULL;

		//D3DMain_ShutdownD3D();
		RELEASE(AppInfo.lpDD);
		AppInfo.lpDD = NULL;
		memset(&AppInfo, 0, sizeof(AppInfo));
	}
	
	//Cb(i, "(D3D)HackDriver", Context);

	return TRUE;
}	

//========================================================================================================
//	EnumModes2
//========================================================================================================
BOOL DRIVERCC EnumModes2(int32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context)
{
	HRESULT			hr;
	int32			i, Width, Height;
	char			ModeName[MAX_DRIVER_NAME];
	DDEnumInfo		Info;
	
	//Cb(0, "HackMode 2", 640, 480, Context);
	//return GE_TRUE;

	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

/*   07/16/2000 Wendell Buckner                                                         
      Convert to Directx7...                                                            
	hr = DirectDrawEnumerate(EnumDriversCB2, &Info);                                   */
    hr = DirectDrawEnumerateEx((LPDDENUMCALLBACKEX) EnumDriversCB2, &Info, NULL);
	

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_EnumModes:  DirectDrawEnumerate failed.\n");
		return FALSE;
    }


	if (!CreateDDFromName(DriverName, &Info))
		return GE_FALSE;

	if (!D3DMain_EnumDisplayModes())
	{
		D3DMain_ShutdownD3D();
		
		D3DMain_Log("D3DMain_EnumModes: D3DMain_EnumDisplayModes failed.\n");
		return FALSE;
	}
		
	for (i=0; i< AppInfo.NumModes; i++)
	{
		if (AppInfo.Modes[i].Bpp != 16)
			continue;

		// Get the width/height of mode
		Width = AppInfo.Modes[i].Width;
		Height = AppInfo.Modes[i].Height;

		// Make a unique name
		sprintf(ModeName, "%ix%i", Width, Height);

		// Call their callback with this driver mode
		if (!Cb(i, ModeName, Width, Height, Context))
		{
			D3DMain_ShutdownD3D();
			return GE_TRUE;
		}
	}
	
	if (AppInfo.CanDoWindow)
	{	
		if (!Cb(i, "WindowMode", -1, -1, Context))
		{
			D3DMain_ShutdownD3D();
			return GE_TRUE;
		}
	}

	D3DMain_ShutdownD3D();

	return TRUE;
}

//================================================================================
//	DDEnumCallback
//================================================================================
static BOOL FAR PASCAL DDEnumCallback(	GUID FAR* lpGUID, 
										LPSTR lpDriverDesc, 
										LPSTR lpDriverName, 
										LPVOID lpContext)
{
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	LPDIRECTDRAW4	pDD6;     */
	LPDIRECTDRAW7	pDD6;
	
	DDCAPS			DriverCaps, HELCaps;
	DD_Enum			*DDEnum;

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	LPDIRECTDRAW	pDD1;     */
	LPDIRECTDRAW7	pDD1;

	HRESULT			hr;

	DDEnum = (DD_Enum*)lpContext;
	  
	if(strncmp(lpDriverDesc, DDEnum->DriverName, strlen(DDEnum->DriverName))) 
		return DDENUMRET_OK;		// Next... This is not the one they wanted

	pDD1 = NULL;

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	hr = DirectDrawCreate( lpGUID, &pDD1, NULL ); */
    hr = DirectDrawCreateEx( lpGUID, (void **) &pDD1, IID_IDirectDraw7, NULL );
		
	if(FAILED( hr ))
		return DDENUMRET_CANCEL;		// Assume this is bad, and stop

	assert(pDD1);

	// Get a ptr to an IDirectDraw4 interface. This interface to DirectDraw
	// represents the DX6 version of the API.

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    
	hr = pDD1->QueryInterface( IID_IDirectDraw4, (VOID**)&pDD6); */
    hr = pDD1->QueryInterface( IID_IDirectDraw7, (VOID**)&pDD6); 

	// Don't need this anymore
	RELEASE(pDD1);

	if( FAILED( hr ) )
		return DDENUMRET_CANCEL;

	memset(&DriverCaps, 0, sizeof(DDCAPS));
	DriverCaps.dwSize = sizeof(DDCAPS);
	memset(&HELCaps, 0, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);
		
	if (FAILED(pDD6->GetCaps(&DriverCaps, &HELCaps))) 
	{
		RELEASE(pDD6);
		return DDENUMRET_CANCEL;
	}

	// Make sure it's a 3d compatible device
	if (!(DriverCaps.dwCaps & DDCAPS_3D)) 
	{
		RELEASE(pDD6);
		return DDENUMRET_CANCEL;
	}
		
	if (!lpGUID)
		AppInfo.IsPrimary = TRUE;
	else
		AppInfo.IsPrimary = FALSE;

	DDEnum->lpDD = pDD6;
	DDEnum->FoundDD = TRUE;
			
	return DDENUMRET_CANCEL;	// We are done
}

//================================================================================
//	D3DMain_CreateDDFromName
//	Creates DD, searching for the specified DD name using DriverName
//================================================================================
static BOOL D3DMain_CreateDDFromName(const char *DriverName)
{
	HRESULT			hr;
	DDCAPS			DriverCaps, HELCaps;
	DDEnumInfo		Info;
	
	D3DMain_Log("--- D3DMain_CreateDDFromName ---\n");
	
	if (strlen(DriverName) >= MAX_DRIVER_NAME)
		return GE_FALSE;

	D3DMain_Log("  Name: %s\n", DriverName);

	Info.Drivers = Drivers;
	Info.NumDrivers = 0;

/*   07/16/2000 Wendell Buckner                                                         
      Convert to Directx7...                                                            
	hr = DirectDrawEnumerate(EnumDriversCB2, &Info);;                                   */
    hr = DirectDrawEnumerateEx((LPDDENUMCALLBACKEX) EnumDriversCB2, &Info, NULL);

    if (hr != DD_OK) 
	{
		D3DMain_Log("D3DMain_CreateDDFromName:  DirectDrawEnumerate failed.\n");
		return FALSE;
    }
		
	{
		char		TempName[1024];

		sprintf(TempName, "(D3D)%s", DriverName);

		if (!CreateDDFromName(TempName, &Info))
			return GE_FALSE;
	}

	assert(AppInfo.lpDD);

	memset(&DriverCaps, 0, sizeof(DDCAPS));
	DriverCaps.dwSize = sizeof(DDCAPS);
	memset(&HELCaps, 0, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);

	if (FAILED(AppInfo.lpDD->GetCaps(&DriverCaps, &HELCaps))) 
	{
		D3DMain_Log("D3DMain_CreateDDFromName:  GetCaps failed.\n");
		D3DMain_ShutdownD3D();
		return FALSE;
	}

	if (DriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
		D3DMain_Log("   DDCAPS2_CANRENDERWINDOWED    : YES\n");
	else
		D3DMain_Log("   DDCAPS2_CANRENDERWINDOWED    : NO\n");

	if (DriverCaps.dwCaps2 & DDCAPS2_NO2DDURING3DSCENE)
		D3DMain_Log("   DDCAPS2_NO2DDURING3DSCENE    : YES\n");
	else
		D3DMain_Log("   DDCAPS2_NO2DDURING3DSCENE    : NO\n");

/* 01/05/2004 Wendell Buckner
    CONFIG DRIVER - Display the drivers capibilities in the the driver log */
	if ( DriverCaps.dwCaps2 & DDCAPS2_FLIPNOVSYNC )
        D3DMain_Log("   DDCAPS2_FLIPNOVSYNC          : YES\n");
	else
        D3DMain_Log("   DDCAPS2_FLIPNOVSYNC          : NO\n");

	// Save the DD object
	strcpy(AppInfo.DDName, DriverName);
	
	return TRUE;
}

//========================================================================================================
//	CreateDDFromDriver
//========================================================================================================
static geBoolean CreateDDFromDriver(D3D_DRIVER *pDriver)
{
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    	
	LPDIRECTDRAW	pDD1;     */
	LPDIRECTDRAW7	pDD1;
	HRESULT			hr;

	AppInfo.IsPrimary = pDriver->IsPrimary;


	if (pDriver->IsPrimary)

/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    	
	LPDIRECTDRAW	pDD1;     
		hr = DirectDrawCreate(NULL, &pDD1, NULL );
	else
		hr = DirectDrawCreate(&pDriver->Guid, &pDD1, NULL );*/
		hr = DirectDrawCreateEx(NULL, (void **) &pDD1, IID_IDirectDraw7, NULL );
	else
		hr = DirectDrawCreateEx(&pDriver->Guid, (void **) &pDD1, IID_IDirectDraw7, NULL );

	if( FAILED( hr ) )
		return GE_FALSE;

	// Get a ptr to an IDirectDraw4 interface. This interface to DirectDraw
	// represents the DX6 version of the API.
/* 07/16/2000 Wendell Buckner
    Convert to Directx7...    	
	LPDIRECTDRAW	pDD1;     
	hr = pDD1->QueryInterface(IID_IDirectDraw4, (VOID**)&AppInfo.lpDD);*/
    hr = pDD1->QueryInterface(IID_IDirectDraw7, (VOID**)&AppInfo.lpDD);

	// Don't need this guy anymore
	RELEASE(pDD1);

	if(FAILED(hr))
		return GE_FALSE;

	return GE_TRUE;
}

//========================================================================================================
//	CreateDDFromName
//========================================================================================================
static geBoolean CreateDDFromName(const char *DriverName, const DDEnumInfo *Info)
{
	int32			i;

	for (i=0; i < Info->NumDrivers; i++)
	{
		if (!strcmp(Info->Drivers[i].Name, DriverName))
			break;
	}

	if (i == Info->NumDrivers)
		return GE_FALSE;

 	// Create the DD object for this driver
	if (!CreateDDFromDriver(&Info->Drivers[i]))
		return GE_FALSE;

	return GE_TRUE;
}

