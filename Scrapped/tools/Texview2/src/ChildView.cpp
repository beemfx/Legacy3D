// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Texview2.h"
#include "ChildView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView():
m_nFilter(IMGFILTER_LINEAR),
m_bAlphaChannel(FALSE),
m_bShowImage(TRUE),
m_bBlackBG(FALSE),
m_nImageHeight(0),
m_nImageWidth(0)
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_VIEW_ALPHACHANNEL, OnViewAlphachannel)
	ON_COMMAND(ID_VIEW_IMAGE, OnViewImage)
	ON_WM_MOVE()
	ON_COMMAND(ID_VIEW_BLACKBG, OnViewBlackbg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_APPWORKSPACE+1), 0);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	RECT rc;
	int cx, cy;
	int x, y;
	m_texImage.GetBitmapDims(&cx, &cy);
	GetClientRect(&rc);

	dc.SetMapMode(MM_TEXT);
	dc.SetViewportOrg(0, 0);

	x=cx>rc.right?GetScrollPos(SB_HORZ):(cx-rc.right)/2;
	y=cy>rc.bottom?GetScrollPos(SB_VERT):(cy-rc.bottom)/2;

	m_texImage.DrawBitmap(x, y, &dc);
}


void CChildView::UpdateScrollBars(BOOL bResetToZero)
{
	int width=0, height=0;
	m_texImage.GetBitmapDims(&width, &height);
	if(TRUE)
	{
		//BITMAP bm;
		//m_bmTexture.GetBitmap(&bm);
		//Setup the scroll information.
		RECT rcClient;
		GetClientRect(&rcClient);
		SCROLLINFO si;
		si.cbSize=sizeof(SCROLLINFO);
		si.nMin=0;
		si.nMax=width-1;
		si.nPos=bResetToZero?0:GetScrollPos(SB_HORZ);
		si.nPage=rcClient.right;
		si.fMask=SIF_ALL;
		SetScrollInfo(SB_HORZ, &si, TRUE);
		si.nMax=height-1;
		si.nPos=bResetToZero?0:GetScrollPos(SB_VERT);
		si.nPage=rcClient.bottom;
		si.fMask=SIF_ALL;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	else
	{
		SCROLLINFO si;
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_RANGE|SIF_PAGE;
		si.nMax=0;
		si.nMin=0;
		si.nPage=0;
		SetScrollInfo(SB_HORZ, &si, TRUE);
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
}

BOOL CChildView::LoadTexture(LPTSTR szFilename)
{
	//Make sure no image is loaded.
	//m_bmTexture.DeleteObject();
	m_texImage.Unload();
	//Get the windows dc for loading the image.
	CDC* dc=GetDC();
	//Load the image.
	//m_bmTexture.m_hObject=(HBITMAP)TGA_CreateDIBitmap(szFilename, dc->m_hDC);
	BOOL bLoaded=m_texImage.Load(szFilename);

	//Reset the alpha channel option to off.
	m_bAlphaChannel=FALSE;
	m_bShowImage=TRUE;
	CMenu* menu=GetParent()->GetMenu();
	menu->CheckMenuItem(ID_VIEW_ALPHACHANNEL, MF_BYCOMMAND|MF_UNCHECKED);
	menu->CheckMenuItem(ID_VIEW_IMAGE, MF_BYCOMMAND|MF_CHECKED);
	menu->Detach();

	//We set the bitmap size to 0, 0, which makes it the default size.
	SetBitmapSize(0, 0);
	//Get rid of the dc and return.
	ReleaseDC(dc);
	UpdateScrollBars(TRUE);
	RedrawWindow();

	if(bLoaded)
	{
		TCHAR szWindowText[MAX_PATH+100];
		_stprintf(szWindowText, _T("Texture Viewer 2 [%s]"), szFilename);
		GetParentFrame()->SetWindowText(szWindowText);
		int cx, cy;
		m_texImage.GetBitmapDims(&cx, &cy);
		((CMainFrame*)GetParentFrame())->UpdateMenuTexSizes(cx, cy);
		return TRUE;
	}
	else
	{
		MessageBox(_T("Could not open texture.\nFile may not be valid."), _T("Texture Viewer 2"), MB_ICONINFORMATION);
		GetParentFrame()->SetWindowText(_T("Texture Viewer 2"));
		((CMainFrame*)GetParentFrame())->UpdateMenuTexSizes(0, 0);
		return FALSE;
	}
	return TRUE;
}



void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	UpdateScrollBars(FALSE);
}

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
SCROLLINFO si;
	GetScrollInfo(SB_VERT, &si, SIF_ALL);
	int nOldPos=si.nPos;
	switch(nSBCode)
	{
	case SB_TOP:
		nPos=si.nMin;
		break;
	case SB_BOTTOM:
		si.nPos=si.nMax;
		break;
	case SB_LINEUP:
		si.nPos-=5;
		break;
	case SB_LINEDOWN:
		si.nPos+=5;
		break;
	case SB_PAGEUP:
		si.nPos-=si.nPage;
		break;
	case SB_PAGEDOWN:
		si.nPos+=si.nPage;
		break;
	case SB_THUMBPOSITION:
		si.nPos=nPos;
		break;
	case SB_THUMBTRACK:
		si.nPos=nPos;
		break;
	}

	SetScrollInfo(SB_VERT, &si, TRUE);
	ScrollWindow(0, nOldPos-GetScrollPos(SB_VERT), NULL, NULL);	
	CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO si;
	GetScrollInfo(SB_HORZ, &si, SIF_ALL);
	int nOldPos=si.nPos;
	switch(nSBCode)
	{
	case SB_LEFT:
		nPos=si.nMin;
		break;
	case SB_RIGHT:
		si.nPos=si.nMax;
		break;
	case SB_LINELEFT:
		si.nPos-=5;
		break;
	case SB_LINERIGHT:
		si.nPos+=5;
		break;
	case SB_PAGELEFT:
		si.nPos-=si.nPage;
		break;
	case SB_PAGERIGHT:
		si.nPos+=si.nPage;
		break;
	case SB_THUMBPOSITION:
		si.nPos=nPos;
		break;
	case SB_THUMBTRACK:
		si.nPos=nPos;
		break;
	}

	SetScrollInfo(SB_HORZ, &si, TRUE);
	ScrollWindow(nOldPos-GetScrollPos(SB_HORZ), 0, NULL, NULL);
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CChildView::SetBitmapSize(int cx, int cy)
{
	CDC* pdc=GetDC();
	m_nImageWidth=cx;
	m_nImageHeight=cy;
	UpdateImage();
	m_texImage.GetBitmapDims(&m_nImageWidth, &m_nImageHeight);
}

void CChildView::SetFilter(IMGFILTER filter)
{
	//If the filter hasn't changed we won't change it.
	if(filter == m_nFilter)
		return;

	m_nFilter=filter;
	UpdateImage();
}

void CChildView::OnViewAlphachannel() 
{
	m_bAlphaChannel=!m_bAlphaChannel;

	DWORD nCheck=m_bAlphaChannel?MF_CHECKED:MF_UNCHECKED;

	CMenu* menu=GetParent()->GetMenu();
	if(!m_bAlphaChannel)
	{
		m_bShowImage=TRUE;
		menu->CheckMenuItem(ID_VIEW_IMAGE, MF_BYCOMMAND|MF_CHECKED);
	}
	menu->CheckMenuItem(ID_VIEW_ALPHACHANNEL, MF_BYCOMMAND|nCheck);
	menu->Detach();
	UpdateImage();
}


void CChildView::OnViewImage() 
{
	m_bShowImage=!m_bShowImage;

	if(!m_bShowImage && !m_bAlphaChannel)
	{
		m_bShowImage=TRUE;
		return;
	}

	DWORD nCheck=m_bShowImage?MF_CHECKED:MF_UNCHECKED;

	CMenu* menu=GetParent()->GetMenu();
	menu->CheckMenuItem(ID_VIEW_IMAGE, MF_BYCOMMAND|nCheck);
	menu->Detach();
	UpdateImage();
}

void CChildView::OnViewBlackbg() 
{
	m_bBlackBG=!m_bBlackBG;
	DWORD nCheck=m_bBlackBG?MF_CHECKED:MF_UNCHECKED;
	CMenu* menu=GetParent()->GetMenu();
	menu->CheckMenuItem(ID_VIEW_BLACKBG, MF_BYCOMMAND|nCheck);
	menu->Detach();

	if(!m_bAlphaChannel)
		return;

	UpdateImage();
}


//Update Image takes all the stored values and resizes creates
//the CBitmap to fit all the menu options that are selected.
void CChildView::UpdateImage()
{
	CDC* pdc=GetDC();
	DWORD dwFlags=0;
	dwFlags|=m_bAlphaChannel?BC_ACHANNEL:0;
	dwFlags|=m_bShowImage?BC_IMAGE:0;
	dwFlags|=m_bBlackBG?0:BC_USEAPPBG;
	m_texImage.CreateCBitmap(m_nImageWidth, m_nImageHeight, m_nFilter, pdc, dwFlags);
	ReleaseDC(pdc);
	UpdateScrollBars(TRUE);
	Invalidate();
}
