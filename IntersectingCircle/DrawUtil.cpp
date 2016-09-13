#include <windows.h>
#include "Fill.h"

DRAWING_DATA gDrawData; // global data

void reDraw(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, 1);
}

void drawPoint(int x, int y)
{
  SetPixel(gDrawData.hdcMem, x, y, CLR_BOUNDARY);
}

void drawPartialPoly()
{
  MoveToEx(gDrawData.hdcMem,
    gDrawData.cornerPts[0].x,gDrawData.cornerPts[0].y, NULL);
  for(int i=1;i < gDrawData.nCornerPts; i++)
  {
    LineTo(gDrawData.hdcMem,
         gDrawData.cornerPts[i].x,gDrawData.cornerPts[i].y);
    MoveToEx(gDrawData.hdcMem,
         gDrawData.cornerPts[i].x,gDrawData.cornerPts[i].y, NULL);
  }
}


void drawPoly(HWND hwnd)
{
  drawPartialPoly();
  LineTo(gDrawData.hdcMem,
         gDrawData.cornerPts[0].x,gDrawData.cornerPts[0].y);
  performFilling(hwnd);
}

void setDrawMode(MODE mode, HWND hwnd)
{
  gDrawData.drawMode = mode;
  setupMenus(hwnd);
}

void createMemoryBitmap(HDC hdc)
{
  gDrawData.hdcMem = CreateCompatibleDC(hdc);
  gDrawData.hbmp = CreateCompatibleBitmap(hdc,
       gDrawData.maxBoundary.cx, gDrawData.maxBoundary.cy);
  SelectObject(gDrawData.hdcMem, gDrawData.hbmp);
  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
       gDrawData.maxBoundary.cy, PATCOPY);
}

void initialize(HWND hwnd, HDC hdc)
{
  gDrawData.nCornerPts = 0;
  gDrawData.hDrawPen=CreatePen(PS_SOLID, 1, CLR_BOUNDARY);
  gDrawData.hFillPen=CreatePen(PS_SOLID, 1, CLR_FILL);

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

  PatBlt(gDrawData.hdcMem, 0, 0, gDrawData.maxBoundary.cx,
         gDrawData.maxBoundary.cy, PATCOPY);

  reDraw(hwnd);
  setupMenus(hwnd);
}

LRESULT CALLBACK DlgAxis(HWND hdlg, UINT mess, WPARAM more, LPARAM pos)
{
	char str[20];
	switch (mess)
	{
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

void addPointToPolygon(HWND hwnd, int x, int y)
{
  /* the coordinates of the points are stored in an array */

  if (gDrawData.nCornerPts < nMaxNoOfCornerPts)
  {
    SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);
	if(DialogBox(NULL, "MyDB", hwnd, (DLGPROC)DlgAxis)){
        int radius = gDrawData.radius[gDrawData.nCornerPts];
        //For transparent filling
        SelectObject(gDrawData.hdcMem, GetStockObject(HOLLOW_BRUSH));
        Ellipse(gDrawData.hdcMem, x-radius, y-radius, x+radius, y+radius);
        SelectObject(gDrawData.hdcMem, gDrawData.hDrawPen);

        gDrawData.cornerPts[gDrawData.nCornerPts].x = x;
        gDrawData.cornerPts[gDrawData.nCornerPts].y = y;
        gDrawData.nCornerPts++;
	}
  }
  else
  {
    drawPoly(hwnd);
    setDrawMode(DRAWN_MODE, hwnd);
    reDraw(hwnd);
  }
  drawPartialPoly();
}


void drawImage(HDC hdc)
{
  BitBlt(hdc, 0, 0, gDrawData.maxBoundary.cx,
    gDrawData.maxBoundary.cy, gDrawData.hdcMem, 0, 0, SRCCOPY);
}

void processLeftButtonDown(HWND hwnd, int x, int y)
{
  switch (gDrawData.drawMode)
  {
    case DRAW_MODE:
      addPointToPolygon(hwnd,x,y);
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
      hdc = GetDC(hwnd);
      initialize(hwnd, hdc);
      ReleaseDC(hwnd, hdc);
      break;

    case WM_COMMAND:
      processCommand(LOWORD(wParam), hwnd);
      break;

    case WM_LBUTTONDOWN:
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

    default:
      break;
  }
  // Call the default window handler
  return DefWindowProc(hwnd, message, wParam, lParam);
}
