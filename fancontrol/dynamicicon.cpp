#include "_prec.h"
#include "DynamicIcon.h"
#include <string.h>


CDynamicIcon::CDynamicIcon(const char *line1,const char *line2, const int iFarbeIconA, const int iFontIconA) {
    //3 chars per line
    char _line1[4],_line2[4];
    strncpy_s(_line1,sizeof(_line1),line1,3);
    strncpy_s(_line2,sizeof(_line2),line2,3);
    _line1[3]=0;
    _line2[3]=0;
	

  //TODO: implement errorhandling

  HDC hDC = GetDC (0);

  memDC1_  = CreateCompatibleDC     (hDC);
  memDC2_  = CreateCompatibleDC     (hDC);
  iconBmp_ = CreateCompatibleBitmap (hDC, iconWidth_, iconHeight_);

  iconMaskBmp_ = CreateCompatibleBitmap (hDC, iconWidth_, iconHeight_);
  oldBmp_1    = (HBITMAP) SelectObject (memDC1_, (HBITMAP) iconBmp_);
  oldBmp_2    = (HBITMAP) SelectObject (memDC2_, (HBITMAP) iconMaskBmp_);

  // prepare mask
  HBRUSH hBrush = CreateSolidBrush (RGB (255, 255, 255)); 
  hOldBrush     = (HBRUSH) SelectObject (memDC2_, hBrush);
  PatBlt (memDC2_, 0, 0, iconWidth_, iconHeight_, PATCOPY);
  SelectObject (memDC2_, hOldBrush);
  DeleteObject (hBrush);
  
  // draw circle on both bitmaps
  rgn = CreateRectRgn (0, 0, iconWidth_, iconHeight_);

  switch (iFarbeIconA) {
  case 10:
  hBrush = CreateSolidBrush (RGB (245,245,245)); //grau
  break;
  case 11:
  hBrush = CreateSolidBrush (RGB (191,239,255)); //blau
  break;
  case 12: 
  hBrush = CreateSolidBrush (RGB (255,255,0)); //gelb
  break;
  case 13:
  hBrush = CreateSolidBrush (RGB (255,165,0)); //orange
  break;
  case 14:
  hBrush = CreateSolidBrush (RGB (255,69,0)); //rot
  break;
  case 21:
  hBrush = CreateSolidBrush (RGB (175,255,175)); //sehr hell grün
  break;
  case 22:
  hBrush = CreateSolidBrush (RGB (123,255,123)); //hell grün
  break;
  case 23:
  hBrush = CreateSolidBrush (RGB (0,255,0)); //grün
  break;
  case 24:
  hBrush = CreateSolidBrush (RGB (0,218,0)); //dunkel grün
  break;
  case 25:
  hBrush = CreateSolidBrush (RGB (0,164,0)); //sehr dunkel grün
  break;
  default:
  hBrush = CreateSolidBrush (RGB (255,255,255)); // weiss
  };


  FillRgn (memDC1_, rgn, hBrush);
  DeleteObject (hBrush);

  hBrush = CreateSolidBrush (RGB (0, 0, 0)); 
  FillRgn (memDC2_, rgn, hBrush);


    HFONT hfnt, hOldFont; 
 
    hfnt = this->CreateFont(memDC1_);

    if (hOldFont = (HFONT)SelectObject(memDC1_, hfnt)) 
    { 
        //SetBkColor(memDC1_,RGB(255,0,0));
        SetBkMode(memDC1_,TRANSPARENT);
        //TODO: Textcolors
        //default textcolor: RGB(32,16,176)

        RECT r;
        r.top = -2; //org -1
        r.left = 0;
        r.bottom = 9; //org 10
        r.right = 15;
        DrawTextEx(memDC1_,(LPSTR)_line1,strlen(_line1),&r,DT_CENTER,NULL); 

        r.top = 5;  //org 6 neu ?
        r.left = 0;
        r.bottom = 16; // org 15
        r.right = 15;
        DrawTextEx(memDC1_,(LPSTR)_line2,strlen(_line2),&r,DT_CENTER,NULL); 

        SelectObject(memDC1_, hOldFont); 
    } 
    DeleteObject(hfnt);

    DeleteObject (hBrush);//


  DeleteObject (hBrush);
  DeleteObject (rgn);
  
  SelectObject (memDC1_, (HBITMAP) oldBmp_1);
  DeleteDC     (memDC1_);
  SelectObject (memDC2_, (HBITMAP) oldBmp_2);
  DeleteDC     (memDC2_);
  DeleteDC     (hDC);
  
  ICONINFO ii = {TRUE, 0, 0, iconMaskBmp_, iconBmp_};
  icon_ = CreateIconIndirect (&ii);

}

CDynamicIcon::~CDynamicIcon(){
      DestroyIcon (icon_);
      DeleteObject(iconBmp_);
      DeleteObject(iconMaskBmp_);
}

HICON CDynamicIcon::GetHIcon() {
      return icon_;
}

HFONT CDynamicIcon::CreateFont(const HDC hDC) 
{ 
    LOGFONT lf; 
 
//#define DEV_FIND_FONT
	#ifdef DEV_FIND_FONT //fontdialog for developper
    CHOOSEFONT cf; 
    cf.lStructSize = sizeof(CHOOSEFONT); 
    cf.hwndOwner = (HWND)NULL; 
    cf.hDC = hDC;//(HDC)NULL; 
    cf.lpLogFont = &lf; 
    cf.iPointSize = 0; 
    cf.Flags = CF_SCREENFONTS; 
    cf.rgbColors = RGB(0,0,0); 
    cf.lCustData = 0L; 
    cf.lpfnHook = (LPCFHOOKPROC)NULL; 
    cf.lpTemplateName = (LPSTR)NULL; 
    cf.hInstance = (HINSTANCE) NULL; 
    cf.lpszStyle = (LPSTR)NULL; 
    cf.nFontType = SCREEN_FONTTYPE; 
    cf.nSizeMin = 0; 
    cf.nSizeMax = 0; 
    ChooseFont(&cf);
	#endif //fontdialog for developper

   SecureZeroMemory(&lf,sizeof(LOGFONT));
    // int iitest = GetDeviceCaps(hDC, LOGPIXELSY);
    lf.lfHeight = -9; //-MulDiv(6, GetDeviceCaps(hDC, LOGPIXELSY), 64); // 64 neu
    lf.lfWeight=400;
    lf.lfOutPrecision = 1;
    lf.lfClipPrecision = 2;
    lf.lfQuality = 1;
    lf.lfPitchAndFamily = 34;
    strcpy_s(lf.lfFaceName,sizeof(lf.lfFaceName),"Small Fonts");
 
    return CreateFontIndirect(&lf);
};




