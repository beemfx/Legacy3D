/* lv_con.h - the graphical console. */
#ifndef __LV_CON_H__
#define __LV_CON_H__
#include "lv_img2d.h"
#include "lv_font.h"

#define LK_PAGEUP   1
#define LK_PAGEDOWN 2
#define LK_END      3

#define CON_KEY1  ('~')
#define CON_KEY2  ('`')
#define CON_KEYDX (DIK_GRAVE)

class CLVCon
{
private:
	//LV2DIMAGE*        m_pBackground; /* The background image. */
	CLImg2D           m_Background;
	
	CLFont*           m_lpFont;
	//void*             m_pFont;       /* The font. */
	lg_dword           m_dwViewHeight;  /* The view height. */
	lg_dword           m_dwFontHeight;
	//HLCONSOLE         m_hConsole;     /* Pointer to the console. */
	IDirect3DDevice9* m_pDevice;     /* The device that created teh console. */
	lg_bool            m_bActive;      /* Whether or not the console is receiving input. */
	lg_bool            m_bDeactivating;/* If the console is in the process of deactivating (scrolling up).*/
	float             m_fPosition;    /* The position of the console. */
	long              m_nScrollPos;  /* Where the console is rendering output. */
	lg_dword           m_dwLastUpdate; /* The last time the console position was updated. */
	lg_bool            m_bCaretOn;
	lg_dword           m_dwLastCaretChange;
	lg_bool            m_bFullMode;
	lg_char            m_szMessage[128];
	lg_char            m_szMessage2[128];
public:
	CLVCon();
	CLVCon(IDirect3DDevice9* pDevice);
	~CLVCon();
	lg_bool Create(
		IDirect3DDevice9* pDevice);

	void Delete();

	lg_bool Render();
	lg_bool Validate();
	lg_bool Invalidate();
	lg_bool OnCharA(char c);
	void SetMessage(lg_dword nSlot, lg_char* szMessage);
	lg_bool IsActive();
	
	void Toggle();
	
private:
	lg_bool UpdatePos();
	static void FontStringToSize(lg_dword* pHeight, lg_dword* pWidth, lg_pstr szString);
};

#endif __LV_CON_H__