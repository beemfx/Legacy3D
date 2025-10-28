//Obsolete test stuff...

#define LVTVERTEX_TYPE (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2)
typedef struct _LVTVERTEX{
	float x, y, z;
	float nx, ny, nz;
	lg_dword diffuse;
	float tu, tv;
	float tcu, tcv;
}LVTVERTEX;

class CLExplorMap
{
private:
	lf_bool m_bOpen;
	lg_word  m_nWidth;
	lg_word  m_nHeight;
	lg_word* m_pTiles;
	
	IDirect3DDevice9* m_pDevice;
	IDirect3DTexture9* m_pWallTex;
	IDirect3DTexture9* m_pDoorTex;
	IDirect3DVertexBuffer9* m_pVB;
public:
	CLExplorMap();
	~CLExplorMap();
	void Open(IDirect3DDevice9* pDevice, lf_path szFilename);
	void Validate();
	void Invalidate();
	void Render();
	void Close();
};




CLExplorMap::CLExplorMap():
	m_bOpen(LG_FALSE),
	m_nWidth(0),
	m_nHeight(0),
	m_pTiles(LG_NULL),
	m_pWallTex(LG_NULL),
	m_pDoorTex(LG_NULL)
{
	
}
CLExplorMap::~CLExplorMap()
{
	Close();
}

void CLExplorMap::Open(IDirect3DDevice9* pDevice, lf_path szFilename)
{
	Close();
	LF_FILE3 fileMap=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fileMap)
		return;
	lg_word nWord1, nWord2;
	LF_Read(fileMap, &nWord1, 2);
	LF_Read(fileMap, &nWord2, 2);
	if(nWord1!=*(lg_word*)"EM" || nWord2!=3)
	{
		LF_Close(fileMap);
		return;
	}
	//Really the only information needed
	//is the width and height, the rest of the information
	//isn't even used (or necessary).
	LF_Seek(fileMap, LF_SEEK_CURRENT, 8);
	LF_Read(fileMap, &m_nWidth, 2);
	LF_Read(fileMap, &m_nHeight, 2);
	LF_Seek(fileMap, LF_SEEK_CURRENT, 14);

	m_pTiles=new lg_word[m_nWidth*m_nHeight];
	LF_Read(fileMap, m_pTiles, m_nWidth*m_nHeight*sizeof(lg_word));
	LF_Close(fileMap);
	//We now have the map all read, so we should prepare the rendering features.
	m_pDevice=pDevice;
	Validate();
	m_bOpen=LG_TRUE;
}

void CLExplorMap::Validate()
{
	if(!m_pWallTex)
		m_pWallTex=Tex_Load2("/dbase/Explor Test/explor_wall.tga");
	if(!m_pDoorTex)
		m_pDoorTex=Tex_Load2("/dbase/Explor Test/explor_door.tga");
}

void CLExplorMap::Invalidate()
{
	L_safe_release(m_pWallTex);
	L_safe_release(m_pDoorTex);
}

void CLExplorMap::Render()
{
	if(!m_bOpen)
		return;
		
	LVTVERTEX verts[10];
		
	ML_MAT matTrans;
	ML_MatIdentity(&matTrans);
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTrans);
	
	m_pDevice->SetFVF(D3DFVF_XYZ|D3DFVF_TEX2);
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_ALPHABLENDENABLE, FALSE);
		
	for(lg_short nx=-1; nx<m_nWidth+1; nx++)
	{
		for(lg_short ny=-1; ny<m_nHeight+1; ny++)
		{
			#define EMAP_SCALE 2.0f
			
			float fx=nx*EMAP_SCALE, fy=ny*EMAP_SCALE;
			
			if(ny<0 || nx<0 || nx>=m_nWidth || ny>=m_nHeight)
			{
				m_pDevice->SetTexture(0, m_pWallTex);
				goto Rasterize;
			}
			
			switch(m_pTiles[nx+m_nWidth*ny])
			{
			case 10:
				m_pDevice->SetTexture(0, m_pWallTex);
				break;
			case 20:
				m_pDevice->SetTexture(0, m_pDoorTex);
				break;
			case 0:
			default:
				m_pDevice->SetTexture(0, m_pWallTex);
				verts[0].x=fx;
				verts[0].y=0.0f;
				verts[0].z=fy;
				verts[0].nx=0.0f;
				verts[0].ny=1.0f;
				
				verts[1].x=fx;
				verts[1].y=0.0f;
				verts[1].z=fy+EMAP_SCALE;
				verts[1].nx=0.0f;
				verts[1].ny=0.0f;
				
				verts[3].x=fx+EMAP_SCALE;
				verts[3].y=0.0f;
				verts[3].z=fy+EMAP_SCALE;
				verts[3].nx=1.0f;
				verts[3].ny=0.0f;
				
				verts[2].x=fx+EMAP_SCALE;
				verts[2].y=0.0f;
				verts[2].z=fy;
				verts[2].nx=1.0f;
				verts[2].ny=1.0f;
				
				m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(LVTVERTEX));
				continue;
				break;
			}
			Rasterize:
			
			//Render the square...
			verts[0].x=fx;
			verts[0].y=0.0f;
			verts[0].z=fy;
			verts[0].nx=0.0f;
			verts[0].ny=1.0f;
			
			verts[1].x=fx;
			verts[1].y=EMAP_SCALE;
			verts[1].z=fy;
			verts[1].nx=0.0f;
			verts[1].ny=0.0f;
			
			verts[2].x=fx+EMAP_SCALE;
			verts[2].y=0.0f;
			verts[2].z=fy;
			verts[2].nx=1.0f;
			verts[2].ny=1.0f;
			
			verts[3].x=fx+EMAP_SCALE;
			verts[3].y=EMAP_SCALE;
			verts[3].z=fy;
			verts[3].nx=1.0f;
			verts[3].ny=0.0f;
			
			verts[4].x=fx+EMAP_SCALE;
			verts[4].y=0.0f;
			verts[4].z=fy+EMAP_SCALE;
			verts[4].nx=0.0f;
			verts[4].ny=1.0f;
			
			verts[5].x=fx+EMAP_SCALE;
			verts[5].y=EMAP_SCALE;
			verts[5].z=fy+EMAP_SCALE;
			verts[5].nx=0.0f;
			verts[5].ny=0.0f;
			
			verts[6].x=fx;
			verts[6].y=0.0f;
			verts[6].z=fy+EMAP_SCALE;
			verts[6].nx=1.0f;
			verts[6].ny=1.0f;
			
			verts[7].x=fx;
			verts[7].y=EMAP_SCALE;
			verts[7].z=fy+EMAP_SCALE;
			verts[7].nx=1.0f;
			verts[7].ny=0.0f;
			
			verts[8].x=fx;
			verts[8].y=0.0f;
			verts[8].z=fy;
			verts[8].nx=0.0f;
			verts[8].ny=1.0f;
			
			verts[9].x=fx;
			verts[9].y=EMAP_SCALE;
			verts[9].z=fy;
			verts[9].nx=0.0f;
			verts[9].ny=0.0f;
			
			
			
			m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 8, verts, sizeof(LVTVERTEX));
			
			verts[0].x=fx;
			verts[0].y=EMAP_SCALE;
			verts[0].z=fy;
			
			verts[1].x=fx;
			verts[1].y=EMAP_SCALE;
			verts[1].z=fy+EMAP_SCALE;
			
			verts[3].x=fx+EMAP_SCALE;
			verts[3].y=EMAP_SCALE;
			verts[3].z=fy+EMAP_SCALE;
			
			verts[2].x=fx+EMAP_SCALE;
			verts[2].y=EMAP_SCALE;
			verts[2].z=fy;
			
			m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(LVTVERTEX));
			
			
		}
	}
}

void CLExplorMap::Close()
{
	if(!m_bOpen)
		return;
	L_safe_delete_array(m_pTiles);
	Invalidate();
	m_bOpen=LG_FALSE;
}



void CLVTObj::RenderTestWall()
{
	ML_MAT matTrans;
	ML_MatIdentity(&matTrans);
	IDirect3DDevice9_SetFVF(m_pDevice, LVTVERTEX_TYPE);
	IDirect3DDevice9_SetStreamSource(m_pDevice,0,m_lpVB,0,sizeof(LVTVERTEX));
	ML_MatIdentity(&matTrans);
	//matTrans._43=50.0f;
	//matTrans._42=0.0f;
	//m_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&matTrans);
	//ML_MatRotationY(&matTrans, LT_GetTimeMS()/-3000.0f);
	//ML_MatIdentity(&matTrans);
	ML_MAT matSize;
	#define SCALE 0.10f
	D3DXMatrixScaling((D3DXMATRIX*)&matSize, SCALE, SCALE, SCALE);
	ML_MatMultiply(&matTrans, &matTrans, &matSize);
	matTrans._43=0.0f;
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTrans);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	#define MULTIPASS
	#ifdef MULTIPASS
	IDirect3DDevice9_SetTexture(m_pDevice,0,m_lpTex);
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_DrawPrimitive(m_pDevice,D3DPT_TRIANGLELIST, 0, 200);
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetTexture(m_pDevice,0,m_lpLM);
	IDirect3DDevice9_SetTextureStageState(m_pDevice, 0, D3DTSS_TEXCOORDINDEX, 1);
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_SRCBLEND, D3DBLEND_ZERO);
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	IDirect3DDevice9_DrawPrimitive(m_pDevice,D3DPT_TRIANGLELIST, 0, 200);
	IDirect3DDevice9_SetTextureStageState(m_pDevice, 0, D3DTSS_TEXCOORDINDEX, 0);
	#else
	m_pDevice->SetTexture(0,m_lpTex);
	m_pDevice->SetTexture(1,m_lpLM);
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 200);
	m_pDevice->SetTexture(1, LG_NULL);
	#endif
	IDirect3DDevice9_SetRenderState(m_pDevice, D3DRS_CULLMODE, D3DCULL_CCW);
}


void CLVTObj::ValidateTestWall()
{
	// Stuff for the test wall.
		LVTVERTEX lpVerts[600];
		int x=0, y=0, i=0;
		lg_result nResult=0;
		void* lpBuffer=LG_NULL;
		
		//Code for test wall.
		m_lpTex=Tex_Load2("/dbase/textures/graybrick.tga");
		m_lpLM=Tex_Load2("/dbase/textures/misc/spotdown01lm.tga");

		
		for(x=0, i=0; x<10; x++)
		{
			for(y=0; y<10; y++, i+=6)
			{
				lg_bool bExtY=LG_FALSE;
				lg_bool bExtX=LG_FALSE;

				float fTop=0.0f;
				float fBottom=1.0f;
				float fLeft=0.0f;
				float fRight=1.0f;

				if(!(x%2))
				{
					fLeft=0.0f;
					fRight=0.5f;
				}
				else
				{
					fLeft=0.5f;
					fRight=1.0f;
				}

				if(!(y%2))
				{
					fTop=0.0f;
					fBottom=0.5f;
				}
				else
				{
					fTop=0.5f;
					fBottom=1.0f;
				}

				lpVerts[i].x=x*10.0f-50.0f;
				lpVerts[i].y=0.0f;
				lpVerts[i].z=y*10.0f-50.0f;
				lpVerts[i].nx=0.0f;
				lpVerts[i].nz=0.0f;
				lpVerts[i].ny=1.0f;
				lpVerts[i].diffuse=0xFFFFFFFF;
				lpVerts[i].tu=0.0f;
				lpVerts[i].tv=1.0f;
				lpVerts[i].tcu=fLeft;
				lpVerts[i].tcv=fBottom;

				lpVerts[i+1].x=x*10.0f+10.0f-50.0f;
				lpVerts[i+1].y=0.0f;
				lpVerts[i+1].z=y*10.0f+10.0f-50.0f;
				lpVerts[i+1].nx=0.0f;
				lpVerts[i+1].nz=0.0f;
				lpVerts[i+1].ny=1.0f;
				lpVerts[i+1].diffuse=0xFFFFFFFF;
				lpVerts[i+1].tu=1.0f;
				lpVerts[i+1].tv=0.0f;
				lpVerts[i+1].tcu=fRight;
				lpVerts[i+1].tcv=fTop;;

				lpVerts[i+2].x=x*10.0f+10.0f-50.0f;
				lpVerts[i+2].y=0.0f;
				lpVerts[i+2].z=y*10.0f-50.0f;
				lpVerts[i+2].nx=0.0f;
				lpVerts[i+2].nz=0.0f;
				lpVerts[i+2].ny=1.0f;
				lpVerts[i+2].diffuse=0xFFFFFFFF;
				lpVerts[i+2].tu=1.0f;
				lpVerts[i+2].tv=1.0f;
				lpVerts[i+2].tcu=fRight;
				lpVerts[i+2].tcv=fBottom;

				memcpy(&lpVerts[i+3], &lpVerts[i+1], sizeof(lpVerts[0]));

				memcpy(&lpVerts[i+4], &lpVerts[i], sizeof(lpVerts[0]));

				lpVerts[i+5].x=x*10.0f-50.0f;
				lpVerts[i+5].y=0.0f;
				lpVerts[i+5].z=y*10.0f+10.0f-50.0f;
				lpVerts[i+5].nx=0.0f;
				lpVerts[i+5].nz=0.0f;
				lpVerts[i+5].ny=1.0f;
				lpVerts[i+5].diffuse=0x00FFFFFF;
				lpVerts[i+5].tu=0.0f;
				lpVerts[i+5].tv=0.0f;
				lpVerts[i+5].tcu=fLeft;
				lpVerts[i+5].tcv=fTop;
			}
		}

		nResult=IDirect3DDevice9_CreateVertexBuffer(
			m_pDevice,
			sizeof(LVTVERTEX)*600, 
			0, 
			LVTVERTEX_TYPE, 
			D3DPOOL_MANAGED, 
			&m_lpVB, 
			LG_NULL);

		if(LG_FAILED(nResult))
		{
			Err_PrintDX("IDirect3DDevice9::CreateVertexBuffer", nResult);
			L_safe_release(m_lpLM);
			L_safe_release(m_lpTex);
			return;
		}

		nResult=IDirect3DVertexBuffer9_Lock(m_lpVB, 0, sizeof(LVTVERTEX)*600, &lpBuffer, 0);
		if(lpBuffer)
			memcpy(lpBuffer, &lpVerts, sizeof(LVTVERTEX)*600);
		IDirect3DVertexBuffer9_Unlock(m_lpVB);
}

