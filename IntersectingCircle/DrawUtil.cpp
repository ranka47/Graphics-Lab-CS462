#include <windows.h>
#include "Fill.h"
#include <string>
#include <sstream>
#include <iostream>
using namespace std;
string legend = "";
DRAWING_DATA gDrawData; // global data
HWND hEdit;


//HWND: Handle to a window
//HDC: handle to a device context (display, printer, memory (or compatible), and information)
//Memory DC corresponds to drawing operations on BitMap
void reDraw(HWND hwnd)
{
  //NULL indicates the whole of th rectangular region
  //1 indicated whether the background needs to be erased
  InvalidateRect(hwnd, NULL, 1);
}

void setDrawMode(MODE mode, HWND hwnd)
{
  gDrawData.drawMode = mode;
}

void createMemoryBitmap(HDC hdc)
{
  //Creates a memory DC handle and returns to the variable
  gDrawData.hdcMem = CreateCompatibleDC(hdc);
  //creates a bitmap compatible with the device that is associated with the specified device context
  gDrawData.hbmp = CreateCompatibleBitmap(hdc,gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  //selects the object in the second arg into the specified HDC
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
  //paints the specified rectangle using the brush that is currently selected into the specified device context.
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.nCornerPts = 0;
  //CreatePen(PenStyle, Width, Color)
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, CLR_BOUNDARY);
  gDrawData.hAxisPen=CreatePen(PS_SOLID, 1, CLR_AXIS);
  gDrawData.hFillPen=CreatePen(PS_SOLID, 1, CLR_BG);
  gDrawData.hFillRed=CreatePen(PS_SOLID, 1, CLR_RED);
  gDrawData.hFillGreen=CreatePen(PS_SOLID, 1, CLR_GREEN);
  gDrawData.hFillBlue=CreatePen(PS_SOLID, 1, CLR_BLUE);
  gDrawData.hFillYellow=CreatePen(PS_SOLID, 1, CLR_YELLOW);
  gDrawData.hFillCyan=CreatePen(PS_SOLID, 1, CLR_CYAN);

  gDrawData.maxBoundary.cx = GetSystemMetrics(SM_CXSCREEN);
  gDrawData.maxBoundary.cy = GetSystemMetrics(SM_CYSCREEN);
  createMemoryBitmap(hdc);
  setDrawMode(DRAW_MODE, hwnd);
}

void cleanup()
{
  DeleteDC(gDrawData.hdcMem);
}

void reset(HWND hwnd)
{
  gDrawData.nCornerPts = 0;
  gDrawData.drawMode = DRAW_MODE;

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy, PATCOPY);
  SetWindowText(hEdit, TEXT("X,Y,Z: CYAN\n X && Y: BLUE\nY && Z: GREEN\n X && Z: YELLOW\n X && Y && Z: RED\n"));
  //function adds a rectangle to the specified window's update region.
  reDraw(hwnd);
}

LRESULT CALLBACK DlgAxis(HWND hdlg, UINT mess, WPARAM more, LPARAM pos)
{
	char str[20];
	switch (mess)
	{
    case WM_INITDIALOG:
        //Sets the keyboard focus
        SetFocus(GetDlgItem(hdlg, ID_EB1));
        break;
	case WM_COMMAND:
		switch (more)
		{
		case ID_OK:
			GetDlgItemText(hdlg, ID_EB1, str, 20);
			gDrawData.radius[gDrawData.nCornerPts] = (long int)(atof(str));
			if (gDrawData.radius[gDrawData.nCornerPts] < 0)
                MessageBox(hdlg, "Radius cannot be negative!", "Error!", MB_ICONERROR);
			else{
				EndDialog(hdlg, TRUE);
				return 1;
			}
			break;

		case ID_CANCEL:
			EndDialog(hdlg, FALSE);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hdlg, FALSE);
		break;
	default:
		break;
	}
	return 0;
}

void updateBoundary(int x, int y)
{
    //Updating the enclosed rectangle
    if(gDrawData.nCornerPts == 0) {
        gDrawData.lowerRightX = x + gDrawData.radius[0];
        gDrawData.upperLeftX = x - gDrawData.radius[0];
        gDrawData.lowerRightY = y - gDrawData.radius[0];
        gDrawData.upperLeftY = y + gDrawData.radius[0];
    }
    else {
        gDrawData.lowerRightX = max(gDrawData.lowerRightX, x+gDrawData.radius[gDrawData.nCornerPts]);
        gDrawData.lowerRightY = min(gDrawData.lowerRightY, y-gDrawData.radius[gDrawData.nCornerPts]);
        gDrawData.upperLeftX = min(gDrawData.upperLeftX, x-gDrawData.radius[gDrawData.nCornerPts]);
        gDrawData.upperLeftY = max(gDrawData.upperLeftY, y+gDrawData.radius[gDrawData.nCornerPts]);
    }
}

HPEN getPen(int x, int y)
{
    //Choosing the appropriate pen that is reqd to color the line containing the point (x,y) as midpoint
    int dist1 = (gDrawData.centre[0].x - x)*(gDrawData.centre[0].x - x) + (gDrawData.centre[0].y - y)*(gDrawData.centre[0].y - y) - gDrawData.radius[0]*gDrawData.radius[0];
    int dist2 = (gDrawData.centre[1].x - x)*(gDrawData.centre[1].x - x) + (gDrawData.centre[1].y - y)*(gDrawData.centre[1].y - y) - gDrawData.radius[1]*gDrawData.radius[1];
    int dist3 = (gDrawData.centre[2].x - x)*(gDrawData.centre[2].x - x) + (gDrawData.centre[2].y - y)*(gDrawData.centre[2].y - y) - gDrawData.radius[2]*gDrawData.radius[2];
    bool one = dist1 < 0;
    bool two = dist2 < 0;
    bool three = dist3 < 0;
    if(one && two && three)
        return gDrawData.hFillRed;
    if(one && two)
        return gDrawData.hFillBlue;
    if(two && three)
        return gDrawData.hFillGreen;
    if(one && three)
        return gDrawData.hFillYellow;
    if(one || two || three)
        return gDrawData.hFillCyan;
    return gDrawData.hFillPen;
}

void scanFill()
{
    int y = gDrawData.lowerRightY+1;
    int startX, endX, x;

    while(y < gDrawData.upperLeftY)
    {
        x = gDrawData.upperLeftX+1;
        if(x < 0)
        {
            startX = 0;
            x = 0;
            while(x < gDrawData.lowerRightX && x <= gDrawData.maxBoundary.cx && !(GetPixel(gDrawData.hdcMem, x+1, y) == CLR_BOUNDARY && GetPixel(gDrawData.hdcMem, x, y) == CLR_BG))
                x++;
            x++;
            endX = x;
            if(startX < gDrawData.lowerRightX && endX <= gDrawData.lowerRightX) {
                SelectObject(gDrawData.hdcMem, getPen((startX+endX)/2, y));
                MoveToEx(gDrawData.hdcMem,startX,y, NULL);
                LineTo(gDrawData.hdcMem,endX,y);
            }
        }
        while(x < gDrawData.lowerRightX)
        {
            while(x < gDrawData.lowerRightX && x<=gDrawData.maxBoundary.cx && !(GetPixel(gDrawData.hdcMem, x-1, y) == CLR_BOUNDARY && GetPixel(gDrawData.hdcMem, x, y) == CLR_BG))
                x++;
            startX = x;
            while(x < gDrawData.lowerRightX && x<=gDrawData.maxBoundary.cx && !(GetPixel(gDrawData.hdcMem, x+1, y) == CLR_BOUNDARY && GetPixel(gDrawData.hdcMem, x, y) == CLR_BG))
                x++;
            x++;
            endX = x;
            if(startX < gDrawData.lowerRightX && endX <= gDrawData.lowerRightX) {
                SelectObject(gDrawData.hdcMem, getPen((startX+endX)/2, y));
                MoveToEx(gDrawData.hdcMem,startX,y, NULL);
                LineTo(gDrawData.hdcMem,endX,y);
            }
        }
        y++;
    }
}

void Marker(LONG x, LONG y, HDC hdc, int i)
{
    SelectObject(gDrawData.hdcMem, gDrawData.hAxisPen);
    MoveToEx(hdc, (int) x - 10 - gDrawData.radius[i], (int) y, (LPPOINT) NULL);
    LineTo(hdc, (int) x + 10 + gDrawData.radius[i], (int) y);
    MoveToEx(hdc, (int) x, (int) y - 10 - gDrawData.radius[i], (LPPOINT) NULL);
    LineTo(hdc, (int) x, (int) y + 10 + gDrawData.radius[i]);
    SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
}

void drawCircle(HWND hwnd, int x, int y)
{
  /* the coordinates of the points are stored in an array */

  if (gDrawData.nCornerPts < nMaxNoOfCornerPts)
  {
    SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
	if(DialogBox(NULL, "MyDB", hwnd, (DLGPROC)DlgAxis)){
        int radius = gDrawData.radius[gDrawData.nCornerPts];
        updateBoundary(x, y);
//        SetPixel(gDrawData.hdcMem,x,y,RGB(128,128,128));
//        Marker(x,y,gDrawData.hdcMem);
        Arc(gDrawData.hdcMem, x-radius, y-radius, x+radius, y+radius, x, y-radius, x, y-radius);
        gDrawData.centre[gDrawData.nCornerPts].x = x;
        gDrawData.centre[gDrawData.nCornerPts].y = y;
        gDrawData.nCornerPts++;
        if(gDrawData.nCornerPts == nMaxNoOfCornerPts){
            setDrawMode(DRAWN_MODE, hwnd);
            ostringstream temp;
            temp<<"X,Y,Z: CYAN\n X && Y: BLUE\nY && Z: GREEN\n X && Z: YELLOW\n X && Y && Z: RED\n";
            temp<<"X:("<<gDrawData.centre[0].x<<","<<gDrawData.centre[0].y<<")"<<endl;
            temp<<"Y:("<<gDrawData.centre[1].x<<","<<gDrawData.centre[1].y<<")"<<endl;
            temp<<"Z:("<<gDrawData.centre[2].x<<","<<gDrawData.centre[2].y<<")"<<endl;
            legend = temp.str();
            const char* a = legend.c_str();
            SetWindowText(hEdit, TEXT(a));
        }
	}
  }
}


void drawImage(HDC hdc)
{
  //performs a bit-block transfer of the color data from src to dst
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx,
    gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_MODE:
      drawCircle(hwnd,x,y);
      reDraw(hwnd);
      break;
    default:
      return;
  }
}

void processCommand(int cmd, HWND hwnd)
{
  int response;
  switch(cmd)
  {
    case ID_CLEAR:
      reset(hwnd);
      setDrawMode(DRAW_MODE, hwnd);
      break;
    case ID_EXIT:
      response=MessageBox(hwnd,"Quit the program?", "EXIT", MB_YESNO);
      if(response==IDYES)
        PostQuitMessage(0);
      break;
    default:
      break;
  }
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
    {
        hEdit = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, GetSystemMetrics(SM_CXSCREEN)-200,0,200,150, hwnd, NULL,GetModuleHandle(NULL), NULL);
        SetWindowText(hEdit, TEXT("X,Y,Z: CYAN\n X && Y: BLUE\nY && Z: GREEN\n X && Z: YELLOW\n X && Y && Z: RED\n"));

        hdc = GetDC(hwnd);
        initialize(hwnd, hdc);
        ReleaseDC(hwnd, hdc);
    }
    break;

    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;
    case WM_LBUTTONDOWN:
      x = LOWORD(lParam);
      y = HIWORD(lParam);
      processLeftButtonDown(hwnd, x,y);
      break;
    case WM_CHAR:
        switch(wParam)
        {
            case 0x0D:
            if(gDrawData.drawMode == DRAWN_MODE){
                 scanFill();
                 reDraw(hwnd);
             }
             break;
        }
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

    default:
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}
