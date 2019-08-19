#include <windows.h>
#include <assert.h>

#include <tchar.h>
#include "SystemTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

CSystemTray::CSystemTray()
{
  Initialise();
}

CSystemTray::CSystemTray(HINSTANCE hInst, HWND hWnd, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID)
{
  Initialise();
  Create(hInst, hWnd, uCallbackMessage, szToolTip, icon, uID);
}

CSystemTray::~CSystemTray()
{
  if(m_Menu) DestroyMenu(m_Menu);
  if(m_SubMenu) DestroyMenu(m_SubMenu);
  RemoveIcon();
  m_IconList.clear();
}

void CSystemTray::Initialise()
{
  memset(&m_tnd, 0, sizeof(m_tnd));
  m_bEnabled=FALSE;
  m_bHidden=FALSE;
  m_uIDTimer=0;
  m_hSavedIcon=NULL;
  m_DefaultMenuItemID=0;
  m_DefaultMenuItemByPos=TRUE;
  m_hWnd=NULL;
  m_Menu=NULL;
  m_SubMenu=NULL;
  m_hInst=NULL;
}

BOOL CSystemTray::Create(HINSTANCE hInst, HWND hWnd, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID)
{
  m_bEnabled=(GetVersion()&0xff)>=4; // this is only for Windows 95 (or higher)
  if(!m_bEnabled) return FALSE;

  // some sanity checks
  assert(hInst);
  assert(IsWindow(hWnd));
  assert(uCallbackMessage>=WM_USER);
  assert(szToolTip);
# if(_WIN32_IE<0x0500)
  assert(_tcslen(szToolTip)<=64);
# else
  assert(_tcslen(szToolTip)<=128);
# endif

  // remember handles
  m_hInst=hInst;
  m_hWnd=hWnd;

  // load up the NOTIFYICONDATA structure
  m_tnd.cbSize=sizeof(NOTIFYICONDATA);
  m_tnd.hWnd=hWnd;
  m_tnd.uID=uID;
  m_tnd.hIcon=icon;
  m_tnd.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
  m_tnd.uCallbackMessage=uCallbackMessage;
  _tcscpy(m_tnd.szTip, szToolTip);

  // load the popup menu
  if(uID)
  {
    m_Menu=LoadMenu(hInst, MAKEINTRESOURCE(uID));
    if(!m_Menu) return FALSE;
    m_SubMenu=GetSubMenu(m_Menu, 0);
    if(!m_SubMenu) return FALSE;
  } // if(uID)

  // set the tray icon
  m_bEnabled=Shell_NotifyIcon(NIM_ADD, &m_tnd);
  if(!m_bEnabled) Initialise();
  return m_bEnabled;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

void CSystemTray::MoveToRight()
{
  HideIcon();
  ShowIcon();
}

BOOL CSystemTray::RemoveIcon()
{
  if(!m_bEnabled) return TRUE;
  m_tnd.uFlags=0;
  m_bEnabled=!Shell_NotifyIcon(NIM_DELETE, &m_tnd);
  return !m_bEnabled;
}

BOOL CSystemTray::HideIcon()
{
  if(m_bEnabled && !m_bHidden)
  {
    m_tnd.uFlags=NIF_ICON;
    m_bHidden=Shell_NotifyIcon (NIM_DELETE, &m_tnd);
    return m_bHidden;
  } // if(m_bEnabled && !m_bHidden)
  return TRUE;
}

BOOL CSystemTray::ShowIcon()
{
  if(m_bEnabled && m_bHidden)
  {
    m_tnd.uFlags=NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_bHidden=!Shell_NotifyIcon(NIM_ADD, &m_tnd);
    return !m_bHidden;
  } // if(m_bEnabled && m_bHidden)
  return TRUE;
}

BOOL CSystemTray::SetIcon(HICON hIcon)
{
  if(!m_bEnabled) return FALSE;
  m_tnd.uFlags=NIF_ICON;
  m_tnd.hIcon=hIcon;
  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName)
{
  HICON hIcon=LoadIcon(m_hInst, lpszIconName);
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetIcon(UINT nIDResource)
{
  HICON hIcon=LoadIcon(m_hInst, MAKEINTRESOURCE(nIDResource));
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName)
{
  HICON hIcon=LoadIcon(NULL, lpIconName);
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
  HICON hIcon=LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID)
{
  if(uFirstIconID>uLastIconID) return FALSE;

  UINT uIconArraySize=uLastIconID-uFirstIconID+1;
  m_IconList.clear();
  try
  {
    for(UINT i=uFirstIconID; i<=uLastIconID; ++i) m_IconList.push_back(LoadIcon(m_hInst, MAKEINTRESOURCE(i)));
  }
  catch (...)
  {
    return FALSE;
  }
  return TRUE;
}

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
  m_IconList.clear();
  try
  {
    for (UINT i=0; i<=nNumIcons; ++i) m_IconList.push_back(pHIconList[i]);
  }
  catch (...)
  {
    return FALSE;
  }

  return TRUE;
}

BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/)
{
  StopAnimation();
  m_nCurrentIcon=0;
  m_dwStartTime=GetTickCount();
  m_nAnimationPeriod=nNumSeconds;
  m_hSavedIcon=GetIcon();

  // setup a timer for the animation (the "this" pointer is passed in as "event ID" so we can use member
  // variables in TimerFunc() via a reinterpret_cast in the static CALLBACK TimerFuncRaw()
  m_uIDTimer=SetTimer(m_hWnd, (UINT_PTR)this, nDelayMilliSeconds, TimerProcRaw);
  return (m_uIDTimer!=0);
}

BOOL CSystemTray::StepAnimation()
{

  if(m_IconList.empty()) return FALSE;
  m_nCurrentIcon=(m_nCurrentIcon+1)%m_IconList.size();
  return SetIcon(m_IconList[m_nCurrentIcon]);
}

BOOL CSystemTray::StopAnimation()
{
  BOOL bResult=FALSE;
  if(m_uIDTimer)
  {
    bResult=KillTimer(m_hWnd, m_uIDTimer);
    m_uIDTimer=0;
  } // if(m_uIDTimer)
  if(m_hSavedIcon) SetIcon(m_hSavedIcon);                                       // relod the saved icon
  m_hSavedIcon=NULL;

  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip)
{
  assert(pszTip);
# if(_WIN32_IE<0x0500)
  assert(_tcslen(pszTip)<=64);
# else
  assert(_tcslen(pszTip)<=128);
# endif

  if(!m_bEnabled) return FALSE;
  m_tnd.uFlags=NIF_TIP;
  _tcscpy(m_tnd.szTip, pszTip);
  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetTooltipText(UINT nID)
{
# if(_WIN32_IE<0x0500)
  TCHAR strText[64];
# else
  TCHAR strText[128];
# endif
  if(!LoadString(m_hInst, nID, strText, sizeof(strText))) return FALSE;
  return SetTooltipText(strText);
}

# if(_WIN32_IE>=0x0500)
BOOL CSystemTray::SetInfoText(LPCTSTR pszInfo, LPCTSTR pszInfoTitle, DWORD dwInfoFlags, UINT uTimeout)
{
  assert(pszInfo);
  assert(pszInfoTitle);

  assert(_tcslen(pszInfo)<=256);
  assert(_tcslen(pszInfoTitle)<=64);

  if(!m_bEnabled) return FALSE;
  m_tnd.uFlags=NIF_INFO;
  _tcscpy(m_tnd.szInfo, pszInfo);
  _tcscpy(m_tnd.szInfoTitle, pszInfoTitle);
  m_tnd.dwInfoFlags=dwInfoFlags;
  m_tnd.uTimeout=uTimeout;
  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetInfoText(UINT nIDInfo, UINT nIDInfoTitle, DWORD dwInfoFlags, UINT uTimeout)
{
  TCHAR strInfo[256];
  TCHAR strInfoTitle[64]="";
  if(!LoadString(m_hInst, nIDInfo, strInfo, sizeof(strInfo))) return FALSE;
  if(nIDInfoTitle && !LoadString(m_hInst, nIDInfoTitle, strInfoTitle, sizeof(strInfoTitle))) return FALSE;
  return SetInfoText(strInfo, strInfoTitle, dwInfoFlags, uTimeout);
}

# endif

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

BOOL CSystemTray::SetNotificationWnd(HWND hWnd)
{
  if(!m_bEnabled) return FALSE;
  assert(IsWindow(hWnd)); // Make sure Notification window is valid

  m_tnd.hWnd=hWnd;
  m_tnd.uFlags=0;
  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

BOOL CSystemTray::SetPopupMenu(LPCSTR lpMenuName)
{
  if(!m_Menu)    { DestroyMenu(m_Menu);    m_Menu=NULL; }
  if(!m_SubMenu) { DestroyMenu(m_SubMenu); m_SubMenu=NULL; }
  m_Menu=LoadMenu(m_hInst, lpMenuName);
  if(!m_Menu) return FALSE;
  m_SubMenu=GetSubMenu(m_Menu, 0);
  if(!m_SubMenu) return FALSE;
  return TRUE;
}

BOOL CSystemTray::SetPopupMenuDefaultItem(UINT uItem, BOOL bByPos)
{
  if((m_DefaultMenuItemID==uItem) && (m_DefaultMenuItemByPos==bByPos)) return TRUE;

  m_DefaultMenuItemID=uItem;
  m_DefaultMenuItemByPos=bByPos;

  if(!m_SubMenu) return FALSE;
  return SetMenuDefaultItem(m_SubMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);
}

void CSystemTray::GetPopupMenuDefaultItem(UINT& uItem, BOOL& bByPos)
{
  uItem=m_DefaultMenuItemID;
  bByPos=m_DefaultMenuItemByPos;
}


VOID CALLBACK CSystemTray::TimerProcRaw(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  reinterpret_cast< CSystemTray* >(idEvent)->TimerProc(hwnd, uMsg, dwTime);
}

VOID CALLBACK CSystemTray::TimerProc(HWND hwnd, UINT uMsg, DWORD dwTime)
{
  DWORD dwTimeDiff=dwTime-m_dwStartTime;                                        // dwTime contains GetTickCount() - calculate ms that have passed since start of animation
  if(m_nAnimationPeriod>0 && dwTimeDiff/1000L>(DWORD)m_nAnimationPeriod)        // animation expired?
  {
    StopAnimation();                                                            // stop it
    return;
  } // if(m_nAnimationPeriod>0 && dwTimeDiff/1000L>(DWORD)m_nAnimationPeriod)
  StepAnimation();                                                              // still active: step it
}

LRESULT CSystemTray::OnTrayNotification(UINT wParam, LONG lParam)
{
  if(wParam!=m_tnd.uID) return 0L;                                              // return quickly if its not for this tray icon
  if(LOWORD(lParam)==WM_RBUTTONUP)                                              // clicking with right button brings up a context menu
  {
    if(!m_SubMenu) return 0;                                                    // no menu there? bail out!
    SetMenuDefaultItem(m_SubMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos); // make chosen menu item the default (bold font)

    // Display and track the popup menu
    POINT Point;
    GetCursorPos(&Point);
    SetForegroundWindow(m_hWnd);
    if(!::TrackPopupMenu(m_SubMenu, 0, Point.x, Point.y, 0, m_hWnd, NULL)) return 0;
    PostMessage(m_hWnd, WM_NULL, 0, 0);                                         // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
  } // if(LOWORD(lParam)==WM_RBUTTONUP)
  else if(LOWORD(lParam)==WM_LBUTTONUP)                                     // double click received, the default action is to execute default menu item
  {
    SetForegroundWindow(m_hWnd);
    UINT uItem;
    if(m_DefaultMenuItemByPos)
    {
      if(!m_SubMenu) return 0;                                                  // no menu there? Useless to continue.
      uItem=GetMenuItemID(m_SubMenu, m_DefaultMenuItemID);
    } // if(m_DefaultMenuItemByPos)
    else uItem=m_DefaultMenuItemID;
    SendMessage(m_hWnd, WM_COMMAND, uItem, 0);                                  // send the command to the handling HWND
  } // else if(LOWORD(lParam)==WM_LBUTTONDBLCLK)

  return 1;
}

