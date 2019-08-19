/*
http://www.codeguru.com/cpp/com-tech/shell/icons/comments.php/c1335/?thread=6517
*/

#ifndef _INCLUDED_SYSTEMTRAY_H_
#define _INCLUDED_SYSTEMTRAY_H_

#include <windows.h>
#include <Shellapi.h>

#include <vector>
#include <map>

/////////////////////////////////////////////////////////////////////////////
// CSystemTray window

class CSystemTray
{
  // Construction/destruction
public:
  CSystemTray();
  CSystemTray(HINSTANCE hInst, HWND hWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID);
  virtual ~CSystemTray();

  // Operations
public:
  BOOL Enabled() { return m_bEnabled; }
  BOOL Visible() { return !m_bHidden; }

  // Create the tray icon
  BOOL Create(HINSTANCE hInst, HWND hWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID);

  // Change or retrieve the Tooltip text
  BOOL SetTooltipText(LPCTSTR pszTooltipText);
  BOOL SetTooltipText(UINT nID);
  LPCTSTR GetTooltipText() const { return m_bEnabled?m_tnd.szTip:NULL; };
# if(_WIN32_IE>=0x0500)
  BOOL SetInfoText(LPCTSTR pszInfo, LPCTSTR pszInfoTitle="", DWORD dwInfoFlags=NIIF_INFO, UINT uTimeout=0);
  BOOL SetInfoText(UINT nIDInfo, UINT nIDInfoTitle=0, DWORD dwInfoFlags=NIIF_INFO, UINT uTimeout=0);
# endif

  // Change or retrieve the icon displayed
  BOOL SetIcon(HICON hIcon);
  BOOL SetIcon(LPCTSTR lpszIconName);
  BOOL SetIcon(UINT nIDResource);
  BOOL SetStandardIcon(LPCTSTR lpIconName);
  BOOL SetStandardIcon(UINT nIDResource);
  HICON GetIcon() const { return (m_bEnabled)?m_tnd.hIcon:NULL; };
  BOOL HideIcon();
  BOOL ShowIcon();
  BOOL RemoveIcon();
  void MoveToRight();

  // For icon animation
  BOOL SetIconList(UINT uFirstIconID, UINT uLastIconID);
  BOOL SetIconList(HICON* pHIconList, UINT nNumIcons);
  BOOL Animate(UINT nDelayMilliSeconds, int nNumSeconds = -1);
  BOOL StepAnimation();
  BOOL StopAnimation();

  // popup menu functions
  BOOL SetPopupMenu(UINT nIDResource) { return SetPopupMenu(MAKEINTRESOURCE(nIDResource)); }
  BOOL SetPopupMenu(LPCSTR lpMenuName);

  HMENU GetPopupMenu() { return m_SubMenu; }
  void GetPopupMenuDefaultItem(UINT& uItem, BOOL& bByPos);
  BOOL SetPopupMenuDefaultItem(UINT uItem, BOOL bByPos);

  // Change or retrieve the window to send notification messages to
  BOOL SetNotificationWnd(HWND hWnd);
  HWND GetNotificationWnd() const { return m_tnd.hWnd; };

  // Default handler for tray notification message
  virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

  // Implementation
protected:
  void Initialise();
  static VOID CALLBACK TimerProcRaw(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, DWORD dwTime);
  HWND m_hWnd;

  BOOL m_bEnabled; // does O/S support tray icon?
  BOOL m_bHidden; // Has the icon been hidden?
  NOTIFYICONDATA m_tnd;

  HMENU m_Menu, m_SubMenu;
  HINSTANCE m_hInst;

  std::vector<HICON> m_IconList;
  static UINT m_nIDEvent;
  UINT m_uIDTimer;
  int m_nCurrentIcon;
  DWORD m_dwStartTime;
  int m_nAnimationPeriod;
  HICON m_hSavedIcon;
  UINT m_DefaultMenuItemID;
  BOOL m_DefaultMenuItemByPos;
};

#endif

