#include <math.h>
#include <windows.h>
#include "Parabola.h"
#include <iostream>
using namespace std;
DRAWING_DATA gDrawData; // global data
void drawParabola(HWND);
LRESULT CALLBACK DlgAxis(HWND hdlg,UINT mess,WPARAM more,LPARAM pos);

void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
  Ellipse(gDrawData.hdcMem,x-2,y-2,x+2,y+2);    //Inbuilt ellipse function. It draws the small circle around the center.
}

void setupMenus(HWND hwnd)
{
  HMENU hMenu;

  hMenu=GetMenu(hwnd);

  switch (gDrawData.drawMode)
  {
    case READY_MODE :
      // enable ellipse menu
      EnableMenuItem(hMenu, ID_PARABOLA, MF_BYCOMMAND|MF_ENABLED);
      break;
    case DRAW_MODE :
      break;
  }
}

void setDrawMode(MODE mode, HWND hwnd)
{
  gDrawData.drawMode = mode;
  setupMenus(hwnd);
}

void createMemoryBitmap(HDC hdc)
{
  gDrawData.hdcMem = CreateCompatibleDC(hdc);   //creates a memory device context compatible with the specified device.
  gDrawData.hbmp = CreateCompatibleBitmap(hdc,
    gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);   //selects object (2nd Param) into the specified DC (1st Param).
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
         gDrawData.maxBoundary.cy, PATCOPY);    //Paints the specified rectangle using the brush i.e. currently selected into specified device context
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

  gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
  gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);

  //Setting the center of the screen as the center of the Cartesian System
  gDrawData.centre.x = GetSystemMetrics(SM_CXSCREEN)/2;
  gDrawData.centre.y = GetSystemMetrics(SM_CYSCREEN)/2;

  createMemoryBitmap(hdc);
  setDrawMode(READY_MODE, hwnd);
}

void cleanup()
{
  DeleteDC(gDrawData.hdcMem);
}

void reset(HWND hwnd)
{
  gDrawData.drawMode = READY_MODE;
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
         gDrawData.maxBoundary.cy, PATCOPY);
  reDraw(hwnd);
  setupMenus(hwnd);
}

void plot_sympoint(int ex, int ey, COLORREF clr)
{
  int cx = gDrawData.centre.x;
  int cy = gDrawData.centre.y;

  SetPixel(gDrawData.hdcMem, ex+cx,cy-ey, clr);
  SetPixel(gDrawData.hdcMem, ex+cx,cy+ey, clr);
}

void addPoint(HWND hwnd)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_MODE:
      /* the coordinates of the centre is stored
         and the ellipse is drawn */
      SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);   //selects pen in the memory DC
      drawParabola(hwnd);
      reDraw(hwnd);
      break;
    default:
      break;
  }
}

void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx,
    gDrawData.maxBoundary.cy, gDrawData.hdcMem,
    0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
}

void processCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      setDrawMode(READY_MODE, hwnd);
      break;
    case ID_PARABOLA:
      setDrawMode(DRAW_MODE, hwnd);
      //DialogBox returns TRUE then only drawParabola
      if(DialogBox(NULL,"MyDB",hwnd,(DLGPROC)DlgAxis)){
        addPoint(hwnd);
      }
      break;
    case ID_EXIT:
        response=MessageBox(hwnd,
          "Quit the program?", "EXIT",
          MB_YESNO);
        if(response==IDYES)
            PostQuitMessage(0);
        break;
    default:
      break;
  }
}

void drawParabola(HWND hwnd)
{
  cout<<"Parabola: y^2="<<gDrawData.latusRectum<<"x"<<endl;
  int x,y;
  x=0;
  y=0;
  plot_sympoint(x,y, RGB(0,0,0));   //Plotted all the axes point
  //Starting from (0,0)
  cout<<"'y' being incremented!"<<endl;
  double decisionParam = gDrawData.latusRectum*0.5 - 1;
  double limit = gDrawData.latusRectum/(double)2;

  while(y < limit && y<=gDrawData.range){
    if(decisionParam<0){
        x++;
        decisionParam = decisionParam + gDrawData.latusRectum - 2*y - 1;
    }
    else{
        decisionParam = decisionParam - 2*y - 1;
    }
    y++;
    cout<<"x:"<<x<<" y:"<<y<<" DecP:"<<decisionParam<<endl;
    plot_sympoint(x,y, RGB(0,0,0));
  }

  cout<<"'x' being incremented!"<<endl;

  decisionParam=(x+1)*gDrawData.latusRectum - (y+0.5)*(y+0.5);    //Initial value of decision parameter
  while(y<=gDrawData.range)    //Condition true until you move out of region with |m| <= 1 line
  {
    if(decisionParam<0)
    {
      decisionParam = decisionParam + gDrawData.latusRectum;
    }
    else
    {
      decisionParam=decisionParam + gDrawData.latusRectum - 2*y - 1;
      y++;
    }
    x++;
    cout<<"x:"<<x<<" y:"<<y<<" DecP:"<<decisionParam<<endl;
    plot_sympoint(x,y, RGB(0,0,0));
  }
}

LRESULT CALLBACK DlgAxis(HWND hdlg,UINT mess,WPARAM more,LPARAM pos)
{
  char str[20];
  switch(mess)
  {
    case WM_COMMAND:
      switch(more)
      {
        case ID_OK:
          GetDlgItemText(hdlg,ID_EB1,str,20);
          gDrawData.range=(long int)(atof(str));
          GetDlgItemText(hdlg,ID_EB2,str,20);
          gDrawData.latusRectum=(long int)(atof(str));
          if(gDrawData.range < 10 || gDrawData.range > 100 || gDrawData.latusRectum < 1 || gDrawData.latusRectum > 50)
          {
            MessageBox(hdlg, "Range of value of y should be between 10 and 100 and latus rectum be between 1 and 50.", "Warning!",MB_ICONERROR);
          }
          else{
                EndDialog(hdlg,TRUE);
                return 1;
          }
          break;

        case ID_CANCEL:
          EndDialog(hdlg,FALSE);
          break;
      }
      break;
    case WM_CLOSE:
        EndDialog(hdlg,FALSE);
        break;
    default:
      break;
  }
  return 0;
}

LRESULT CALLBACK WindowF(HWND hwnd,UINT message,WPARAM wParam,
                         LPARAM lParam)
{
  HDC hdc;
  PAINTSTRUCT ps;
  int x,y;

  switch(message)
  {
    case WM_CREATE:
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      ReleaseDC(hwnd, hdc);
      break;

    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;

    case WM_LBUTTONDOWN:
      //NOT USED!!!
      x = LOWORD(lParam);
      y = HIWORD(lParam);
      processLeftButtonDown(hwnd, x,y);
      break;

    case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);
      drawImage(hdc);
      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      cleanup();
      PostQuitMessage(0);
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}
