#include "stdafx.h"
#include "CommonCtrls.h"
#include "common/CommonRes.h"

GMap<float,uint32> CtrlCV;

// ---- CCtrlWnd ----------------------------------------------------------------------------------

LRESULT CCtrlWnd::SendCtrlNotifyMsg( DWORD Notify, UINT message, WPARAM wParam, LPARAM lParam )
{
	NotifyMessage NotifyMsg;
	NotifyMsg.hrd.hwndFrom = m_hWnd;
	NotifyMsg.hrd.idFrom = GetDlgCtrlID();
	NotifyMsg.hrd.code = Notify;
	NotifyMsg.pWnd = this;
	NotifyMsg.Message = message;
	NotifyMsg.wParam = wParam;
	NotifyMsg.lParam = lParam;
	return GetParent()->SendMessage( WM_NOTIFY, NotifyMsg.hrd.idFrom, (LPARAM) (LPNMHDR)&NotifyMsg ); 
}

LRESULT CCtrlWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{		
	//将鼠标信息传给父窗口
	if( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST || message == WM_SETCURSOR )
	{
		LRESULT Re = SendCtrlNotifyMsg( NM_CHILDETOPARENT, message, wParam, lParam ); 
		if( !m_IsNeedMsg )
			return Re;
	}
	if( WM_LBUTTONDOWN < message && WM_MBUTTONDBLCLK > message )
		SetFocus();

	return CStatic::WindowProc( message, wParam, lParam );
}

// ---- CRadios ----------------------------------------------------------------------------------

void CRadios::CheckRadio( CWnd* pWnd, int Check )
{
	for( int i = 0; i< (int)m_Container.size(); i++ )
	{
		if( Check >= m_Container[i].m_FirstRadio && Check <= m_Container[i].m_LastRadio )
			m_Container[i].m_CheckeRadio = Check;
		pWnd->CheckRadioButton( m_Container[i].m_FirstRadio, m_Container[i].m_LastRadio, m_Container[i].m_CheckeRadio );
	}
}
void CRadios::CheckRadio( CWnd* pWnd, int First, int Last, int Check )
{
	m_Container.push_back( CRadioGroup( First, Last, Check ) );
	pWnd->CheckRadioButton( First, Last, Check );
}

// ---- CColorBlock -------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CColorBlock, CWnd)
BEGIN_MESSAGE_MAP(CColorBlock, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CColorBlock::OnPaint()
{
	CPaintDC dc(this); 

	// Do not call CCtrlWnd::OnPaint() for painting messages
	OnDraw( &dc );
}

void CColorBlock::PreSubclassWindow()
{
	m_Color = GMap<float,CColor>();
	SetColorList( m_Color );
	CCtrlWnd::PreSubclassWindow();
}

void CColorBlock::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCurPos( point );
	CCtrlWnd::OnLButtonDown(nFlags, point);
}

void CColorBlock::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CRect Rect;
	GetClientRect( &Rect );

	float Percent = ( point.x - 3 )/( (float)Rect.Width() - 6 );
	CColorDialog Dlg( GetGDIColor( Percent ), CC_FULLOPEN );
	if( Dlg.DoModal() == IDCANCEL )
		return;
	DWORD Color = Dlg.GetColor();
	TCHAR* CBuf  = (TCHAR*)&Color;
	TCHAR temp = CBuf[0];
	CBuf[0] = CBuf[2];
	CBuf[2] = temp;
	m_Color[ m_CurColorPercent ] = ( Color&0xffffff )|GetAlpha( Percent );
	ResetPic();

	Invalidate();

	CCtrlWnd::OnLButtonDblClk(nFlags, point);
}

void CColorBlock::OnMouseMove(UINT nFlags, CPoint point)
{
	if( nFlags&MK_LBUTTON )
	{
		CRect Rect;
		GetClientRect( &Rect );

		DWORD Color = GetColor( m_CurColorPercent );
		if( m_CurColorPercent > 0.01f && m_CurColorPercent < 0.99f && !( GetKeyState( VK_CONTROL )&0x8000 ) )
		{
			m_Color.erase( m_CurColorPercent );
			m_CurColorPercent = ( point.x - 3 )/( (float)Rect.Width() - 6 );
		}
		if( m_CurColorPercent <= 0.01f )
			m_CurColorPercent = 0.0f;
		if( m_CurColorPercent >= 0.99f )
			m_CurColorPercent = 1.0f;

		int Alpha = 255 - (int)( ( point.y - 3.0f )*255/( Rect.Height() - 6 ) );
		Alpha = max( Alpha, 0 );
		Alpha = min( Alpha, 255 );
		m_Color[m_CurColorPercent] = ( Color&0xffffff )|( Alpha<<24 );
		ResetPic();

		CDC* pDC = GetDC();
		OnDraw( pDC );
		ReleaseDC( pDC );
	}

	CCtrlWnd::OnMouseMove(nFlags, point);
}

void CColorBlock::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if( GetKeyState(VK_CONTROL)&0x8000 )
	{
		if( nChar == 'C' )
		{
			GMap<float,CColor> tMap;
			GetColorList( tMap );
			GMap<float,CColor>::iterator it,eit = tMap.end();
			CtrlCV.clear();
			for( it = tMap.begin(); it!=eit; ++it )
			{
				CtrlCV.insert( GMap<float,DWORD>::value_type(it->first,it->second) );
			}
		}
		else if( nChar == 'V' )
		{
			m_Color.clear();
			GMap<float,uint32>::iterator it,eit = CtrlCV.end();
			for( it = CtrlCV.begin(); it!=eit; ++it )
			{
				m_Color.insert(  GMap<float,CColor>::value_type(it->first,it->second) );
			}
			SetColorList( m_Color );
		}
	}

	if( nChar == VK_DELETE )
	{		
		if( m_CurColorPercent > 0.01f && m_CurColorPercent < 0.99f )
			m_Color.erase( m_CurColorPercent );
		ResetPic();

		CDC* pDC = GetDC();
		OnDraw( pDC );
		ReleaseDC( pDC );
	}

	CCtrlWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CColorBlock::OnDraw( CDC* pDC )
{
	CRect Rect;
	GetClientRect( &Rect );

	CDC MemDC;
	MemDC.CreateCompatibleDC( pDC );
	MemDC.SelectObject( &m_Bitmap );
	pDC->BitBlt( 0, 0, Rect.Width(), Rect.Height(), &MemDC, 0, 0, SRCCOPY );

	DWORD Color;
	CGdiObject* pOldObj = pDC->SelectObject( &m_Font );
	pDC->SetBkMode( TRANSPARENT );

	for( GMap<float,CColor>::iterator it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		DWORD Alpha = uint32(it->second)>>24;
		int PosX = (int)( (*it).first*( Rect.Width() - 6 ) + 3 );
		int PosY = (int)( ( Rect.Height() - 6 )*( 255.0f - Alpha )/255 + 3 );

		if( it == m_Color.begin() )
			pDC->MoveTo( PosX, PosY );
		else
		{
			CPen Pen( PS_SOLID, 1, Color );
			CGdiObject* OldObj = pDC->SelectObject( &Pen );
			pDC->LineTo( PosX, PosY );
			pDC->SelectObject( OldObj );
		}

		Color = GetGDIColor( (*it).first );
		Color =  ~Color;
		Color &= 0xffffff;
		Color |= 0xcc000000;
		if( (*it).first == m_CurColorPercent )
			pDC->FillSolidRect( PosX-3, PosY-3, 6, 6, Color );
		else
			pDC->Draw3dRect( PosX-3, PosY-3, 6, 6, Color, Color );
		CRect rt = Rect;
		rt.left = PosX;
		rt.top  = PosY;
		if( rt.left > Rect.right - 20 )
			rt.left -= 20;
		if( rt.top > Rect.bottom - 14 )
			rt.top -= 14;
		TCHAR Buf[256];
		sprintf( Buf, "%d", Alpha );
		pDC->SetTextColor( Color );
		pDC->DrawText( Buf, &rt, DT_LEFT );
	}
	pDC->SelectObject( pOldObj );
}

void  CColorBlock::SetColorList( GMap<float,float>& Color )
{
	Update();
	m_EditColor = &Color;
	m_eType = CT_FLOAT;

	GMap<float,CColor> tColorMap;
	GMap<float,float>::iterator it,eit = Color.end();
	for( it = Color.begin(); it!=eit; ++it )
		tColorMap[it->first] = ((uint32)( log10( it->second )*64.0f ) )<<24;
	UpdateColorList(tColorMap);
}

void  CColorBlock::GetColorList( GMap<float,float>& Color )
{
	GMap<float,CColor> tColorMap;
	GetColorList(tColorMap);
	Color.clear();
	GMap<float,CColor>::iterator it,eit = tColorMap.end();
	for( it = tColorMap.begin(); it!=eit; ++it )
		Color[it->first] = pow( 10.0f, ( it->second>>24 )/64.0f );;
}

void  CColorBlock::SetColorList( GMap<float,uint32>& Color )
{
	Update();
	m_EditColor = &Color;
	m_eType = CT_UINT32;

	GMap<float,CColor> tColorMap;
	GMap<float,uint32>::iterator it,eit = Color.end();
	for( it = Color.begin(); it!=eit; ++it )
		tColorMap[it->first] = it->second<<24;
	UpdateColorList(tColorMap);
}

void  CColorBlock::GetColorList( GMap<float,uint32>& Color )
{
	GMap<float,CColor> tColorMap;
	GetColorList(tColorMap);
	Color.clear();
	GMap<float,CColor>::iterator it,eit = tColorMap.end();
	for( it = tColorMap.begin(); it!=eit; ++it )
		Color[it->first] = it->second.dwColor>>24;
}

void  CColorBlock::SetColorList( GMap<float,CColor4>& Color )
{
	Update();
	m_EditColor = &Color;
	m_eType = CT_COLOR4;

	GMap<float,CColor> tColorMap;
	GMap<float,CColor4>::iterator it,eit = Color.end();
	for( it = Color.begin(); it!=eit; ++it )
		tColorMap[it->first] = it->second.MakeDWordSafe();
	UpdateColorList(tColorMap);
}

void CColorBlock::ClearEditColor()
{	
	m_EditColor = NULL;
}

void  CColorBlock::GetColorList( GMap<float,CColor4>& Color )
{
	GMap<float,CColor> tColorMap;
	GetColorList(tColorMap);
	Color.clear();
	GMap<float,CColor>::iterator it,eit = tColorMap.end();
	for( it = tColorMap.begin(); it!=eit; ++it )
		Color[it->first] = it->second;
}

void  CColorBlock::SetColorList( GMap<float,CColor>& Color )
{
	if( &Color != &m_Color )
	{
		Update();
		m_EditColor = &Color;
		m_eType = CT_COLOR;
	}

	UpdateColorList(Color);
};

void  CColorBlock::UpdateColorList( GMap<float,CColor>& Color)
{
	m_Color = Color;
	if( m_Color.size() < 2 )
	{
		m_Color.clear();
		m_Color[0.0f] = 0;
		m_Color[1.0f] = 0xffffffff;
	}

	if( m_Color.find( 1.0 ) == m_Color.end() )
	{
		m_Color[1.0f] = 0xffffffff;
	}

	CRect Rect;
	GetClientRect( &Rect );

	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= Rect.Width();
	bmi.bmiHeader.biHeight		= Rect.Height();
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	if( !m_MemDC.GetSafeHdc() )
		m_MemDC.CreateCompatibleDC( NULL );
	HBITMAP hBmp = CreateDIBSection( m_MemDC.m_hDC, &bmi, 
		DIB_RGB_COLORS, (void **)&m_pBit, NULL, 0 );

	m_Bitmap.DeleteObject();
	m_Bitmap.Attach( hBmp );
	ResetPic();
	CDC* pDC = GetDC();
	OnDraw( pDC );
	ReleaseDC( pDC );
}

void  CColorBlock::Update()
{
	if( m_EditColor )
	{
		switch(m_eType)
		{
		case CT_FLOAT:
			GetColorList( *(GMap<float,float>*)m_EditColor ); 
			break;
		case CT_UINT32:
			GetColorList( *(GMap<float,uint32>*)m_EditColor ); 
			break;
		case CT_COLOR:
			GetColorList( *(GMap<float,CColor>*)m_EditColor ); 
			break;
		case CT_COLOR4:
			GetColorList( *(GMap<float,CColor4>*)m_EditColor ); 
			break;
		}
	}
}

void  CColorBlock::GetColorList( GMap<float,CColor>& Color )
{
	Color = m_Color;
};

//填写颜色缓冲区
void CColorBlock::ResetPic()
{
	if( m_Color.size() >= 2 )
	{
		CRect Rect;
		GetClientRect( &Rect );
		GMap<float,CColor>::iterator itend = m_Color.end();
		itend--;

		for( int j = 0; j < 3; j++ )
		{
			for( int i = 0; i< Rect.Height(); i++ )
			{
				BYTE* Pix;
				BYTE* SC;

				Pix = (BYTE*)&m_pBit[ i*Rect.Width() + j ];
				SC  = (BYTE*)&( m_Color.begin()->second );
				for( int k = 0; k < 4; k ++ )
					Pix[k] = SC[k]; 

				Pix = (BYTE*)&m_pBit[ i*Rect.Width() + Rect.Width() - j - 1 ];
				SC  = (BYTE*)&( itend->second );
				for( int k = 0; k < 4; k ++ )
					Pix[k] = SC[k]; 
			}
		}

		for( GMap<float,CColor>::iterator it = m_Color.begin(); it != itend; ++it )
		{
			GMap<float,CColor>::iterator Next = it;
			Next++;
			int Start = (int)( (*it).first*Rect.Width() );
			int End   = (int)( (*Next).first*Rect.Width() );
			for( int j = Start; j < End; j++ )
			{
				float W = ( (float)End - j )/( End - Start );
				for( int i = 0; i< Rect.Height(); i++ )
				{
					BYTE* Pix = (BYTE*)&m_pBit[ i*Rect.Width() + j ];
					for( int k = 0; k < 4; k ++ )
						Pix[k] = (BYTE)( min( (*it).second.byColor[k]*W + (*Next).second.byColor[k]*( 1 - W ),255) );
				}
			}
		}
	}
}

//得到指定百分比的颜色
DWORD CColorBlock::GetGDIColor( float Percent )
{
	GMap<float,CColor>::iterator it = m_Color.end();
	it--;
	DWORD Color = GetColor( Percent );
	TCHAR* CBuf  = (TCHAR*)&Color;
	TCHAR temp = CBuf[0];
	CBuf[0] = CBuf[2];
	CBuf[2] = temp;

	return Color;
}

CColor CColorBlock::GetColor( float Percent )
{
	GMap<float,CColor>::iterator it = m_Color.end();
	it--;
	CColor Color = (*it).second;
	for( it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( Percent < it->first )
		{
			GMap<float,CColor>::iterator pre = it;
			if(pre!=m_Color.begin())
				pre--;
			//BYTE* SC  = (BYTE*)&((*pre).second);
			//BYTE* EC  = (BYTE*)&((*it).second);
			float W = ( it->first - Percent )/( it->first - pre->first );

			//BYTE* Pix = (BYTE*)&Color;
			for( int i = 0; i<4; i++ )
				Color.byColor[i] = min( (BYTE)(pre->second.byColor[i]*W +  it->second.byColor[i]*( 1 - W )),255 ); 
			break;
		}
	}
	return Color;
}

DWORD CColorBlock::GetAlpha( float Percent )
{
	GMap<float,CColor>::iterator it = m_Color.end();
	it--;
	DWORD Alpha = (*it).second;
	for( it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( Percent < it->first )
		{
			GMap<float,CColor>::iterator pre = it;
			if(it!=m_Color.begin())
				pre--;
			float W = ( it->first - Percent )/( it->first - pre->first );

			Alpha = (DWORD)( ( (*pre).second&0xff000000 )*W + ( (*it).second&0xff000000 )*( 1 - W ) ); 
			break;
		}
	}

	return Alpha&0xff000000;
}

void CColorBlock::SetCurPos( CPoint& point )
{
	CRect Rect;
	GetClientRect( &Rect );

	m_ColorValid = TRUE;
	m_CurColorPercent = ( point.x - 3 )/( (float)Rect.Width() - 6 );
	if( m_CurColorPercent < 0.01f )
		m_CurColorPercent = 0.0f;
	if( m_CurColorPercent > 0.99f )
		m_CurColorPercent = 1.0f;
	for( GMap<float,CColor>::iterator it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( abs( it->first - m_CurColorPercent ) < 3.1f/( (float)Rect.Width() - 6 ) )
		{
			m_CurColorPercent = it->first;
			break;
		}
	}

	Invalidate();
}

// ---- CBlockCombo -------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CBlockCombo, CComboBox)
BEGIN_MESSAGE_MAP(CBlockCombo, CComboBox)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnCbnSelendok)
END_MESSAGE_MAP()

void CBlockCombo::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	SetItemHeight( -1, 14 );

	CComboBox::PreSubclassWindow();
}

void CBlockCombo::OnCbnSelendok()
{
	// TODO: Add your control notification handler code here
	if( m_BlockEdit )
	{
		int Index = GetCurSel();
		if( Index >= 0 )
			m_BlockEdit->SetCurState( (DWORD)GetItemData( Index ) );
	}
}

void CBlockCombo::SetCurSelByDataValue( DWORD Data )
{
	for( int i = 0; i < GetCount(); i++ )
	{
		if( GetItemData( i ) == Data )
		{
			SetCurSel( i );
			if( m_BlockEdit )
				m_BlockEdit->SetCurState( Data );
			return;
		}
	}
	SetCurSel(0);
	if( m_BlockEdit )
	{
		m_BlockEdit->SetCurState( (DWORD)GetItemData( 0 ) );
	}
}

// ---- CCompBlock --------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CCompBlock, CWnd)
BEGIN_MESSAGE_MAP(CCompBlock, CWnd)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CCompBlock::SetRange( UINT Range )
{ 
	m_Range = Range; 
	Invalidate(); 
}

UINT CCompBlock::GetRange()
{ 
	return m_Range; 
}

void CCompBlock::SetCurState( DWORD State )
{ 
	m_State[m_CurPercent] = State; 
}

void CCompBlock::Update()
{ 
	if( m_EditState )
		*m_EditState = m_State; 
};

// CCompBlock message handlers

void CCompBlock::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	m_Font.CreateFont( 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

	m_State = GMap<float,uint32>();
	SetStateList( m_State, NULL );
	SetRange( 100 );
	m_BlockCursor = AfxGetApp()->LoadCursor( IDC_BLOCKMOVE );

	CCtrlWnd::PreSubclassWindow();
}

void CCompBlock::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CCtrlWnd::OnPaint() for painting messages
	OnDraw( &dc );
}

void CCompBlock::OnDraw( CDC* pDC )
{
	//	CCtrlWnd::OnDraw( pDC );

	CRect ClienRect;
	for( GMap<float,uint32>::iterator it = m_State.begin(); it != m_State.end(); ++it )
	{
		GMap<float,uint32>::iterator next = it;
		next++;
		GetClientRect( &ClienRect );
		int Width = ClienRect.Width();
		ClienRect.left = (int)( (*it).first*Width );
		if( next != m_State.end() )
			ClienRect.right = (int)( (*next).first*Width );
		pDC->FillSolidRect( &ClienRect, m_CurPercent == (*it).first ? 0xcc808080 : 0xcccccccc );
		pDC->Draw3dRect( &ClienRect, 0xccffffff, 0xcc222222 );
		ClienRect.bottom--;
		ClienRect.right--;
		pDC->Draw3dRect( &ClienRect, 0xccffffff, 0xcc505050 );

		CString str;
		str.Format( "%.1f", (*it).first*m_Range );
		ClienRect.left += 3;
		ClienRect.top += 1;
		pDC->SelectObject( &m_Font );
		pDC->SetBkMode( TRANSPARENT );
		pDC->DrawText( str, &ClienRect, DT_LEFT );
	}
}

void  CCompBlock::SetStateList( GMap<float,uint32>& State, CBlockCombo* StateOption )
{
	if( &State != &m_State )
	{
		Update();
		if( m_BlockOption )
			m_BlockOption->SetCompBlock( NULL );
		m_BlockOption = StateOption;
		m_EditState = &State;
		if( m_BlockOption )
			m_BlockOption->SetCompBlock( this );
	}

	m_State = State;
	if( m_State.size() < 1 )
	{
		m_State.clear();
		m_State[0.0f] = 0;
	}

	if( m_BlockOption )
	{
		if( m_State.find( m_CurPercent ) == m_State.end() )
			m_CurPercent = 0.0f;
		m_BlockOption->SetCurSelByDataValue( m_State[m_CurPercent] );
	}

	CDC* pDC = GetDC();
	OnDraw( pDC );
	ReleaseDC( pDC );
}

void CCompBlock::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect Rect;
	GetClientRect( &Rect );

	m_CurPercent = point.x/(float)Rect.Width();

	m_PreMoveBlock = -1;
	for( GMap<float,uint32>::iterator it = m_State.begin(); it != m_State.end(); ++it )
	{
		if( abs( m_CurPercent - it->first )*Rect.Width() < 4.0f )
		{
			if( it->first > 0.01f && it->first < 0.99f )
				m_PreMoveBlock = it->first;
			::SetCursor( m_BlockCursor );
			m_SetBlockCur = TRUE;
			break;
		}
	}

	if( m_CurPercent < 0.01f )
		m_CurPercent = 0.0f;
	if( m_CurPercent > 0.99f )
		m_CurPercent = 1.0f;
	GMap<float,uint32>::iterator it = m_State.begin();
	for( ; it != m_State.end(); ++it )
		if( m_CurPercent < it->first )
			break;
	it--;
	m_CurPercent = it->first;
	if( m_BlockOption )
		m_BlockOption->SetCurSelByDataValue( it->second );

	CDC* pDC = GetDC();
	OnDraw( pDC );
	ReleaseDC( pDC );

	CCtrlWnd::OnLButtonDown(nFlags, point);
}

void CCompBlock::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect Rect;
	GetClientRect( &Rect );

	m_CurPercent = point.x/(float)Rect.Width();
	m_State[m_CurPercent] = 0;
	CDC* pDC = GetDC();
	OnDraw( pDC );
	ReleaseDC( pDC );

	CCtrlWnd::OnLButtonDblClk(nFlags, point);
}

void CCompBlock::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect Rect;
	GetClientRect( &Rect );

	float CurPercent = point.x/(float)Rect.Width();
	if( CurPercent < 0.01f )
		CurPercent = 0.0f;
	if( CurPercent > 0.99f )
		CurPercent = 1.0f;

	m_SetBlockCur = FALSE;

	if( m_PreMoveBlock > 0.01f && nFlags&MK_LBUTTON )
	{
		DWORD State = m_State[m_PreMoveBlock];
		m_State.erase( m_PreMoveBlock );
		m_State[CurPercent] = State;
		m_PreMoveBlock = CurPercent;
		m_CurPercent = m_PreMoveBlock;
		if( m_BlockOption )
			m_BlockOption->SetCurSelByDataValue( m_State[m_CurPercent] );

		CDC* pDC = GetDC();
		OnDraw( pDC );
		ReleaseDC( pDC );
		m_SetBlockCur = TRUE;
	}
	else
	{
		m_PreMoveBlock = -1;
		for( GMap<float,uint32>::iterator it = m_State.begin(); it != m_State.end(); ++it )
		{
			if( abs( CurPercent - it->first )*Rect.Width() < 4.0f )
			{
				if( nFlags&MK_LBUTTON )
				{
					if( it->first > 0.01f && it->first < 0.99f )
					{
						uint32 State = it->second;
						m_State.erase( it );
						m_State[CurPercent] = State;
						m_PreMoveBlock = CurPercent;
						m_CurPercent = m_PreMoveBlock;

						CDC* pDC = GetDC();
						OnDraw( pDC );
						ReleaseDC( pDC );
					}
				}
				m_SetBlockCur = TRUE;
				break;
			}
		}
	}

	CCtrlWnd::OnMouseMove(nFlags, point);
}

BOOL CCompBlock::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if( m_SetBlockCur )
	{
		::SetCursor( m_BlockCursor );
		return 1;
	}
	return CCtrlWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CCompBlock::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	if( GetKeyState(VK_CONTROL)&0x8000 )
	{
		if( nChar == 'C' )
			CtrlCV = m_State;
		else if( nChar == 'V' )
		{
			m_State = CtrlCV;
			SetStateList( m_State, NULL );
		}
	}

	if( nChar == VK_DELETE )
	{
		if( m_CurPercent > 0.01f && m_CurPercent < 0.99f )
			m_State.erase( m_CurPercent );

		GMap<float,uint32>::iterator it = m_State.begin();
		for( ; it != m_State.end(); ++it )
			if( m_CurPercent < it->first )
				break;
		it--;
		m_CurPercent = it->first;

		CDC* pDC = GetDC();
		OnDraw( pDC );
		ReleaseDC( pDC );
	}

	CCtrlWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

// ---- CMsgWnd -----------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CMsgWnd, CWnd)
BEGIN_MESSAGE_MAP(CMsgWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CMsgWnd::OnPaint()
{
	CPaintDC dc(this); 
	// Do not call CWnd::OnPaint() for painting messages
	CFont Font;
	Font.CreateFont( 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "宋体" );
	dc.SelectObject( &Font );
	CString str;
	CRect rt;
	GetWindowText( str );
	GetClientRect( &rt );
	dc.FillSolidRect( rt, 0xccccffff );
	dc.DrawText( str, &rt, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
	dc.Draw3dRect( rt, 0, 0 );
}

// ---- CModelFileDialog --------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CModelFileDialog, CFileDialog)
CModelFileDialog::CModelFileDialog( BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
									DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) 
									: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}

BEGIN_MESSAGE_MAP(CModelFileDialog, CFileDialog)
END_MESSAGE_MAP()

INT_PTR CModelFileDialog::DoModal( CString& RootPath )
{
	INT_PTR re = CFileDialog::DoModal();
	if( re == IDOK )
		RootPath = m_FloderName;

	return re;
}

void CModelFileDialog::OnFolderChange()
{
	m_FloderName = GetFolderPath();

	CFileDialog::OnFolderChange();
}

// ---- CAutoDlg ----------------------------------------------------------------------------------

void CAutoDlg::FindFile( TCHAR* Path, CAutoDlg* pDlg )
{
	WIN32_FIND_DATA fd;
	TCHAR Buf[1024];
	sprintf( Buf, "%s\\*", Path );
	HANDLE h = FindFirstFile( Buf, &fd );

	if( h != INVALID_HANDLE_VALUE )
	{
		int re=1;
		while(re)
		{
			_strlwr( fd.cFileName );
			if( strcmp( "..", fd.cFileName ) && strcmp( ".", fd.cFileName ) )
			{
				TCHAR Buf[1024];
				sprintf( Buf, "%s\\%s", Path, fd.cFileName );
				if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					FindFile( Buf, pDlg );
				else
					pDlg->AddFile( Buf );
			}
			re = FindNextFile( h, &fd );
		}
		FindClose(h);
	}
}

// ---- CColorSimpleBlock -------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CColorSimpleBlock, CWnd)
BEGIN_MESSAGE_MAP(CColorSimpleBlock, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
void CColorSimpleBlock::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CCtrlWnd::OnPaint() for painting messages
	OnDraw( &dc );
}
void CColorSimpleBlock::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	m_Color = map<float,CColor4>();
	SetColorList( m_Color );
	CCtrlWnd::PreSubclassWindow();
}

void CColorSimpleBlock::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect Rect;
	GetClientRect( &Rect );

	float Percent = ( point.x - 3 )/( (float)Rect.Width() - 6 );
	CColorDialog Dlg( GetGDIColor( Percent ).MakeDWord(), CC_FULLOPEN );
	if( Dlg.DoModal() == IDCANCEL )
		return;
	DWORD Color = Dlg.GetColor();
	TCHAR* CBuf  = (TCHAR*)&Color;
	TCHAR temp = CBuf[0];
	CBuf[0] = CBuf[2];
	CBuf[2] = temp;
	m_Color[ m_CurColorPercent ] = ( Color&0xffffff )|GetAlpha( Percent );
	ResetPic();

	Invalidate();

	CCtrlWnd::OnLButtonDblClk(nFlags, point);
}

void CColorSimpleBlock::OnDraw( CDC* pDC )
{
	//	CCtrlWnd::OnDraw( pDC );

	CRect Rect;
	GetClientRect( &Rect );

	CDC MemDC;
	MemDC.CreateCompatibleDC( pDC );
	MemDC.SelectObject( &m_Bitmap );
	pDC->BitBlt( 0, 0, Rect.Width(), Rect.Height(), &MemDC, 0, 0, SRCCOPY );

	DWORD Color;
	//	CGdiObject* pOldObj = pDC->SelectObject( &m_Font );
	pDC->SetBkMode( TRANSPARENT );

	for( map<float,CColor4>::iterator it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		DWORD Alpha = (DWORD)(it->second.a * 0xFF);
		int PosX = (int)( (*it).first*( Rect.Width() - 6 ) + 3 );
		int PosY = (int)( ( Rect.Height() - 6 )*( 255.0f - Alpha )/255 + 3 );

		//if( it == m_Color.begin() )
		//	pDC->MoveTo( PosX, PosY );
		//else
		//{
		//	CPen Pen( PS_SOLID, 1, Color );
		//	CGdiObject* OldObj = pDC->SelectObject( &Pen );
		//	pDC->LineTo( PosX, PosY );
		//	pDC->SelectObject( OldObj );
		//}

		Color = GetGDIColor( (*it).first ).MakeDWord();
		Color =  ~Color;
		Color &= 0xffffff;
		Color |= 0xcc000000;
		//		if( (*it).first == m_CurColorPercent )
		//			pDC->FillSolidRect( PosX-3, PosY-3, 6, 6, Color );
		//		else
		//			pDC->Draw3dRect( PosX-3, PosY-3, 6, 6, Color, Color );
		//CRect rt = Rect;
		//rt.left = PosX;
		//rt.top  = PosY;
		//if( rt.left > Rect.right - 20 )
		//	rt.left -= 20;
		//if( rt.top > Rect.bottom - 14 )
		//	rt.top -= 14;
		//TCHAR Buf[256];
		//sprintf( Buf, "%d", Alpha );
		//pDC->SetTextColor( Color );
		//pDC->DrawText( Buf, &rt, DT_LEFT );
	}
	//	pDC->SelectObject( pOldObj );
}

void  CColorSimpleBlock::SetColorList( map<float,CColor4>& Color )
{
	if( &Color != &m_Color )
	{
		Update();
		m_EditColor = &Color;
	}

	m_Color = Color;
	if( m_Color.size() < 2 )
	{
		m_Color.clear();
		m_Color[0.0f] = 0;
		m_Color[1.0f] = 0xffffffff;
	}

	if( m_Color.find( 1.0 ) == m_Color.end() )
	{
		m_Color[1.0f] = 0xffffffff;
	}

	CRect Rect;
	GetClientRect( &Rect );

	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= Rect.Width();
	bmi.bmiHeader.biHeight		= Rect.Height();
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	if( !m_MemDC.GetSafeHdc() )
		m_MemDC.CreateCompatibleDC( NULL );
	HBITMAP hBmp = CreateDIBSection( m_MemDC.m_hDC, &bmi, 
		DIB_RGB_COLORS, (void **)&m_pBit, NULL, 0 );

	m_Bitmap.DeleteObject();
	m_Bitmap.Attach( hBmp );
	ResetPic();
	CDC* pDC = GetDC();
	OnDraw( pDC );
	ReleaseDC( pDC );
};

void  CColorSimpleBlock::GetColorList( map<float,CColor4>& Color )
{
	Color = m_Color;
};

//填写颜色缓冲区
void CColorSimpleBlock::ResetPic()
{
	if( m_Color.size() >= 2 )
	{
		CRect Rect;
		GetClientRect( &Rect );
		map<float,CColor4>::iterator itend = m_Color.end();
		itend--;

		for( int j = 0; j < 3; j++ )
		{
			for( int i = 0; i< Rect.Height(); i++ )
			{
				BYTE* Pix;
				BYTE* SC;

				Pix = (BYTE*)&m_pBit[ i*Rect.Width() + j ];
				SC  = (BYTE*)&( m_Color.begin()->second );
				for( int k = 0; k < 4; k ++ )
					Pix[k] = SC[k]; 

				Pix = (BYTE*)&m_pBit[ i*Rect.Width() + Rect.Width() - j - 1 ];
				SC  = (BYTE*)&( itend->second );
				for( int k = 0; k < 4; k ++ )
					Pix[k] = SC[k]; 
			}
		}

		for( map<float,CColor4>::iterator it = m_Color.begin(); it != itend; ++it )
		{
			map<float,CColor4>::iterator Next = it;
			Next++;
			int Start = (int)( (*it).first*Rect.Width() );
			int End   = (int)( (*Next).first*Rect.Width() );
			BYTE* SC  = (BYTE*)&((*it).second);
			BYTE* EC  = (BYTE*)&((*Next).second);
			for( int j = Start; j < End; j++ )
			{
				float W = ( (float)End - j )/( End - Start );
				for( int i = 0; i< Rect.Height(); i++ )
				{
					BYTE* Pix = (BYTE*)&m_pBit[ i*Rect.Width() + j ];
					for( int k = 0; k < 4; k ++ )
						Pix[k] = (BYTE)( min( SC[k]*W + EC[k]*( 1 - W ), 255 ) ); 
				}
			}
		}
	}
}

//得到指定百分比的颜色
CColor4 CColorSimpleBlock::GetGDIColor( float Percent )
{
	map<float,CColor4>::iterator it = m_Color.end();
	it--;
	CColor4 Color = GetColor( Percent );

	float temp = Color.c[0];
	Color.c[0] = Color.c[2];
	Color.c[2] = temp;

	return Color;
}

CColor4 CColorSimpleBlock::GetColor( float Percent )
{
	map<float,CColor4>::iterator it = m_Color.end();
	it--;
	CColor4 Color = (*it).second;
	for( it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( Percent < it->first )
		{
			map<float,CColor4>::iterator pre = it;
			pre--;
			float W = ( it->first - Percent )/( it->first - pre->first );
			for( int i = 0; i<4; i++ )
				Color.c[i] = (BYTE)( min( (*pre).second.c[i]*W + (*it).second.c[i]*( 1 - W ), 255 ) ); 
			break;
		}
	}

	return Color;
}

DWORD CColorSimpleBlock::GetAlpha( float Percent )
{
	map<float,CColor4>::iterator it = m_Color.end();
	it--;
	DWORD Alpha = (*it).second.MakeDWordSafe();
	for( it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( Percent < it->first )
		{
			map<float,CColor4>::iterator pre = it;
			pre--;
			float W = ( it->first - Percent )/( it->first - pre->first );

			Alpha = (DWORD)( ( (*pre).second.MakeDWordSafe()&0xff000000 )*W + ( (*it).second.MakeDWordSafe()&0xff000000 )*( 1 - W ) ); 
			break;
		}
	}

	return Alpha&0xff000000;
}

void CColorSimpleBlock::SetCurPos( CPoint& point )
{
	CRect Rect;
	GetClientRect( &Rect );

	m_ColorValid = TRUE;
	m_CurColorPercent = ( point.x - 3 )/( (float)Rect.Width() - 6 );
	if( m_CurColorPercent < 0.01f )
		m_CurColorPercent = 0.0f;
	if( m_CurColorPercent > 0.99f )
		m_CurColorPercent = 1.0f;
	for( map<float,CColor4>::iterator it = m_Color.begin(); it != m_Color.end(); ++it )
	{
		if( abs( it->first - m_CurColorPercent ) < 3.1f/( (float)Rect.Width() - 6 ) )
		{
			m_CurColorPercent = it->first;
			break;
		}
	}

	Invalidate();
}
