/**
This program demonstrates how to create a win32 GUI program
with a system tray icon. The project is created with Code::Blocks.

TODO:
  allow only one instance

documentation:
  http://www.codeguru.com/cpp/com-tech/shell/icons/comments.php/c1335/?thread=6517
  http://www.daniweb.com/blogs/entry3389.html
  http://forum.pcmweb.nl/archive/index.php/t-4733.html
*/

#include <windows.h>
#include "SystemTray.h"
#include "resource.h"

#define WM_ICONNOTIFY           (WM_USER + 101)     // notification for the systemtray icon


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "CodeBlocksWindowsApp";
char szTitle[ ] = "Demo program with systray icon";
char szToolTip[ ] = "Demo program - System tray icon";

CSystemTray sysTray;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
  HWND hwnd;               /* This is the handle for our window */
  MSG messages;            /* Here messages to the application are saved */
  WNDCLASSEX wincl;        /* Data structure for the windowclass */

  /* The Window structure */
  wincl.hInstance = hThisInstance;
  wincl.lpszClassName = szClassName;
  wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
  wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
  wincl.cbSize = sizeof (WNDCLASSEX);

  /* Use default icon and mouse-pointer */
  wincl.hIcon = LoadIcon(wincl.hInstance, MAKEINTRESOURCE(APPLICATION_ICON));
  wincl.hIconSm = LoadIcon (wincl.hInstance, MAKEINTRESOURCE(APPLICATION_ICON));
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL;                 /* No menu */
  wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;                      /* structure or the window instance */
  /* Use Windows's default colour as the background of the window */
  wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

  /* Register the window class, and if it fails quit the program */
  if (!RegisterClassEx (&wincl))
      return 0;

  /* The class is registered, let's create the program*/
  hwnd = CreateWindowEx (
         0,                   /* Extended possibilites for variation */
         szClassName,         /* Classname */
         szTitle,             /* Title Text */
         WS_OVERLAPPEDWINDOW, /* default window */
         CW_USEDEFAULT,       /* Windows decides the position */
         CW_USEDEFAULT,       /* where the window ends up on the screen */
         544,                 /* The programs width */
         375,                 /* and height in pixels */
         HWND_DESKTOP,        /* The window is a child-window to desktop */
         NULL,                /* No menu */
         hThisInstance,       /* Program Instance handler */
         NULL                 /* No Window Creation data */
         );

  /* Create a systray icon */
  UINT uCallbackMessage = WM_ICONNOTIFY;
  HICON icon = wincl.hIconSm;
  //CSystemTray sysTray = CSystemTray(hThisInstance, hwnd, uCallbackMessage, szToolTip, icon, 0);
  sysTray.Create(hThisInstance, hwnd, uCallbackMessage, szToolTip, icon, 0);
  sysTray.SetPopupMenuDefaultItem(IDM_LOGFILE, 0);
  sysTray.SetPopupMenu(SYSTRAY_MENU);

  /* Do NOT make the window visible on the screen */
  //ShowWindow (hwnd, nCmdShow);
  ShowWindow (hwnd, false);

  /* Run the message loop. It will run until GetMessage() returns 0 */
  while (GetMessage (&messages, NULL, 0, 0))
  {
    /* Translate virtual-key messages into character messages */
    TranslateMessage(&messages);
    /* Send message to WindowProcedure */
    DispatchMessage(&messages);
  }

  /* The program return-value is 0 - The value that PostQuitMessage() gave */
  return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)                  /* handle the messages */
  {
    case WM_DESTROY:
      PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
      break;

    case WM_COMMAND:
      // the wParam is the command message from the menu bar
      // so we are going to process it.
      switch ( wParam )
      {
        case IDM_LOGFILE:
          MessageBox( hwnd  ,  "Open logfile..." , "Logfile" , MB_OK );
          return 0;
        case IDM_SETTINGS:
          MessageBox ( hwnd  , "Show settings..." , "Settings" , MB_OK );
          return 0;
        case IDM_HELP:
          MessageBox ( hwnd  , "Help menu..." , "Help" , MB_OK );
          return 0;
        case IDM_EXIT:
          {
            int res = MessageBox ( hwnd,
                      "Are you sure you want to exit the program?" ,
                      "Exit program",
                      MB_YESNOCANCEL | MB_ICONEXCLAMATION) ;
            if (res == IDYES)
            {
              PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            }
          }
          return 0;
        default:
          MessageBox ( hwnd ,"Unknown menu item " , "Error" , MB_OK );
          return 0;
      }

    case WM_ICONNOTIFY:
      // Handle actions when user clicks on the systray icon
      return sysTray.OnTrayNotification(wParam, lParam);

    default:                      /* for messages that we don't deal with */
      return DefWindowProc (hwnd, message, wParam, lParam);
  }

  return 0;
}
