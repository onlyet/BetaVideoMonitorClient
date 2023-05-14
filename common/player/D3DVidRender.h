#pragma once
#include "d3d9.h"

class CD3DVidRender
{
public:
	CD3DVidRender(void);
	~CD3DVidRender(void);
	
	void Cleanup();

	BOOL InitD3D_RGB32(HWND hwnd, int img_width, int img_height,int nFlip=0);

	BOOL InitD3D_YUV(HWND hwnd, int img_width, int img_height);

	BOOL InitD3D_NV12(HWND hwnd, int img_width, int img_height);

	BOOL Render(unsigned char* pdata, int width, int height)
	{
		if (m_nColor == 0)
			return Render_YUV(pdata, width, height);
		else if (m_nColor == 1)
			return Render_NV12(pdata, width, height);
		else if (m_nColor == 2)
			return Render_RGB32(pdata, width, height);
		return Render_YUV(pdata, width, height);
	}

	BOOL Render_RGB32(unsigned char* pdata, int width, int height);

	BOOL Render_YUV(unsigned char * pdata, int img_width, int img_height);

	BOOL Render_NV12(unsigned char * pdata, int img_width, int img_height);

	void calculate_display_rect(RECT *rect,int img_width, int img_height, int scr_width, int scr_height) ;

public:
    int                   m_nFlip;
    int                   m_nColor;
    RECT                  m_rtViewport;
    IDirect3D9*           m_pDirect3D9;
    D3DPRESENT_PARAMETERS d3dpp;  // 显示参数
    IDirect3DDevice9*     m_pDirect3DDevice;
    IDirect3DSurface9*    m_pDirect3DSurfaceRender;  // 离屏表面
    IDirect3DSurface9*    m_pBackBuffer;             // 后台缓冲表面

	RECT m_rtFont;
};
