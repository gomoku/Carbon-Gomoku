/*
 *  OXMain.cpp
 *  23.01.2002, Michal Czardybon
 *
 *  Klient Window API - aplikacja u¿ytkownika
 *
 */

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"

#include "OXGame.h"
#include "AISimple.h"
#include "AICarbon.h"
#include "Timer.h"

#pragma comment(lib,"comctl32.lib")

int info_timeout_turn=1000; /* time for one turn in milliseconds */
int info_time_left=1000000000; /* left time for a game */
int info_max_memory=350000000; /* maximum memory in bytes */
int info_exact5=0;
int info_renju=0;
int terminateAI;

int va, vb;
// Komunikat - komputer skonczyl liczyc
#define WM_PERFORM_MOVE (WM_USER + 0)

// -----------------------------------------------------------------------------
// Dane inteface-u
// -----------------------------------------------------------------------------
const LPSTR lpszMainWndClassName = "CarbonGomokuMainWndClass";
const LPSTR lpszCaption = "Carbon Gomoku";
const int cellSize = 17;

HINSTANCE ghInstance; // global handle
HMENU     hMenu;
DWORD     aiThreadId;
HANDLE    hAiThread;
bool      aiThinking;
HWND      hWndLog;    // uchwyt okna logowania

RECT    clientRect;
#define clientWidth (clientRect.right - clientRect.left)
#define clientHeight (clientRect.bottom - clientRect.top)

// -----------------------------------------------------------------------------
// Dane dotyczace stanu gry
// -----------------------------------------------------------------------------
const int  boardSize = 19;

const char playerName[][32] = {"Human", "AI - Simple 1", "AI - Carbon"};
const int  playerCount = 3;

OXGame    game;
OXPlayer  *playerO, *playerX, *currPlayer;
int       iPlayerO, iPlayerX;
int       oSearchDepth, oTimeLimit;
int       xSearchDepth, xTimeLimit;

Timer     oTimer, xTimer;

// -----------------------------------------------------------------------------
// Prototypy funkcji modyfikujacych stan gry
// -----------------------------------------------------------------------------
void  NewPlayer(OXPlayer *&player, int playerIndex);
void  StartNewGame(HWND hWnd);
void  PerformMove(HWND hWnd, int x, int y);
DWORD AIThreadProc(LPVOID param);
// -----------------------------------------------------------------------------
// Prototypy funkcji obslugujacych zdarzenia
// -----------------------------------------------------------------------------
void OnCreate(HWND hWnd);
void OnCommand(HWND hWnd, WPARAM wParam);
void OnLeftClick(HWND hWnd, int x, int y);
void OnRightClick(HWND hWnd, int x, int y);
void OnPaint(HWND hWnd);
void OnMove(HWND hWnd, int cx, int cy);
void RestoreSize(HWND hWnd);
void RestoreSize(HWND hWnd, RECT *pRect);
void InvalidateCell(HWND hWnd, int cx, int cy);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// -----------------------------------------------------------------------------
// Prototypy funkcji obslugujacych okienko dialogowe NewGame
// -----------------------------------------------------------------------------
INT_PTR CALLBACK NewGameDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnInitDialog(HWND hWnd);
void SwitchPlayers(HWND hWnd);
void UpdateControls(HWND hWnd);
void GetValues(HWND hWnd);
// -----------------------------------------------------------------------------
// Prototypy funkcji dla okienka dialogowego Log
// -----------------------------------------------------------------------------
INT_PTR CALLBACK LogDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ClearLog();
int  nTotalO, nTotalX; // searched
// -----------------------------------------------------------------------------
// Standard Windows API WinMain function
// -----------------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR /*lpszCmdLine*/, int nCmdShow)
{
  WNDCLASS wc;
  MSG msg;
  HWND hWndMain;

  ghInstance = hInstance;

  if(!hPrevInstance)
    {
      wc.lpszClassName = lpszMainWndClassName;
      wc.lpfnWndProc   = MainWndProc;
      wc.style         = 0;
      wc.hInstance     = hInstance;
      wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
      wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
      wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
      wc.lpszMenuName  = "MainMenu";
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = 0;
      RegisterClass(&wc);
    }

  InitCommonControls();

  /* ustalanie rozmiarow okna, tak aby obszar klienta mial zadana wielkosc */
  SetRect(&clientRect, 0, 0, boardSize * cellSize + 1, boardSize * cellSize + 1);
  AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_CLIENTEDGE);

  hWndMain = CreateWindowEx(
                WS_EX_CLIENTEDGE, lpszMainWndClassName, lpszCaption,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                clientWidth, clientHeight,
                NULL, NULL, hInstance, NULL);

  HACCEL haccel = LoadAccelerators(hInstance, "Accel");

  hWndLog = CreateDialog(hInstance, "LogDialog", 0, LogDialogProc);
  //ShowWindow(hWndLog, nCmdShow);
  
  ShowWindow(hWndMain, nCmdShow);

  while(GetMessage(&msg, NULL, 0, 0))
    {
      if(!TranslateAccelerator(hWndMain, haccel, &msg))
        if (!IsDialogMessage(hWndLog, &msg))
          {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
    }

  return (int)msg.wParam;
}
// -----------------------------------------------------------------------------
// Standard Windows API window's procedure
// -----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
      case WM_CREATE:   OnCreate(hWnd);      break;
      case WM_DESTROY:  PostQuitMessage(0);  break;
      case WM_PAINT:    OnPaint(hWnd);       break;    
      case WM_SIZE:     RestoreSize(hWnd);   break;
      case WM_SIZING:   RestoreSize(hWnd, (RECT*)lParam); break;
      case WM_COMMAND:
        OnCommand(hWnd, wParam);
        break;   
      case WM_LBUTTONDOWN:
        OnLeftClick(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;
      case WM_RBUTTONDOWN:
        OnRightClick(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;
      case WM_PERFORM_MOVE:
        OnMove(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;
      default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
  return 0;
}
// -----------------------------------------------------------------------------
// Funkcja watku realizujacego obliczenia AI
// -----------------------------------------------------------------------------
DWORD AIThreadProc(LPVOID param)
{
  int cx, cy;
  
  //Sleep(2000);
  game.player() == OP ?
    currPlayer->yourTurn(cx, cy, oSearchDepth, oTimeLimit) :
    currPlayer->yourTurn(cx, cy, xSearchDepth, xTimeLimit);
  currPlayer->move(cx, cy);

  aiThinking = false;
  if(terminateAI != 9) 
    SendMessage((HWND)param, WM_PERFORM_MOVE, 0, MAKELONG(cx, cy));

  return 0;
}
// -----------------------------------------------------------------------------
// Funkcje modyfikujace stan gry
// -----------------------------------------------------------------------------
void NewPlayer(OXPlayer *&player, int playerIndex)
{
  delete player;
  switch (playerIndex)
    {
      case 0: player = 0; break;
      case 1: player = new AISimple; break;
      case 2: player = new AICarbon; break;
    }
}
// -----------------------------------------------------------------------------
void StartThinking(HWND hWnd)
{
  if(currPlayer)
  {
    EnableMenuItem(hMenu, IDM_FILE_UNDO, MF_GRAYED);

    if(hAiThread) CloseHandle(hAiThread);
    terminateAI = 0;
    aiThinking = true;
    hAiThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AIThreadProc, hWnd, 0, &aiThreadId);
  }
  else if(game.moveCount() >= 3)
  {
    EnableMenuItem(hMenu, IDM_FILE_UNDO, MF_ENABLED);
  }
}
// -----------------------------------------------------------------------------
void StartNewGame(HWND hWnd)
{
  ClearLog();

  NewPlayer(playerO, iPlayerO);
  NewPlayer(playerX, iPlayerX);

  if (playerO != 0) playerO->start(boardSize);
  if (playerX != 0) playerX->start(boardSize);

  char cap[256];
  strcpy(cap, lpszCaption);
  strcat(cap, " - [");
  if (playerO != 0) strcat(cap, playerO->name()); else strcat(cap, "Human");
  strcat(cap, " vs ");
  if (playerX != 0) strcat(cap, playerX->name()); else strcat(cap, "Human");
  strcat(cap, "]");
  SetWindowText(hWnd, cap);

  oTimer.reset();
  xTimer.reset(); oTimer.start();

  game.start(boardSize);
  UpdateWindow(hWnd);

  currPlayer = playerO;
  StartThinking(hWnd);
}
// -----------------------------------------------------------------------------
void PerformMove(HWND hWnd, int x, int y)
{
  game.player() == OP ? oTimer.stop() : xTimer.stop();

  InvalidateCell(hWnd, game.lastMove().x, game.lastMove().y);
  game.move(x, y);
  InvalidateCell(hWnd, x, y);

  if (game.finished())
    {
      InvalidateRect(hWnd, 0, 1);
      char info[256] = "The winner is:\n";
      char buf[256];
      strcat(info, currPlayer == 0 ? "Human" : currPlayer->name());
      strcat(info, "\n");

      sprintf(buf, "O TIME = %.3lf\n", oTimer.time());
      strcat(info, buf);
      sprintf(buf, "X TIME = %.3lf", xTimer.time());
      strcat(info, buf);

      MessageBox(hWnd, info, "Game Over", MB_ICONINFORMATION);
      delete playerO;
      delete playerX;
      playerO = 0;
      playerX = 0;
      return;
    }
 
  game.player() == OP ? oTimer.start() : xTimer.start();

  currPlayer = currPlayer == playerO ? playerX : playerO;

  if(currPlayer) currPlayer->move(x, y);
  StartThinking(hWnd);
}
// -----------------------------------------------------------------------------
// Funkcje obslugujace zdarzenia okna glownego
// -----------------------------------------------------------------------------
void RestoreSize(HWND hWnd, RECT *pRect)
{
  pRect->right = pRect->left + clientWidth;
  pRect->bottom = pRect->top + clientHeight;
}
// -----------------------------------------------------------------------------
void RestoreSize(HWND hWnd)
{
  SetWindowPos(hWnd, HWND_TOP, 0, 0, clientWidth, clientHeight, SWP_NOMOVE);
}
// -----------------------------------------------------------------------------
void OnMove(HWND hWnd, int cx, int cy)
{
  PerformMove(hWnd, cx, cy);
}
// -----------------------------------------------------------------------------
void OnCreate(HWND hWnd)
{
  aiThinking = false;
  playerO = playerX = currPlayer = 0;
  iPlayerO = 2; iPlayerX = 0;
  // deaktywowanie polecenia UNDO
  hMenu = GetMenu(hWnd);
  EnableMenuItem(hMenu, IDM_FILE_UNDO, MF_GRAYED);
}
// -----------------------------------------------------------------------------
void OnCommand(HWND hWnd, WPARAM wParam)
{
  switch(LOWORD(wParam))
    {
      case IDM_FILE_NEW:
        if (aiThinking)
        {
            terminateAI=9;
            if(WaitForSingleObject(hAiThread, 3000) == WAIT_TIMEOUT)
              TerminateThread(hAiThread,1);
        }
        if (DialogBox(ghInstance, "NewGameDialog", hWnd, NewGameDialogProc) == 1)
          {
            StartNewGame(hWnd);       
            InvalidateRect(hWnd, 0, 1);
          }
        break;
      
      case IDM_FILE_UNDO:
        game.undo();
        game.undo();
        if (playerO != 0) {playerO->undo(); playerO->undo();}
        if (playerX != 0) {playerX->undo(); playerX->undo();}
        InvalidateRect(hWnd, 0, 1);
        EnableMenuItem(hMenu, IDM_FILE_UNDO, MF_GRAYED);
        break;

      case IDM_FILE_LOG:
        ShowWindow(hWndLog, SW_SHOW);
        break;

      case IDM_HELP_ABOUT:
        MessageBox(hWnd, "Carbon Gomoku 2.2 (" __DATE__ ")\nauthor: Micha³ Czardybon", "About", 0);
        break;

      case IDM_FILE_EXIT:
        SendMessage(hWnd, WM_DESTROY, 0, 0);
        break;
      
    }
}
// -----------------------------------------------------------------------------
// informacje dla debuggowania
void OnRightClick(HWND hWnd, int x, int y)
{
  /*int cx = x / cellSize;
  int cy = y / cellSize;
  char s[100], s1[100], s2[100], s3[100], s4[100];
  int v1, v2, v3, v4;
  AICarbon *carbon = (AICarbon*)playerO;
  carbon->cell[cx + 4][cy + 4].update4();
  v1 = carbon->cell[cx + 4][cy + 4].status1[0][1];
  v2 = carbon->cell[cx + 4][cy + 4].status1[1][1];
  v3 = carbon->cell[cx + 4][cy + 4].status1[2][1];
  v4 = carbon->cell[cx + 4][cy + 4].status1[3][1];
  itoa(v1, s1, 10);
  itoa(v2, s2, 10);
  itoa(v3, s3, 10);
  itoa(v4, s4, 10);
  strcpy(s, s1); strcat(s, " ");
  strcat(s, s2); strcat(s, " ");
  strcat(s, s3); strcat(s, " ");
  strcat(s, s4); strcat(s, " ");
  SetWindowText(hWnd, s);*/
}
// -----------------------------------------------------------------------------
void OnLeftClick(HWND hWnd, int x, int y)
{
  int cx = x / cellSize;
  int cy = y / cellSize;

  if (game.finished() || currPlayer != 0 || game.cell(cx, cy) != EMPTY)
    {
      MessageBeep(0);
      return;
    }
  SendMessage(hWnd, WM_PERFORM_MOVE, 0, MAKELONG(cx, cy));
}
// -----------------------------------------------------------------------------
// Powoduje odmalowanie komorki [cx][cy]
void InvalidateCell(HWND hWnd, int cx, int cy)
{
  RECT r;
  SetRect(&r, cx * cellSize, cy * cellSize, (cx + 1) * cellSize, (cy + 1) * cellSize);
  InvalidateRect(hWnd, &r, true);
}
// -----------------------------------------------------------------------------
void OnPaint(HWND hWnd)
{
  PAINTSTRUCT ps;
  HDC    hdc;
  HPEN   gridPen, defPen, oPen, xPen, oPenThick, xPenThick, framePen;
  HBRUSH defBrush;
  RECT   clientRect;
  int x, y;
  // inicjowanie
  hdc = BeginPaint(hWnd, &ps);
  
  // Ustalanie koloru siatki jako zmodyfikowanego koloru tla
  int r, g, b;
  COLORREF gridCol = GetSysColor(COLOR_BTNFACE);
  r = GetRValue(gridCol);
  g = GetGValue(gridCol);
  b = GetBValue(gridCol);
  gridCol = RGB(0.9 * r, 0.9 * g, 0.9 * b);
  gridPen = CreatePen(PS_SOLID, 1, gridCol); 

  // Tworzenie pozostalych pior
  oPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 128));
  xPen = CreatePen(PS_SOLID, 2, RGB(128, 0, 0));
  oPenThick = CreatePen(PS_SOLID, 4, RGB(0, 0, 128));
  xPenThick = CreatePen(PS_SOLID, 3, RGB(128, 0, 0));
  framePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));

  defPen = (HPEN)SelectObject(hdc, gridPen);
  defBrush = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  GetClientRect(hWnd, &clientRect);
  // pionowe linie siatki
  for (x = clientRect.left; x <= clientRect.right; x += cellSize)
    {
      MoveToEx(hdc, x, clientRect.top, 0);
      LineTo(hdc, x, clientRect.bottom);
    }
  // poziome linie siatki
  for (y = clientRect.top; y <= clientRect.bottom; y += cellSize)
    {
      MoveToEx(hdc, clientRect.left, y, 0);
      LineTo(hdc, clientRect.right, y);
    }
  // kolka i krzyzyki
  for (y = 0; y < game.boardSize(); y++)
    for (x = 0; x < game.boardSize(); x++)
      {
        if (game.cell(x, y) == OP)
          {
            SelectObject(hdc, game.lastMove() == OXPoint(x, y) ? oPenThick : oPen);
            Ellipse(hdc, x * cellSize + 3, y * cellSize + 3, (x + 1) * cellSize - 2, (y + 1) * cellSize - 2);
          }
        else if (game.cell(x, y) == XP)
          {
            SelectObject(hdc, game.lastMove() == OXPoint(x, y) ? xPenThick : xPen);
            MoveToEx(hdc, x * cellSize + 3, y * cellSize + 3, 0);
            LineTo(hdc, (x + 1) * cellSize - 2, (y + 1) * cellSize - 2);
            MoveToEx(hdc, (x + 1) * cellSize - 2, y * cellSize + 3, 0);
            LineTo(hdc, x * cellSize + 3, (y + 1) * cellSize - 2);
          }
        if (game.isWinPoint(x, y))
          {
            SelectObject(hdc, framePen);
            Rectangle(hdc, x * cellSize + 1, y * cellSize + 1, (x + 1) * cellSize + 1, (y + 1) * cellSize + 1);
          }
      }
  // zakonczenie operacji graficznych
  SelectObject(hdc, defPen);
  SelectObject(hdc, defBrush);
  DeleteObject(gridPen);
  DeleteObject(oPen);
  DeleteObject(oPenThick);
  DeleteObject(xPen);
  DeleteObject(xPenThick);
  DeleteObject(framePen);
  EndPaint(hWnd, &ps);
}
// -----------------------------------------------------------------------------
// Procedura okna dialogowego NewGame
// -----------------------------------------------------------------------------
INT_PTR CALLBACK NewGameDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
    {
      case WM_INITDIALOG:
        OnInitDialog(hWnd);
        return TRUE;
      
      case WM_COMMAND:
        switch (LOWORD(wParam))
          {
            case ID_BUTTON_START:
              GetValues(hWnd);
              EndDialog(hWnd, 1);              
              return TRUE;
            
            case ID_BUTTON_CANCEL:
            case IDCANCEL:
              GetValues(hWnd);
              EndDialog(hWnd, 0);
              return TRUE;

            case ID_BUTTON_SWITCH:
              SwitchPlayers(hWnd);
              return TRUE;
            
            case ID_COMBO_O:
              if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                  iPlayerO = (int)SendDlgItemMessage(hWnd, ID_COMBO_O, CB_GETCURSEL, 0, 0);
                  GetValues(hWnd);
                  UpdateControls(hWnd);
                }
              return TRUE;

            case ID_COMBO_X:
              if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                  iPlayerX = (int)SendDlgItemMessage(hWnd, ID_COMBO_X, CB_GETCURSEL, 0, 0);
                  GetValues(hWnd);
                  UpdateControls(hWnd);
                }
              return TRUE;
          }
        return FALSE;

      case WM_CLOSE:
        EndDialog(hWnd, 0);
        return TRUE;
    }
  return FALSE;
}
// -----------------------------------------------------------------------------
// Wywolywana przed wyswietleniem okienka NewGame
// -----------------------------------------------------------------------------
void OnInitDialog(HWND hWnd)
{
  // Wypelnianie opcji ComboBox-ow
  for (int i = 0; i < playerCount; i++)
    {
      SendDlgItemMessage(hWnd, ID_COMBO_O, CB_ADDSTRING, 0, (LPARAM)playerName[i]);
      SendDlgItemMessage(hWnd, ID_COMBO_X, CB_ADDSTRING, 0, (LPARAM)playerName[i]);
    }

  // Ustawianie UpDown-ow
  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_O, UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, ID_EDIT_DEPTH_O), 0);
  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_O, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));

  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_X, UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, ID_EDIT_DEPTH_X), 0);
  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_X, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 0));

  SendDlgItemMessage(hWnd, ID_SPIN_TIME_O, UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, ID_EDIT_TIME_O), 0);
  SendDlgItemMessage(hWnd, ID_SPIN_TIME_O, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60, 0));

  SendDlgItemMessage(hWnd, ID_SPIN_TIME_X, UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, ID_EDIT_TIME_X), 0);
  SendDlgItemMessage(hWnd, ID_SPIN_TIME_X, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60, 0));

  UpdateControls(hWnd);
}
// -----------------------------------------------------------------------------
void swap(int &a, int &b) {int t = a; a = b; b = t;}
// -----------------------------------------------------------------------------
// zamienia graczy w okienku NewGame
// -----------------------------------------------------------------------------
void SwitchPlayers(HWND hWnd)
{
  GetValues(hWnd);

  swap(iPlayerO, iPlayerX);
  swap(oSearchDepth, xSearchDepth);
  swap(oTimeLimit, xTimeLimit);
  
  UpdateControls(hWnd);
}
// -----------------------------------------------------------------------------
// Uaktualnia zawartosc elementow sterujacych w oknie NewGame
// -----------------------------------------------------------------------------
void UpdateControls(HWND hWnd)
{
  SendDlgItemMessage(hWnd, ID_COMBO_O, CB_SETCURSEL, iPlayerO, 0);
  SendDlgItemMessage(hWnd, ID_COMBO_X, CB_SETCURSEL, iPlayerX, 0);
  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_O, UDM_SETPOS, 0, oSearchDepth);
  SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_X, UDM_SETPOS, 0, xSearchDepth);
  SendDlgItemMessage(hWnd, ID_SPIN_TIME_O, UDM_SETPOS,  0, oTimeLimit);
  SendDlgItemMessage(hWnd, ID_SPIN_TIME_X, UDM_SETPOS,  0, xTimeLimit);

  EnableWindow(GetDlgItem(hWnd, ID_EDIT_DEPTH_O), iPlayerO != 0);
  EnableWindow(GetDlgItem(hWnd, ID_EDIT_TIME_O),  iPlayerO != 0);
  EnableWindow(GetDlgItem(hWnd, ID_EDIT_DEPTH_X), iPlayerX != 0);
  EnableWindow(GetDlgItem(hWnd, ID_EDIT_TIME_X),  iPlayerX != 0);

  EnableWindow(GetDlgItem(hWnd, ID_SPIN_DEPTH_O), iPlayerO != 0);
  EnableWindow(GetDlgItem(hWnd, ID_SPIN_TIME_O),  iPlayerO != 0);
  EnableWindow(GetDlgItem(hWnd, ID_SPIN_DEPTH_X), iPlayerX != 0);
  EnableWindow(GetDlgItem(hWnd, ID_SPIN_TIME_X),  iPlayerX != 0);
}
// -----------------------------------------------------------------------------
// Zapamiectuje w zmiennych globalnych dane z przyciskow up-down
// -----------------------------------------------------------------------------
void GetValues(HWND hWnd)
{
  oSearchDepth = (int)SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_O, UDM_GETPOS, 0, 0);
  xSearchDepth = (int)SendDlgItemMessage(hWnd, ID_SPIN_DEPTH_X, UDM_GETPOS, 0, 0);
  oTimeLimit = (int)SendDlgItemMessage(hWnd, ID_SPIN_TIME_O, UDM_GETPOS, 0, 0);
  xTimeLimit = (int)SendDlgItemMessage(hWnd, ID_SPIN_TIME_X, UDM_GETPOS, 0, 0);
}
// -----------------------------------------------------------------------------
// Procedura okna dialogowego Log
// -----------------------------------------------------------------------------
INT_PTR CALLBACK LogDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
    {
      case WM_INITDIALOG:
        SendDlgItemMessage(hWnd, ID_LOG_O, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), 0);
        SendDlgItemMessage(hWnd, ID_LOG_X, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), 0);
        return TRUE;

      case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        return TRUE;
    }
  return FALSE;
}
// -----------------------------------------------------------------------------
void ClearLog()
{
  nTotalO = nTotalX = 0;
  SendDlgItemMessage(hWndLog, ID_LOG_O, LB_RESETCONTENT, 0, 0);
  SendDlgItemMessage(hWndLog, ID_LOG_X, LB_RESETCONTENT, 0, 0);

  char* header = "points searched speed depth";
  SendDlgItemMessage(hWndLog, ID_LOG_O, LB_ADDSTRING, 0, (LPARAM)header);
  SendDlgItemMessage(hWndLog, ID_LOG_X, LB_ADDSTRING, 0, (LPARAM)header);
}
// -----------------------------------------------------------------------------
void WriteLog(int points, int nSearched, int speed, int depth, bool debug)
{
  char lpszStr[128];
  char lpszCap[128];

  if(terminateAI == 9) return;

  sprintf(lpszStr, "%6d%9d%6d%4d", points, nSearched, speed, depth);
  if (currPlayer == playerO)
    {
      nTotalO += nSearched;
      SendDlgItemMessage(hWndLog, ID_LOG_O, LB_ADDSTRING, 0, (LPARAM)lpszStr);
    }
  else
    {
      nTotalX += nSearched;
      SendDlgItemMessage(hWndLog, ID_LOG_X, LB_ADDSTRING, 0, (LPARAM)lpszStr);
    }
  sprintf(lpszCap, "%d : %d", nTotalO, nTotalX);
  SetWindowText(hWndLog, lpszCap);
}
// -----------------------------------------------------------------------------
long getTime()
{
  static LARGE_INTEGER freq;
  if(!freq.QuadPart){
    QueryPerformanceFrequency(&freq);
    if(!freq.QuadPart) return GetTickCount();
  }
  LARGE_INTEGER c;
  QueryPerformanceCounter(&c);
  return (long)(c.QuadPart * 1000 / freq.QuadPart);
}
// -----------------------------------------------------------------------------
