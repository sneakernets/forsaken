
#include <d3dx9.h>

extern "C" {
#include "typedefs.h"
#include "d3dappi.h"
#include "tload.h"
#include "new3d.h"
#include <assert.h>
#include "util.h"

extern  BOOL DontColourKey;
extern	BOOL MipMap;
extern	BOOL	Is3Dfx2;
extern	int		TexturePalettized;
extern	int		TextureRedBPP;
extern	int		TextureGreenBPP;
extern	int		TextureBlueBPP;
extern	int		TextureAlphaBPP;
extern	int		TextureIndexBPP;

BOOL	TriLinear;

/***************************************************************************/
/*                            Creation of D3D                              */
/***************************************************************************/

BOOL Init3DRenderer(HWND hwnd, D3DAppInfo** D3DApp)
{
	HRESULT LastError;

	D3DAppISetDefaults();
	

	/* Set up Direct3D interface object */
	d3dappi.lpD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3dappi.lpD3D)
	{
		OutputDebugString("couldnt create d3d9\n");
		return FALSE;
	}

	D3DDISPLAYMODE d3ddm;
	LastError = d3dappi.lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

	/* create d3d device */
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory (&d3dpp, sizeof(d3dpp));
	d3dpp.hDeviceWindow = hwnd;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	if (/*windowed*/1) 
	{
		d3dpp.Windowed = TRUE;
		d3dpp.BackBufferWidth = 800;
		d3dpp.BackBufferHeight = 600;
		d3dpp.PresentationInterval = 0;
		SetWindowPos( d3dappi.hwnd, HWND_TOP, 0, 0, /*d3dpp.BackBufferWidth, d3dpp.BackBufferHeight*/800, 600, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
	}

	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; // 32 bit alpha channel
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	LastError = d3dappi.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, &d3dpp, &d3dappi.lpD3DDevice);

	if (FAILED(LastError)) 
	{
		LastError = d3dappi.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dappi.lpD3DDevice);
	}
	if (FAILED(LastError)) 
	{
		LastError = d3dappi.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_MIXED_VERTEXPROCESSING , &d3dpp, &d3dappi.lpD3DDevice);
	}
	if (FAILED(LastError)) 
	{
		LastError = d3dappi.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dappi.lpD3DDevice);
	}

	d3dappi.hwnd = hwnd;
	*D3DApp = &d3dappi;

	bD3DAppInitialized = TRUE;
	d3dappi.bRenderingIsOK = TRUE;

	d3dappi.szClient.cx = d3dpp.BackBufferWidth; 
	d3dappi.szClient.cy = d3dpp.BackBufferHeight;

	d3dapp->WindowsDisplay.w = d3dpp.BackBufferWidth;
	d3dapp->WindowsDisplay.h = d3dpp.BackBufferHeight;

	/* do "after device created" stuff */
	ZeroMemory( &d3dappi.D3DViewport, sizeof(d3dappi.D3DViewport) );
	d3dappi.D3DViewport.X = 0;
	d3dappi.D3DViewport.Y = 0;
	d3dappi.D3DViewport.Width = 800;
	d3dappi.D3DViewport.Height = 600;
	d3dappi.D3DViewport.MinZ = 0.0f;
	d3dappi.D3DViewport.MaxZ = 1.0f;

	LastError = FSSetViewPort(&d3dappi.D3DViewport);
	if (FAILED(LastError))
	{
		OutputDebugString("couldn't set viewport\n");
	}

	// load the view
	if (!InitView() )
	{
	    Msg("InitView failed.\n");
//		CleanUpAndPostQuit();
        return FALSE;
	}

	return TRUE;
}

BOOL FlipBuffers()
{
	if (!d3dappi.bRenderingIsOK) 
	{
		OutputDebugString("Cannot call D3DAppShowBackBuffer while bRenderingIsOK is FALSE.\n");
		return FALSE;
	}

	d3dappi.lpD3DDevice->Present(NULL, NULL, NULL, NULL);

	return TRUE;
}

/***************************************************************************/
/*                      Setting the render state                           */
/***************************************************************************/
/*
 * D3DAppISetRenderState
 * Create and execute an execute buffer which will set the render state and
 * light state for the current viewport.
 */
static BOOL SetUpZBuf( DWORD type )
{
	return TRUE;
#if 0 // bjd
	D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPVOID lpBuffer, lpInsStart;
    size_t size;
	LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf;

	lpD3DExCmdBuf = NULL;

    /*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
        return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE);
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
        d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
                                             &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;
    /*
     * Set render state
     */
    OP_STATE_RENDER(1, lpBuffer);

	STATE_DATA(D3DRENDERSTATE_ZFUNC, type, lpBuffer);

    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
                                                          (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);

    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
                                                     lpD3DExCmdBuf,
                                                     d3dappi.lpD3DViewport,
                                                     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }

    RELEASE( lpD3DExCmdBuf );

    return TRUE;

exit_with_error:
    RELEASE( lpD3DExCmdBuf );
    return FALSE;
#endif
}

extern BOOL ZClearsOn;
extern BOOL g_OddFrame;

BOOL SetZCompare( void )
{
	if( !ZClearsOn && g_OddFrame )
	{
		if( !SetUpZBuf( D3DCMP_GREATEREQUAL ) )
			return FALSE;
	}else
	{
		if( !SetUpZBuf( D3DCMP_LESSEQUAL ) )
			return FALSE;
	}

    return TRUE;
}

#define STATE( K, V ) \
	d3dappi.lpD3DDevice->SetRenderState( K, V );

#define TSTATE( N, K, V ) \
	d3dappi.lpD3DDevice->SetTextureStageState( N, K, V );

#define SSTATE( N, K, V ) \
	d3dappi.lpD3DDevice->SetSamplerState( N, K, V );

// c helper
void render_state( D3DRENDERSTATETYPE type, int val )
{
	d3dappi.lpD3DDevice->SetRenderState( type, val );
}

void set_trans_state_3( void )
{
	STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
	STATE(D3DRS_DESTBLEND,	D3DBLEND_SRCALPHA);
}

void set_trans_state_2( void )
{
	STATE(D3DRS_SRCBLEND,	D3DBLEND_ONE);
	STATE(D3DRS_DESTBLEND,	D3DBLEND_ONE);
}

void set_trans_state_9( void )
{
	STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
	STATE(D3DRS_DESTBLEND,	D3DBLEND_ONE);
}

void set_trans_state_5( void )
{
	STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
	STATE(D3DRS_DESTBLEND,	D3DBLEND_INVSRCALPHA);
}

// transparency tester
void render_state_trans( void )
{
	static int state = 0;
	static int count = 0;

	/*
    D3DBLEND_ZERO               = 1,
    D3DBLEND_ONE                = 2,
    D3DBLEND_SRCCOLOR           = 3,
    D3DBLEND_INVSRCCOLOR        = 4,
    D3DBLEND_SRCALPHA           = 5,
    D3DBLEND_INVSRCALPHA        = 6,
    D3DBLEND_DESTALPHA          = 7,
    D3DBLEND_INVDESTALPHA       = 8,
    D3DBLEND_DESTCOLOR          = 9,
    D3DBLEND_INVDESTCOLOR       = 10,
    D3DBLEND_SRCALPHASAT        = 11,
    D3DBLEND_BOTHSRCALPHA       = 12,
    D3DBLEND_BOTHINVSRCALPHA    = 13,
	*/

	// set the state
	switch( state )
	{
	case 0:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_ZERO);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_ONE);
		break;
	case 1:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCCOLOR);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_INVDESTCOLOR);
		break;
	case 2:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_ONE);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_ONE);
		break;
	case 3:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_SRCALPHA);
		break;
	case 4:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_INVSRCALPHA);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_SRCALPHA);
		break;
	case 5: // normal trans
		STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_INVSRCALPHA);
		break;
	// new modes
	case 6:
		STATE(D3DRS_SRCBLEND,	D3DBLEND_ONE);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_ZERO);
		break;
	case 7: // color trans
		STATE(D3DRS_SRCBLEND,	D3DBLEND_ZERO);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_SRCCOLOR);
		break;
	case 8: // inv color trans
		STATE(D3DRS_SRCBLEND,	D3DBLEND_ZERO);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_INVSRCCOLOR);
		break;
	case 9: // glowing
		STATE(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_ONE);
		break;
	case 10: // darkening color
		STATE(D3DRS_SRCBLEND,	D3DBLEND_INVDESTCOLOR);
		STATE(D3DRS_DESTBLEND,	D3DBLEND_ZERO);
		break;
	}
	DebugPrintf("trans state = %d ...... count %d\n",state,count);

	//FlipBuffers();

	// try the next state
	count++;
	if( count > 100 )	
	{
		count = 0;
		state++;
		if( state > 10 )
			state = 0;
	}
}

// set transparency off
void reset_trans( void )
{
	STATE(	D3DRS_ALPHABLENDENABLE,		FALSE );
	STATE(	D3DRS_SRCBLEND,				D3DBLEND_ONE);
	STATE(	D3DRS_DESTBLEND,			D3DBLEND_ZERO);
}

void reset_zbuff( void )
{
	STATE(	D3DRS_ZENABLE,			D3DZB_TRUE);
	STATE(	D3DRS_ZWRITEENABLE,		TRUE);
	STATE(	D3DRS_ZFUNC,			D3DCMP_LESS);
}

void disable_zbuff_write( void )
{
	STATE(	D3DRS_ZWRITEENABLE,		FALSE);
}

void disable_zbuff( void )
{
	STATE(	D3DRS_ZENABLE,			D3DZB_FALSE);
}

void reset_filtering( void )
{
	SSTATE(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
	SSTATE(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
	SSTATE(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

// i think this should be bileaner based on BilinearSolidScrPolys
void screenpoly_filtering( void )
{
	SSTATE(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	SSTATE(0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
}

void cull_none( void )
{
	STATE(	D3DRS_CULLMODE,	D3DCULL_NONE);
}

void cull_cw( void )
{
	STATE(	D3DRS_CULLMODE,	D3DCULL_CW);
}

void reset_cull( void )
{
	STATE(	D3DRS_CULLMODE,	D3DCULL_CCW);
}

void set_alpha_ignore( void )
{
	STATE( D3DRS_ALPHATESTENABLE,	TRUE); 
	STATE( D3DRS_ALPHAREF,			(DWORD)100 );
	STATE( D3DRS_ALPHAFUNC,			D3DCMP_GREATER);
}

void unset_alpha_ignore( void )
{
	STATE( D3DRS_ALPHATESTENABLE,	FALSE); 
}

// old lpD3DNormCmdBuf
void set_normal_states( void )
{
	reset_zbuff();
	reset_trans();
	// need to find equivalants
	//STATE( D3DRS_WRAPU, 				FALSE				);
	//STATE( D3DRS_WRAPV, 				FALSE				);
	//STATE( D3DRS_STIPPLEDALPHA,		FALSE				);
	//STATE( D3DRS_TEXTUREMAPBLEND, 	D3DTBLEND_MODULATE	);
}

// old lpD3DTransCmdBuf
void set_alpha_states( void )
{	   
//	if( UsedStippledAlpha == TRUE )
//  {
//      STATE( D3DRS_STIPPLEDALPHA , TRUE );
//	}else{
//#if ACTUAL_TRANS
//      STATE( D3DRS_ZWRITEENABLE, TRUE );
//#else
      disable_zbuff_write();
//#endif
      STATE( D3DRS_ALPHABLENDENABLE, TRUE );
	  // 9 seems to be perfect !
	  set_trans_state_9();
//      STATE( D3DRS_SRCBLEND, CurrentSrcBlend );
//      STATE( D3DRS_DESTBLEND, CurrentDestBlend );

	  // how to convert this ?

//      STATE( D3DRS_TEXTUREMAPBLEND, CurrentTextureBlend );
//	}
}

// old lpD3DSpcFxTransCmdBuf
void set_alpha_fx_states( void )
{  
//    if( UsedStippledAlpha == TRUE )
//      STATE( D3DRS_STIPPLEDALPHA , TRUE );
//	else
//    {
    STATE( D3DRS_ALPHABLENDENABLE, TRUE );
//    STATE( D3DRS_SRCBLEND, CurrentSrcBlend );
//    STATE( D3DRS_DESTBLEND, CurrentDestBlend );
//	  STATE( D3DRS_TEXTUREMAPBLEND, CurrentTextureBlend );
//    }
}

BOOL
D3DAppISetRenderState()
{
	//STATE( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	STATE(	D3DRS_SHADEMODE,		d3dapprs.ShadeMode);
	STATE(	D3DRS_SPECULARENABLE,	d3dapprs.bSpecular);
	STATE(	D3DRS_LIGHTING,			FALSE);

	reset_cull();
	reset_trans();
	reset_filtering();

	set_normal_states();

	return TRUE;
}

char buf[100];
HRESULT LastError;

BOOL FSClear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	if (FAILED(d3dappi.lpD3DDevice->Clear( Count, pRects, Flags, Color, Z, Stencil )))
	{
		return FALSE;
	}
	else return TRUE;
}

BOOL FSClearBlack(void)
{
	if (FAILED(d3dappi.lpD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, FSColourKeyBlack, 1.0f, 0 )))
	{
		return FALSE;
	}
	else return TRUE;
}

HRESULT FSGetViewPort(MYD3DVIEWPORT9 *returnViewPort)
{
	return d3dapp->lpD3DDevice->GetViewport( (D3DVIEWPORT9*) returnViewPort );
}

HRESULT FSSetViewPort(MYD3DVIEWPORT9 *newViewPort)
{
	return d3dapp->lpD3DDevice->SetViewport( (D3DVIEWPORT9*) newViewPort );
}

HRESULT FSSetMatrix(D3DTRANSFORMSTATETYPE type, const D3DMATRIX *matrix)
{
	return d3dappi.lpD3DDevice->SetTransform(type, matrix);
}

HRESULT FSGetMatrix(D3DTRANSFORMSTATETYPE type, D3DMATRIX *matrix)
{
	return d3dappi.lpD3DDevice->GetTransform(type, matrix);
}

HRESULT FSSetMaterial(const D3DMATERIAL9 *material)
{
	return d3dappi.lpD3DDevice->SetMaterial(material);
}

HRESULT FSBeginScene()
{
	return d3dappi.lpD3DDevice->BeginScene();
}

HRESULT FSEndScene()
{
	return d3dappi.lpD3DDevice->EndScene();
}

void save_texture( char * path, LPDIRECT3DTEXTURE9 texture )
{
	D3DXSaveTextureToFile(path, D3DXIFF_PNG, texture, 0);
}

char saveFile[MAX_PATH];

int imageCount = 0;

HRESULT FSCreateTexture(LPDIRECT3DTEXTURE9 *texture, const char *fileName, int width, int height, int numMips, BOOL * colourkey)
{
	D3DXIMAGE_INFO imageInfo;

	HRESULT LastError = D3DXCreateTextureFromFileEx(d3dappi.lpD3DDevice, 
				fileName, 
				width, 
				height, 
				numMips, 
				0,
				// most likely will end up as the format the file is in
				D3DFMT_A8R8G8B8,
				D3DPOOL_MANAGED,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				(*colourkey) ? FSColourKeyBlack : 0, // colour key
				&imageInfo,
				NULL,
				texture);

	if (FAILED(LastError))
	{
		OutputDebugString("couldn't create texture\n");
	}

	// image has no alpha layer
	if( imageInfo.Format != D3DFMT_A8R8G8B8 )
		(*colourkey) = FALSE;

	DebugPrintf("FSCreateTexture: %s\n",fileName);

	{
		static int count = 0;
		sprintf(buf, ".\\Dumps\\%s.png", fileName);
		D3DXSaveTextureToFile(buf, D3DXIFF_PNG, (*texture), 0);
		count++;
	}

	return LastError;
}

HRESULT FSCreateVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
//	assert (numVertices < 10000);

// this is not good cause LEVELRENDEROBJECT is of a diff size...
//	memset(renderObject, 0, sizeof(RENDEROBJECT));


	LastError = d3dappi.lpD3DDevice->CreateVertexBuffer(numVertices * sizeof(D3DLVERTEX), /*D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY*/0, D3DFVF_LVERTEX, D3DPOOL_MANAGED, &renderObject->lpD3DVertexBuffer, NULL);
	if (FAILED(LastError))
	{
		OutputDebugString("can't create vertex buffer\n");
	}

	OutputDebugString("created vertex buffer\n");

	return LastError;
}

HRESULT FSCreateDynamicVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
//	assert (numVertices < 10000);

// this is not good cause LEVELRENDEROBJECT is of a diff size...
//	memset(renderObject, 0, sizeof(RENDEROBJECT));

	LastError = d3dappi.lpD3DDevice->CreateVertexBuffer(numVertices * sizeof(D3DLVERTEX), /*D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY*/0, D3DFVF_LVERTEX, D3DPOOL_MANAGED, &renderObject->lpD3DVertexBuffer, NULL);
	if (FAILED(LastError))
	{
		OutputDebugString("can't create vertex buffer\n");
	}

	OutputDebugString("created vertex buffer\n");

	return LastError;
}


HRESULT FSCreatePretransformedVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
//	assert (numVertices < 10000);

	assert (renderObject->lpD3DVertexBuffer == NULL);

// this is not good cause LEVELRENDEROBJECT is of a diff size...
//	memset(renderObject, 0, sizeof(RENDEROBJECT));

	LastError = d3dappi.lpD3DDevice->CreateVertexBuffer(numVertices * sizeof(D3DTLVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_TLVERTEX, D3DPOOL_DEFAULT, &renderObject->lpD3DVertexBuffer, NULL);
	if (FAILED(LastError))
	{
		OutputDebugString("can't create vertex buffer\n");
	}

	OutputDebugString("created vertex buffer\n");

	return LastError;
}

int lockTest = 0;

HRESULT FSLockVertexBuffer(RENDEROBJECT *renderObject, D3DLVERTEX **verts)
{
	assert(renderObject->vbLocked == 0);

	/* TODO - check the Lock type flag. Do we ever need to discard? read only? */
	LastError = renderObject->lpD3DVertexBuffer->Lock(0, 0, (void**)verts, 0);
	if (FAILED(LastError))
	{
		OutputDebugString("can't lock vertex buffer!\n");
	}

	renderObject->vbLocked = TRUE;
	lockTest++;

//	OutputDebugString("locked vertex buffer\n");

	return LastError;
}

HRESULT FSLockPretransformedVertexBuffer(RENDEROBJECT *renderObject, D3DTLVERTEX **verts)
{
	assert(renderObject->vbLocked == 0);

	/* TODO - check the Lock type flag. Do we ever need to discard? read only? */
	LastError = renderObject->lpD3DVertexBuffer->Lock(0, 0, (void**)verts, 0);
	if (FAILED(LastError))
	{
		OutputDebugString("can't lock vertex buffer!\n");
	}

	renderObject->vbLocked = TRUE;
	lockTest++;

//	OutputDebugString("locked vertex buffer\n");

	return LastError;
}

HRESULT FSUnlockVertexBuffer(RENDEROBJECT *renderObject)
{
	assert(renderObject->vbLocked == 1);

//	OutputDebugString("unlocking vertex buffer\n");
	LastError = renderObject->lpD3DVertexBuffer->Unlock();
	if (FAILED(LastError))
	{
		OutputDebugString("can't unlock vertex buffer!\n");
	}

	renderObject->vbLocked = FALSE;
	lockTest--;

//	OutputDebugString("unlocked vertex buffer\n");

	return LastError;
}

// can just use the above if we want...
HRESULT FSUnlockPretransformedVertexBuffer(RENDEROBJECT *renderObject)
{
	assert(renderObject->vbLocked == 1);

//	OutputDebugString("unlocking vertex buffer\n");
	LastError = renderObject->lpD3DVertexBuffer->Unlock();
	if (FAILED(LastError))
	{
		OutputDebugString("can't unlock vertex buffer!\n");
	}

	renderObject->vbLocked = FALSE;
	lockTest--;

//	OutputDebugString("unlocked vertex buffer\n");

	return LastError;
}

HRESULT FSCreateIndexBuffer(RENDEROBJECT *renderObject, int numIndices)
{
	LastError = d3dappi.lpD3DDevice->CreateIndexBuffer(numIndices * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &renderObject->lpD3DIndexBuffer, NULL);
	if (FAILED(LastError))
	{
		OutputDebugString("can't create vertex buffer\n");
	}

	OutputDebugString("created vertex buffer\n");

	return LastError;
}

HRESULT FSLockIndexBuffer(RENDEROBJECT *renderObject, WORD **indices)
{
	LastError = renderObject->lpD3DIndexBuffer->Lock(0, 0, (void**)indices, 0);
	if (FAILED(LastError))
	{
		OutputDebugString("can't lock index buffer!\n");
	}

	return LastError;
}

HRESULT FSUnlockIndexBuffer(RENDEROBJECT *renderObject)
{
	LastError = renderObject->lpD3DIndexBuffer->Unlock();
	if (FAILED(LastError))
	{
		OutputDebugString("can't lock index buffer!\n");
	}

	return LastError;
}

HRESULT draw_render_object( RENDEROBJECT *renderObject, BOOL transformed /*aka 2d*/, D3DPRIMITIVETYPE primitive_type )
{
	HRESULT LastError;

	assert(renderObject->vbLocked == 0);

	if( transformed )
		LastError = d3dappi.lpD3DDevice->SetStreamSource(0, renderObject->lpD3DVertexBuffer, 0, sizeof(D3DTLVERTEX));
	else
		LastError = d3dappi.lpD3DDevice->SetStreamSource(0, renderObject->lpD3DVertexBuffer, 0, sizeof(D3DLVERTEX));

	if (FAILED(LastError))
		return LastError;

	if(renderObject->lpD3DIndexBuffer)
	{
		LastError = d3dappi.lpD3DDevice->SetIndices(renderObject->lpD3DIndexBuffer);
		if(FAILED(LastError)) 
			return LastError;
	}

	if( transformed )
		LastError = d3dappi.lpD3DDevice->SetFVF(D3DFVF_TLVERTEX);
	else
		LastError = d3dappi.lpD3DDevice->SetFVF(D3DFVF_LVERTEX);

	if (FAILED(LastError))
		return LastError;

	LastError = d3dappi.lpD3DDevice->SetMaterial(&renderObject->material);
	if (FAILED(LastError))
		return LastError;

	for (int i = 0; i < renderObject->numTextureGroups; i++)
	{
		if(renderObject->textureGroups[i].colourkey)
			set_alpha_ignore();

		LastError = d3dappi.lpD3DDevice->SetTexture(0, renderObject->textureGroups[i].texture);
		if (FAILED(LastError))
			return LastError;

		if(renderObject->lpD3DIndexBuffer)
		{
			LastError = d3dappi.lpD3DDevice->DrawIndexedPrimitive(
				primitive_type,
				renderObject->textureGroups[i].startVert,
				0, 
				renderObject->textureGroups[i].numVerts,
				renderObject->textureGroups[i].startIndex,
				renderObject->textureGroups[i].numTriangles
			);
		}
		else
		{
			LastError = d3dappi.lpD3DDevice->DrawPrimitive(
				primitive_type,
				renderObject->textureGroups[i].startVert,
				renderObject->textureGroups[i].numVerts
			);
		}

		unset_alpha_ignore();

		if (FAILED(LastError))
			return LastError;
	}

	return S_OK;
}

HRESULT draw_line_object(RENDEROBJECT *renderObject)
{
	return draw_render_object( renderObject, FALSE, D3DPT_LINELIST );
}

HRESULT draw_object(RENDEROBJECT *renderObject)
{
	return draw_render_object( renderObject, FALSE, D3DPT_TRIANGLELIST );
}

HRESULT draw_2d_object(RENDEROBJECT *renderObject)
{
	return draw_render_object( renderObject, TRUE, D3DPT_TRIANGLELIST );
}

void FSReleaseRenderObject(RENDEROBJECT *renderObject)
{
	if (renderObject->lpD3DVertexBuffer)
	{
		renderObject->lpD3DVertexBuffer->Release();
		renderObject->lpD3DVertexBuffer = NULL;
	}
	if (renderObject->lpD3DIndexBuffer)
	{
		renderObject->lpD3DIndexBuffer->Release();
		renderObject->lpD3DIndexBuffer = NULL;
	}
	for (int i = 0; i < renderObject->numTextureGroups; i++)
	{
		renderObject->textureGroups[i].numVerts = 0;
		renderObject->textureGroups[i].startVert = 0;

		if (renderObject->textureGroups[i].texture)
		{
			// the texture code probably already deals with this.
			// but we need to figure out why this is crashing for sure.
			//renderObject->textureGroups[i].texture->Release();
			renderObject->textureGroups[i].texture = NULL;
		}
	}
}

LPDIRECT3DSURFACE9 FSLoadBitmap(char* pathname, D3DCOLOR m_ColourKey )
{
    HBITMAP             hbm;
    HRESULT             hr;
	BITMAP				Bitmap;
	LPDIRECT3DSURFACE9	pdds = NULL;
    hbm = (HBITMAP)LoadImage(NULL, pathname, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
    if (hbm == NULL)
    {
		DebugPrintf("FSLoadBitmap: Handle is null\n");
        return NULL;
    }
	GetObject(hbm, sizeof(BITMAP), &Bitmap);
    DeleteObject(hbm);
	hr=d3dappi.lpD3DDevice->CreateOffscreenPlainSurface(
		Bitmap.bmWidth, Bitmap.bmHeight,
		D3DFMT_A8R8G8B8, // 32 bit alpha channel
		D3DPOOL_SYSTEMMEM,
		&pdds,
		NULL
	);
	if(FAILED(hr))
	{
        DebugPrintf("FSLoadBitmap: CreateOffscreenPlainSurface failed\n");
	}
	// set colour key black by default
	hr=D3DXLoadSurfaceFromFile(pdds, NULL, NULL, pathname, NULL, D3DX_FILTER_NONE, m_ColourKey, NULL);
	if(FAILED(hr))
	{
        DebugPrintf("FSLoadBitmap: D3DXLoadSurfaceFromFile failed\n");
	}
    return pdds;
}

// pass FSBackBuffer or NULL for "to" argument to point to back buffer
void FSBlit(LPDIRECT3DSURFACE9 from, LPDIRECT3DSURFACE9 to, RECT * src, POINT * dest )
{
	HRESULT hr;
	if(!from)
	{
		DebugPrintf("FSBlit: !pdds\n");
		return;
	}
	if(to)
	{
		d3dappi.lpD3DDevice->UpdateSurface(from, src, to, dest);
	}
	else
	{
		hr=d3dappi.lpD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &to);
		if(FAILED(hr))
		{
			DebugPrintf("FSBlit: GetBackBuffer failed\n");
			return;
		}
		if(!to)
		{
			DebugPrintf("FSBlit: !pRenderSurface\n");
			return;
		}
		d3dappi.lpD3DDevice->UpdateSurface(from, src, to, dest);
		to->Release();
	}
}

};