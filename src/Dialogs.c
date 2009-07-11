/******************************************************************************
*
*
* Notepad2
*
* Dialogs.c
*   Notepad2 dialog boxes implementation
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2009
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/
#define _WIN32_WINNT 0x501
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <string.h>
#include "notepad2.h"
#include "scintilla.h"
#include "edit.h"
#include "helpers.h"
#include "dlapi.h"
#include "dialogs.h"
#include "resource.h"


extern HWND  hwndMain;
extern HWND  hwndEdit;
extern HINSTANCE g_hInstance;
extern LPMALLOC  g_lpMalloc;
extern BOOL bSkipUnicodeDetection;
extern BOOL bFixLineEndings;
extern BOOL bAutoStripBlanks;
extern WCHAR szCurFile[MAX_PATH+40];


//=============================================================================
//
//  MsgBox()
//
int MsgBox(int iType,UINT uIdMsg,...)
{

  WCHAR szText [256*2];
  WCHAR szBuf  [256*2];
  WCHAR szTitle[64];
  int iIcon = 0;
  va_list args;
  HWND hwnd;

  if (!GetString(uIdMsg,szBuf,COUNTOF(szBuf)))
    return(0);

  va_start(args, uIdMsg);
  wvsprintf(szText,szBuf,args);
  va_end(args);

  GetString(IDS_APPTITLE,szTitle,COUNTOF(szTitle));

  switch (iType) {
    case MBINFO: iIcon = MB_ICONEXCLAMATION; break;
    case MBWARN: iIcon = MB_ICONEXCLAMATION; break;
    case MBYESNO: iIcon = MB_ICONEXCLAMATION | MB_YESNO; break;
    case MBYESNOCANCEL: iIcon = MB_ICONEXCLAMATION | MB_YESNOCANCEL; break;
    case MBYESNOWARN: iIcon = MB_ICONEXCLAMATION | MB_YESNO; break;
    case MBOKCANCEL: iIcon = MB_ICONEXCLAMATION | MB_OKCANCEL; break;
  }

  if (!(hwnd = GetFocus()))
    hwnd = hwndMain;

  return MessageBoxEx(hwnd,
           szText,szTitle,
           MB_SETFOREGROUND | iIcon,
           MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT));

}


//=============================================================================
//
//  DisplayCmdLineHelp()
//
void DisplayCmdLineHelp()
{
  MSGBOXPARAMS mbp;

  WCHAR szTitle[32];
  WCHAR szText[1024];

  GetString(IDS_APPTITLE,szTitle,COUNTOF(szTitle));
  GetString(IDS_CMDLINEHELP,szText,COUNTOF(szText));

  mbp.cbSize = sizeof(MSGBOXPARAMS);
  mbp.hwndOwner = NULL;
  mbp.hInstance = g_hInstance;
  mbp.lpszText = szText;
  mbp.lpszCaption = szTitle;
  mbp.dwStyle = MB_OK | MB_USERICON | MB_SETFOREGROUND;
  mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND);
  mbp.dwContextHelpId = 0;
  mbp.lpfnMsgBoxCallback = NULL;
  mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL);

  MessageBoxIndirect(&mbp);
}


//=============================================================================
//
//  BFFCallBack()
//
int CALLBACK BFFCallBack(HWND hwnd,UINT umsg,LPARAM lParam,LPARAM lpData)
{
  if (umsg == BFFM_INITIALIZED)
    SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);

  return(0);

  lParam;
}


//=============================================================================
//
//  GetDirectory()
//
BOOL GetDirectory(HWND hwndParent,int iTitle,LPWSTR pszFolder,LPCWSTR pszBase,BOOL bNewDialogStyle)
{

  BROWSEINFO bi;
  LPITEMIDLIST pidl;
  //LPMALLOC lpMalloc;
  WCHAR szTitle[256];
  WCHAR szBase[MAX_PATH];
  BOOL fOk = FALSE;

  lstrcpy(szTitle,L"");
  GetString(iTitle,szTitle,COUNTOF(szTitle));

  if (!pszBase || !*pszBase)
    GetCurrentDirectory(MAX_PATH,szBase);
  else
    lstrcpy(szBase,pszBase);

  bi.hwndOwner = hwndParent;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszFolder;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS;
  if (bNewDialogStyle)
    bi.ulFlags |= BIF_NEWDIALOGSTYLE;
  bi.lpfn = &BFFCallBack;
  bi.lParam = (LPARAM)szBase;
  bi.iImage = 0;

  //if (SHGetMalloc(&lpMalloc) != NOERROR)
  //   return FALSE;

  pidl = SHBrowseForFolder(&bi);
  if (pidl)
  {
    SHGetPathFromIDList(pidl,pszFolder);

    g_lpMalloc->lpVtbl->Free(g_lpMalloc,pidl);

    fOk = TRUE;
  }
  //lpMalloc->lpVtbl->Release(lpMalloc);

  return fOk;

}


//=============================================================================
//
//  AboutDlgProc()
//
static const DWORD  dwVerMajor    = 3;
static const DWORD  dwVerMinor    = 1;
static const DWORD  dwBuildNumber = 21;
static const WCHAR* szRevision    = L"";
static const WCHAR* szExtra       = L"";
static const BOOL   bReleaseBuild = TRUE;

BOOL CALLBACK AboutDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static HFONT hFontTitle;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR  szVersion[64];
        WCHAR  szDate[64];
        WCHAR  szLink[256];
        WCHAR  szLinkCode[256];
        LOGFONT lf;

        if (bReleaseBuild) {
          wsprintf(szVersion,L"Notepad2 %u.%u.%0.2u%s",
            dwVerMajor,dwVerMinor,dwBuildNumber,szRevision);
          SetDlgItemText(hwnd,IDC_VERSION,szVersion);
        }
        else {
          MultiByteToWideChar(CP_ACP,0,__DATE__,-1,szDate,COUNTOF(szDate));
          wsprintf(szVersion,L"Notepad2 %u.%u.%0.2u%s%s %s",
            dwVerMajor,dwVerMinor,dwBuildNumber,szRevision,szExtra,szDate);
          SetDlgItemText(hwnd,IDC_VERSION,szVersion);
        }

        if (hFontTitle)
          DeleteObject(hFontTitle);

        if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd,IDC_VERSION,WM_GETFONT,0,0)))
          hFontTitle = GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hFontTitle,sizeof(LOGFONT),&lf);
        lf.lfWeight = FW_BOLD;
        hFontTitle = CreateFontIndirect(&lf);
        SendDlgItemMessage(hwnd,IDC_VERSION,WM_SETFONT,(WPARAM)hFontTitle,TRUE);

        if (GetDlgItem(hwnd,IDC_WEBPAGE) == NULL)
          ShowWindow(GetDlgItem(hwnd,IDC_WEBPAGE2),SW_SHOWNORMAL);
        else {
          GetDlgItemText(hwnd,IDC_WEBPAGE2,szLink,COUNTOF(szLink));
          wsprintf(szLinkCode,L"<A>%s</A>",szLink);
          SetDlgItemText(hwnd,IDC_WEBPAGE,szLinkCode);
        }

        if (GetDlgItem(hwnd,IDC_EMAIL) == NULL)
          ShowWindow(GetDlgItem(hwnd,IDC_EMAIL2),SW_SHOWNORMAL);
        else {
          GetDlgItemText(hwnd,IDC_EMAIL2,szLink,COUNTOF(szLink));
          wsprintf(szLinkCode,L"<A>%s</A>",szLink);
          SetDlgItemText(hwnd,IDC_EMAIL,szLinkCode);
        }

        CenterDlgInParent(hwnd);
      }
      return TRUE;

    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) {

          case NM_CLICK:
          case NM_RETURN:
            {
              if (pnmhdr->idFrom == IDC_WEBPAGE) {
                ShellExecute(hwnd,L"open",L"http://www.flos-freeware.ch",NULL,NULL,SW_SHOWNORMAL);
              }
              else if (pnmhdr->idFrom == IDC_EMAIL) {
                ShellExecute(hwnd,L"open",L"mailto:florian.balmer@gmail.com",NULL,NULL,SW_SHOWNORMAL);
              }
            }
            break;
        }
      }
      break;

    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog(hwnd,IDOK);
          break;
      }
      return TRUE;
  }
  return FALSE;
}



//=============================================================================
//
//  RunDlgProc()
//
BOOL CALLBACK RunDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        MakeBitmapButton(hwnd,IDC_SEARCHEXE,g_hInstance,IDB_OPEN);

        SendDlgItemMessage(hwnd,IDC_COMMANDLINE,EM_LIMITTEXT,MAX_PATH - 1,0);
        SetDlgItemText(hwnd,IDC_COMMANDLINE,(LPCWSTR)lParam);
        SHAutoComplete(GetDlgItem(hwnd,IDC_COMMANDLINE),SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_SEARCHEXE);
      return FALSE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_SEARCHEXE:
          {
            WCHAR szArgs[MAX_PATH];
            WCHAR szArg2[MAX_PATH];
            WCHAR szFile[MAX_PATH * 2];
            WCHAR szFilter[256];
            OPENFILENAME ofn;
            ZeroMemory(&ofn,sizeof(OPENFILENAME));

            GetDlgItemText(hwnd,IDC_COMMANDLINE,szArgs,COUNTOF(szArgs));
            ExpandEnvironmentStringsEx(szArgs,COUNTOF(szArgs));
            ExtractFirstArgument(szArgs,szFile,szArg2);

            GetString(IDS_FILTER_EXE,szFilter,COUNTOF(szFilter));
            PrepareFilterStr(szFilter);

            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = szFilter;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = COUNTOF(szFile);
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                      | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            if (GetOpenFileName(&ofn)) {
              PathQuoteSpaces(szFile);
              if (lstrlen(szArg2))
              {
                lstrcat(szFile,L" ");
                lstrcat(szFile,szArg2);
              }
              SetDlgItemText(hwnd,IDC_COMMANDLINE,szFile);
            }

            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;


        case IDC_COMMANDLINE:
          {
            BOOL bEnableOK = FALSE;
            WCHAR args[MAX_PATH];

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,args,MAX_PATH))
              if (ExtractFirstArgument(args,args,NULL))
                if (lstrlen(args))
                  bEnableOK = TRUE;

            EnableWindow(GetDlgItem(hwnd,IDOK),bEnableOK);
          }
          break;


        case IDOK:
          {
            WCHAR arg1[MAX_PATH];
            WCHAR arg2[MAX_PATH];
            SHELLEXECUTEINFO sei;
            WCHAR wchDirectory[MAX_PATH] = L"";

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,arg1,MAX_PATH))
            {
              BOOL bQuickExit = FALSE;

              ExpandEnvironmentStringsEx(arg1,COUNTOF(arg1));
              ExtractFirstArgument(arg1,arg1,arg2);

              if (lstrcmpi(arg1,L"notepad2") == 0 ||
                  lstrcmpi(arg1,L"notepad2.exe") == 0) {
                GetModuleFileName(NULL,arg1,COUNTOF(arg1));
                bQuickExit = TRUE;
              }

              if (lstrlen(szCurFile)) {
                lstrcpy(wchDirectory,szCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));

              sei.cbSize = sizeof(SHELLEXECUTEINFO);
              sei.fMask = 0;
              sei.hwnd = hwnd;
              sei.lpVerb = NULL;
              sei.lpFile = arg1;
              sei.lpParameters = arg2;
              sei.lpDirectory = wchDirectory;
              sei.nShow = SW_SHOWNORMAL;

              if (bQuickExit) {
                EndDialog(hwnd,IDOK);
                ShellExecuteEx(&sei);
              }

              else {

                if (ShellExecuteEx(&sei))
                  EndDialog(hwnd,IDOK);

                else
                  PostMessage(hwnd,WM_NEXTDLGCTL,
                    (WPARAM)(GetDlgItem(hwnd,IDC_COMMANDLINE)),1);
              }
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  RunDlg()
//
void RunDlg(HWND hwnd,LPCWSTR lpstrDefault)
{

  ThemedDialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_RUN),
    hwnd,RunDlgProc,(LPARAM)lpstrDefault);

}


//=============================================================================
//
//  OpenWithDlgProc()
//
extern WCHAR tchOpenWithDir[MAX_PATH];
extern int  flagNoFadeHidden;

extern int cxOpenWithDlg;
extern int cyOpenWithDlg;

BOOL CALLBACK OpenWithDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        RECT rc;
        WCHAR tch[MAX_PATH];
        int cGrip;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (cxOpenWithDlg < (rc.right-rc.left))
          cxOpenWithDlg = rc.right-rc.left;
        if (cyOpenWithDlg < (rc.bottom-rc.top))
          cyOpenWithDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,rc.left,rc.top,cxOpenWithDlg,cyOpenWithDlg,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,DWLP_USER,lParam);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_OPENWITHDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),tchOpenWithDir,DL_ALLOBJECTS,NULL,FALSE,flagNoFadeHidden,DS_NAME,FALSE);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,g_hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;

        DirList_Destroy(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);

        GetWindowRect(hwnd,&rc);
        cxOpenWithDlg = rc.right-rc.left;
        cyOpenWithDlg = rc.bottom-rc.top;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_GETOPENWITHDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_GETOPENWITHDIR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_OPENWITHDESCR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_OPENWITHDESCR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDESCR),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                EnableWindow(GetDlgItem(hwnd,IDOK),(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_OPENWITHDIR)))
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GETOPENWITHDIR:
          {
            if (GetDirectory(hwnd,IDS_OPENWITH,tchOpenWithDir,tchOpenWithDir,TRUE))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),tchOpenWithDir,DL_ALLOBJECTS,NULL,FALSE,flagNoFadeHidden,DS_NAME,FALSE);
              DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,FALSE);
              ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_OPENWITHDIR)),1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),(-1),lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd,IDOK);
            else
              MessageBeep(0);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  OpenWithDlg()
//
BOOL OpenWithDlg(HWND hwnd,LPCWSTR lpstrFile)
{

  DLITEM dliOpenWith;
  dliOpenWith.mask = DLI_FILENAME;

  if (IDOK == ThemedDialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_OPENWITH),
                             hwnd,OpenWithDlgProc,(LPARAM)&dliOpenWith))
  {
    SHELLEXECUTEINFO sei;
    WCHAR szParam[MAX_PATH];
    WCHAR wchDirectory[MAX_PATH] = L"";

    if (lstrlen(szCurFile)) {
      lstrcpy(wchDirectory,szCurFile);
      PathRemoveFileSpec(wchDirectory);
    }

    ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = 0;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = dliOpenWith.szFileName;
    sei.lpParameters = szParam;
    sei.lpDirectory = wchDirectory;
    sei.nShow = SW_SHOWNORMAL;

    // resolve links and get short path name
    if (!(PathIsLnkFile(lpstrFile) && PathGetLnkPath(lpstrFile,szParam,COUNTOF(szParam))))
      lstrcpy(szParam,lpstrFile);
    //GetShortPathName(szParam,szParam,sizeof(WCHAR)*COUNTOF(szParam));
    PathQuoteSpaces(szParam);

    ShellExecuteEx(&sei);

    return(TRUE);
  }

  return(FALSE);

}


//=============================================================================
//
//  FavoritesDlgProc()
//
extern WCHAR tchFavoritesDir[MAX_PATH];
//extern int  flagNoFadeHidden;

extern int cxFavoritesDlg;
extern int cyFavoritesDlg;

BOOL CALLBACK FavoritesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        RECT rc;
        WCHAR tch[MAX_PATH];
        int cGrip;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (cxFavoritesDlg < (rc.right-rc.left))
          cxFavoritesDlg = rc.right-rc.left;
        if (cyFavoritesDlg < (rc.bottom-rc.top))
          cyFavoritesDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,rc.left,rc.top,cxFavoritesDlg,cyFavoritesDlg,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,DWLP_USER,lParam);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_FAVORITESDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_FAVORITESDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),tchFavoritesDir,DL_ALLOBJECTS,NULL,FALSE,flagNoFadeHidden,DS_NAME,FALSE);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETFAVORITESDIR,g_hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;

        DirList_Destroy(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        DeleteBitmapButton(hwnd,IDC_GETFAVORITESDIR);

        GetWindowRect(hwnd,&rc);
        cxFavoritesDlg = rc.right-rc.left;
        cyFavoritesDlg = rc.bottom-rc.top;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_FAVORITESDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_FAVORITESDIR),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_FAVORITESDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_GETFAVORITESDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_GETFAVORITESDIR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_GETFAVORITESDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_FAVORITESDESCR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_FAVORITESDESCR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_FAVORITESDESCR),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_FAVORITESDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_FAVORITESDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                EnableWindow(GetDlgItem(hwnd,IDOK),(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_FAVORITESDIR)))
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GETFAVORITESDIR:
          {
            if (GetDirectory(hwnd,IDS_FAVORITES,tchFavoritesDir,tchFavoritesDir,TRUE))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),tchFavoritesDir,DL_ALLOBJECTS,NULL,FALSE,flagNoFadeHidden,DS_NAME,FALSE);
              DirList_StartIconThread(GetDlgItem(hwnd,IDC_FAVORITESDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,FALSE);
              ListView_SetItemState(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_FAVORITESDIR)),1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd,IDC_FAVORITESDIR),(-1),lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd,IDOK);
            else
              MessageBeep(0);
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  FavoritesDlg()
//
BOOL FavoritesDlg(HWND hwnd,LPWSTR lpstrFile)
{

  DLITEM dliFavorite;
  dliFavorite.mask = DLI_FILENAME;

  if (IDOK == ThemedDialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_FAVORITES),
                             hwnd,FavoritesDlgProc,(LPARAM)&dliFavorite))
  {
    lstrcpyn(lpstrFile,dliFavorite.szFileName,MAX_PATH);
    return(TRUE);
  }

  return(FALSE);

}


//=============================================================================
//
//  AddToFavDlgProc()
//
//  Controls: 100 Edit
//
BOOL CALLBACK AddToFavDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    WCHAR *pszName;

    case WM_INITDIALOG:
      pszName = (LPWSTR)lParam;
      SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)pszName);

      SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,MAX_PATH-1,0);
      SetDlgItemText(hwnd,100,pszName);

      CenterDlgInParent(hwnd);
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case 100:
            EnableWindow(GetDlgItem(hwnd,IDOK),
              GetWindowTextLength(GetDlgItem(hwnd,100)));
          break;


        case IDOK:
          pszName = (LPWSTR)GetWindowLongPtr(hwnd,DWLP_USER);
          GetDlgItemText(hwnd,100,pszName,
            MAX_PATH-1);
          EndDialog(hwnd,IDOK);
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  AddToFavDlg()
//
BOOL AddToFavDlg(HWND hwnd,LPCWSTR lpszName,LPCWSTR lpszTarget)
{

  int iResult;

  WCHAR pszName[MAX_PATH];
  lstrcpy(pszName,lpszName);

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(IDD_ADDTOFAV),
              hwnd,
              AddToFavDlgProc,(LPARAM)pszName);

  if (iResult == IDOK)
  {
    if (!PathCreateFavLnk(pszName,lpszTarget,tchFavoritesDir)) {
      MsgBox(MBWARN,IDS_FAV_FAILURE);
      return FALSE;
    }

    else {
      MsgBox(MBINFO,IDS_FAV_SUCCESS);
      return TRUE;
    }
  }

  else
    return FALSE;

}


//=============================================================================
//
//  FileMRUDlgProc()
//
//
extern LPMRULIST pFileMRU;
extern BOOL bSaveRecentFiles;
extern int  cxFileMRUDlg;
extern int  cyFileMRUDlg;
extern int  flagNoFadeHidden;

typedef struct tagIconThreadInfo
{
  HWND hwnd;                 // HWND of ListView Control
  HANDLE hExitThread;        // Flag is set when Icon Thread should terminate
  HANDLE hTerminatedThread;  // Flag is set when Icon Thread has terminated

} ICONTHREADINFO, *LPICONTHREADINFO;

DWORD WINAPI FileMRUIconThread(LPVOID lpParam) {

  HWND hwnd;
  LPICONTHREADINFO lpit;
  LV_ITEM lvi;
  WCHAR tch[MAX_PATH];
  SHFILEINFO shfi;
  DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;
  DWORD dwAttr  = 0;
  int iItem = 0;
  int iMaxItem;

  lpit = (LPICONTHREADINFO)lpParam;
  ResetEvent(lpit->hTerminatedThread);

  hwnd = lpit->hwnd;
  iMaxItem = ListView_GetItemCount(hwnd);

  CoInitialize(NULL);

  ZeroMemory(&lvi,sizeof(LV_ITEM));

  while (iItem < iMaxItem && WaitForSingleObject(lpit->hExitThread,0) != WAIT_OBJECT_0) {

    lvi.mask = LVIF_TEXT;
    lvi.pszText = tch;
    lvi.cchTextMax = COUNTOF(tch);
    lvi.iItem = iItem;
    if (ListView_GetItem(hwnd,&lvi)) {

      if (!PathFileExists(tch)) {
        dwFlags |= SHGFI_USEFILEATTRIBUTES;
        dwAttr = FILE_ATTRIBUTE_NORMAL;
        shfi.dwAttributes = 0;
        SHGetFileInfo(PathFindFileName(tch),dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
      }

      else {
        shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
        SHGetFileInfo(tch,dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
      }

      lvi.mask = LVIF_IMAGE;
      lvi.iImage = shfi.iIcon;
      lvi.stateMask = 0;
      lvi.state = 0;

      if (shfi.dwAttributes & SFGAO_LINK) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(2);
      }

      if (shfi.dwAttributes & SFGAO_SHARE) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(1);
      }

      dwAttr = GetFileAttributes(tch);

      if (!flagNoFadeHidden &&
          dwAttr != INVALID_FILE_ATTRIBUTES &&
          dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_CUT;
        lvi.state |= LVIS_CUT;
      }

      lvi.iSubItem = 0;
      ListView_SetItem(hwnd,&lvi);
    }
    iItem++;
  }

  CoUninitialize();

  SetEvent(lpit->hTerminatedThread);
  ExitThread(0);
  return(0);
}

BOOL CALLBACK FileMRUDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        RECT rc;
        WCHAR tch[MAX_PATH];
        int cGrip;
        SHFILEINFO shfi;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        LPICONTHREADINFO lpit = (LPVOID)GlobalAlloc(GPTR,sizeof(ICONTHREADINFO));
        SetProp(hwnd,L"it",(HANDLE)lpit);
        lpit->hwnd = GetDlgItem(hwnd,IDC_FILEMRU);
        lpit->hExitThread = CreateEvent(NULL,TRUE,FALSE,NULL);
        lpit->hTerminatedThread = CreateEvent(NULL,TRUE,TRUE,NULL);

        SetWindowLongPtr(hwnd,DWLP_USER,lParam);

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (cxFileMRUDlg < (rc.right-rc.left))
          cxFileMRUDlg = rc.right-rc.left;
        if (cyFileMRUDlg < (rc.bottom-rc.top))
          cyFileMRUDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,0,0,cxFileMRUDlg,cyFileMRUDlg,SWP_NOZORDER|SWP_NOMOVE);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        ListView_SetImageList(GetDlgItem(hwnd,IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\",0,&shfi,sizeof(SHFILEINFO),SHGFI_SMALLICON | SHGFI_SYSICONINDEX),
          LVSIL_SMALL);

        ListView_SetImageList(GetDlgItem(hwnd,IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\",0,&shfi,sizeof(SHFILEINFO),SHGFI_LARGEICON | SHGFI_SYSICONINDEX),
          LVSIL_NORMAL);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_FILEMRU));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_FILEMRU),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_FILEMRU),0,&lvc);

        // Update view
        SendMessage(hwnd,WM_COMMAND,MAKELONG(0x00A0,1),0);

        if (bSaveRecentFiles)
          CheckDlgButton(hwnd,IDC_SAVEMRU,BST_CHECKED);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;

        LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd,L"it");
        SetEvent(lpit->hExitThread);
        while (WaitForSingleObject(lpit->hTerminatedThread,0) != WAIT_OBJECT_0) {
          MSG msg;
          if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }
        CloseHandle(lpit->hExitThread);
        CloseHandle(lpit->hTerminatedThread);
        RemoveProp(hwnd,L"it");
        GlobalFree(lpit);

        GetWindowRect(hwnd,&rc);
        cxFileMRUDlg = rc.right-rc.left;
        cyFileMRUDlg = rc.bottom-rc.top;

        bSaveRecentFiles = (IsDlgButtonChecked(hwnd,IDC_SAVEMRU)) ? 1 : 0;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_FILEMRU),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_FILEMRU),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_FILEMRU),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_FILEMRU),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_SAVEMRU),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_SAVEMRU),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_SAVEMRU),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_NOTIFY: {
      if (((LPNMHDR)(lParam))->idFrom == IDC_FILEMRU) {

      switch (((LPNMHDR)(lParam))->code) {

        case NM_DBLCLK:
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
          break;


        case LVN_GETDISPINFO: {
            /*
            LV_DISPINFO *lpdi = (LPVOID)lParam;

            if (lpdi->item.mask & LVIF_IMAGE) {

              WCHAR tch[MAX_PATH];
              LV_ITEM lvi;
              SHFILEINFO shfi;
              DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;
              DWORD dwAttr  = 0;

              ZeroMemory(&lvi,sizeof(LV_ITEM));

              lvi.mask = LVIF_TEXT;
              lvi.pszText = tch;
              lvi.cchTextMax = COUNTOF(tch);
              lvi.iItem = lpdi->item.iItem;

              ListView_GetItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);

              if (!PathFileExists(tch)) {
                dwFlags |= SHGFI_USEFILEATTRIBUTES;
                dwAttr = FILE_ATTRIBUTE_NORMAL;
                shfi.dwAttributes = 0;
                SHGetFileInfo(PathFindFileName(tch),dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
              }

              else {
                shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
                SHGetFileInfo(tch,dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
              }

              lpdi->item.iImage = shfi.iIcon;
              lpdi->item.mask |= LVIF_DI_SETITEM;

              lpdi->item.stateMask = 0;
              lpdi->item.state = 0;

              if (shfi.dwAttributes & SFGAO_LINK) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_OVERLAYMASK;
                lpdi->item.state |= INDEXTOOVERLAYMASK(2);
              }

              if (shfi.dwAttributes & SFGAO_SHARE) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_OVERLAYMASK;
                lpdi->item.state |= INDEXTOOVERLAYMASK(1);
              }

              dwAttr = GetFileAttributes(tch);

              if (!flagNoFadeHidden &&
                  dwAttr != INVALID_FILE_ATTRIBUTES &&
                  dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_CUT;
                lpdi->item.state |= LVIS_CUT;
              }
            }
            */
          }
          break;


        case LVN_ITEMCHANGED:
        case LVN_DELETEITEM:
            EnableWindow(GetDlgItem(hwnd,IDOK),ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_FILEMRU)));
            break;
          }
        }
      }

      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case 0x00A0:
          {
            int i;
            WCHAR tch[MAX_PATH];
            LV_ITEM lvi;
            SHFILEINFO shfi;

            DWORD dwtid;
            LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd,L"it");

            SetEvent(lpit->hExitThread);
            while (WaitForSingleObject(lpit->hTerminatedThread,0) != WAIT_OBJECT_0) {
              MSG msg;
              if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
              }
            }
            ResetEvent(lpit->hExitThread);
            SetEvent(lpit->hTerminatedThread);

            ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_FILEMRU));

            ZeroMemory(&lvi,sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;

            SHGetFileInfo(L"Icon",FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
              SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
            lvi.iImage = shfi.iIcon;

            for (i = 0; i < MRU_Enum(pFileMRU,0,NULL,0); i++) {
              MRU_Enum(pFileMRU,i,tch,COUNTOF(tch));
              PathAbsoluteFromApp(tch,NULL,0,TRUE);
              //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_ADDSTRING,0,(LPARAM)tch); }
              //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_SETCARETINDEX,0,FALSE);
              lvi.iItem = i;
              lvi.pszText = tch;
              ListView_InsertItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);
            }

            ListView_SetItemState(GetDlgItem(hwnd,IDC_FILEMRU),0,LVIS_FOCUSED,LVIS_FOCUSED);
            ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_FILEMRU),0,LVSCW_AUTOSIZE_USEHEADER);

            CreateThread(NULL,0,FileMRUIconThread,(LPVOID)lpit,0,&dwtid);
          }
          break;

        case IDC_FILEMRU:
          break;


        case IDOK:
          {
            WCHAR tch[MAX_PATH];
            //int  iItem;

            //if ((iItem = SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_GETCURSEL,0,0)) != LB_ERR)
            if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_FILEMRU)))
            {
              //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_GETTEXT,(WPARAM)iItem,(LPARAM)tch);
              LV_ITEM lvi;
              ZeroMemory(&lvi,sizeof(LV_ITEM));

              lvi.mask = LVIF_TEXT;
              lvi.pszText = tch;
              lvi.cchTextMax = COUNTOF(tch);
              lvi.iItem = ListView_GetNextItem(GetDlgItem(hwnd,IDC_FILEMRU),-1,LVNI_ALL | LVNI_SELECTED);

              ListView_GetItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);

              PathUnquoteSpaces(tch);

              if (!PathFileExists(tch)) {

                // Ask...
                if (IDYES == MsgBox(MBYESNO,IDS_ERR_MRUDLG)) {

                    MRU_Delete(pFileMRU,lvi.iItem);

                    //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_DELETESTRING,(WPARAM)iItem,0);
                    //ListView_DeleteItem(GetDlgItem(hwnd,IDC_FILEMRU),lvi.iItem);
                    // must use IDM_VIEW_REFRESH, index might change...
                    SendMessage(hwnd,WM_COMMAND,MAKELONG(0x00A0,1),0);

                    //EnableWindow(GetDlgItem(hwnd,IDOK),
                    //  (LB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,LB_GETCURSEL,0,0)));

                    EnableWindow(GetDlgItem(hwnd,IDOK),
                      ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_FILEMRU)));
                }
              }

              else {
                lstrcpy((LPWSTR)GetWindowLongPtr(hwnd,DWLP_USER),tch);
                EndDialog(hwnd,IDOK);
              }
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  FileMRUDlg()
//
//
BOOL FileMRUDlg(HWND hwnd,LPWSTR lpstrFile)
{

  if (IDOK == ThemedDialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_FILEMRU),
                hwnd,FileMRUDlgProc,(LPARAM)lpstrFile))
    return TRUE;
  else
    return FALSE;

}


//=============================================================================
//
//  ColumnWrapDlgProc()
//
//  Controls: 100 Edit
//
BOOL CALLBACK ColumnWrapDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int *piNumber;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {

        piNumber = (int*)lParam;

        SetDlgItemInt(hwnd,100,*piNumber,FALSE);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,15,0);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated;

          int iNewNumber = GetDlgItemInt(hwnd,100,&fTranslated,FALSE);

          if (fTranslated)
          {
            *piNumber = iNewNumber;

            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,100)),1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  ColumnWrapDlg()
//
BOOL ColumnWrapDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              ColumnWrapDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  WordWrapSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Combo
//
extern BOOL bShowWordWrapSymbols;
extern int  iWordWrapSymbols;

BOOL CALLBACK WordWrapSettingsDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int *piNumber;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {

        WCHAR tch[512];
        WCHAR *p1, *p2;

        piNumber = (int*)lParam;

        SetDlgItemInt(hwnd,100,*piNumber,FALSE);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,15,0);

        GetDlgItemText(hwnd,102,tch,COUNTOF(tch));
        lstrcat(tch,L"|");
        p1 = tch;
        while (p2 = StrChr(p1,L'|')) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,101,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
        }

        SendDlgItemMessage(hwnd,101,CB_SETCURSEL,(WPARAM)(bShowWordWrapSymbols) ? iWordWrapSymbols+1 : 0,0);
        SendDlgItemMessage(hwnd,101,CB_SETEXTENDEDUI,TRUE,0);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated;
          int  iSel;

          int iNewNumber = GetDlgItemInt(hwnd,100,&fTranslated,FALSE);

          if (fTranslated)
          {
            *piNumber = iNewNumber;

            iSel = SendDlgItemMessage(hwnd,101,CB_GETCURSEL,0,0);
            if (iSel > 0) {
              bShowWordWrapSymbols = TRUE;
              iWordWrapSymbols = iSel-1;
            }
            else {
              bShowWordWrapSymbols = FALSE;
            }

            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,100)),1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  WordWrapSettingsDlg()
//
BOOL WordWrapSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              WordWrapSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  LongLineSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Radio1
//            102 Radio2
//
extern int iLongLineMode;

BOOL CALLBACK LongLineSettingsDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int *piNumber;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {

        piNumber = (int*)lParam;

        SetDlgItemInt(hwnd,100,*piNumber,FALSE);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,15,0);

        if (iLongLineMode == EDGE_LINE)
          CheckRadioButton(hwnd,101,102,101);
        else
          CheckRadioButton(hwnd,101,102,102);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated;

          int iNewNumber = GetDlgItemInt(hwnd,100,&fTranslated,FALSE);

          if (fTranslated)
          {
            *piNumber = iNewNumber;

            iLongLineMode = (IsDlgButtonChecked(hwnd,101)) ? EDGE_LINE : EDGE_BACKGROUND;

            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,100)),1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  LongLineSettingsDlg()
//
BOOL LongLineSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              LongLineSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  TabSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Edit
//            102 Check
//
extern int iTabWidth;
extern int iIndentWidth;
extern BOOL bTabsAsSpaces;

BOOL CALLBACK TabSettingsDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {

        SetDlgItemInt(hwnd,100,iTabWidth,FALSE);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,15,0);

        SetDlgItemInt(hwnd,101,iIndentWidth,FALSE);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,15,0);

        if (bTabsAsSpaces)
          CheckDlgButton(hwnd,102,BST_CHECKED);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated1,fTranslated2;

          int iNewTabWidth = GetDlgItemInt(hwnd,100,&fTranslated1,FALSE);
          int iNewIndentWidth = GetDlgItemInt(hwnd,101,&fTranslated2,FALSE);

          if (fTranslated1 && fTranslated2)
          {
            iTabWidth = iNewTabWidth;
            iIndentWidth = iNewIndentWidth;

            bTabsAsSpaces = (IsDlgButtonChecked(hwnd,102)) ? TRUE : FALSE;

            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,(fTranslated1) ? 101 : 100)),1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  TabSettingsDlg()
//
BOOL TabSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              TabSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  SelectEncodingDlgProc()
//
//  Controls: 100 Combo
//            IDC_CPINFO
//
BOOL CALLBACK SelectEncodingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piOption;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR tch[512], tchFmt[64];
        WCHAR *p1, *p2;
        CPINFOEX cpi;

        piOption = (int*)lParam;

        GetCPInfoEx(GetACP(),0,&cpi);
        GetDlgItemText(hwnd,101,tchFmt,COUNTOF(tchFmt));
        wsprintf(tch,tchFmt,cpi.CodePageName);
        lstrcat(tch,L"|");
        p1 = tch;
        while (p2 = StrChr(p1,L'|')) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,100,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
        }

        SendDlgItemMessage(hwnd,100,CB_SETCURSEL,(WPARAM)*piOption,0);
        SendDlgItemMessage(hwnd,100,CB_SETEXTENDEDUI,TRUE,0);

        if (bSkipUnicodeDetection)
          CheckDlgButton(hwnd,IDC_NOUNICODEDETECTION,BST_CHECKED);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piOption = SendDlgItemMessage(hwnd,100,CB_GETCURSEL,0,0);
            bSkipUnicodeDetection = (IsDlgButtonChecked(hwnd,IDC_NOUNICODEDETECTION) == BST_CHECKED) ? 1 : 0;
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  SelectEncodingDlg()
//
BOOL SelectEncodingDlg(HWND hwnd,int *iOption)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(IDD_ENCODING),
              hwnd,
              SelectEncodingDlgProc,
              (LPARAM)iOption);

  return (iResult == IDOK) ? TRUE : FALSE;

}

//=============================================================================
//
//  SelectLineEndingDlgProc()
//
//  Controls: 100 Combo
//            IDC_CONSISTENTEOLS
//            IDC_AUTOSTRIPBLANKS
//
BOOL CALLBACK SelectLineEndingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piOption;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR tch[512];
        WCHAR *p1, *p2;

        piOption = (int*)lParam;

        // Load options
        GetDlgItemText(hwnd,101,tch,COUNTOF(tch));
        lstrcat(tch,L"|");
        p1 = tch;
        while (p2 = StrChr(p1,L'|')) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,100,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
        }

        SendDlgItemMessage(hwnd,100,CB_SETCURSEL,(WPARAM)*piOption,0);
        SendDlgItemMessage(hwnd,100,CB_SETEXTENDEDUI,TRUE,0);

        if (bFixLineEndings)
          CheckDlgButton(hwnd,IDC_CONSISTENTEOLS,BST_CHECKED);

        if (bAutoStripBlanks)
          CheckDlgButton(hwnd,IDC_AUTOSTRIPBLANKS, BST_CHECKED);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piOption = SendDlgItemMessage(hwnd,100,CB_GETCURSEL,0,0);
            bFixLineEndings = (IsDlgButtonChecked(hwnd,IDC_CONSISTENTEOLS) == BST_CHECKED) ? 1 : 0;
            bAutoStripBlanks = (IsDlgButtonChecked(hwnd,IDC_AUTOSTRIPBLANKS) == BST_CHECKED) ? 1 : 0;
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  SelectLineEndingDlg()
//
BOOL SelectLineEndingDlg(HWND hwnd,int *iOption)
{

  int iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCE(IDD_EOLMODE),
              hwnd,
              SelectLineEndingDlgProc,
              (LPARAM)iOption);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  InfoBoxDlgProc()
//
//
typedef struct _infobox {
  LPWSTR lpstrMessage;
  LPWSTR lpstrSetting;
} INFOBOX, *LPINFOBOX;

BOOL CALLBACK InfoBoxDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  LPINFOBOX lpib;

  switch(umsg)
  {
    case WM_INITDIALOG:
      lpib = (LPINFOBOX)lParam;
      SetWindowLongPtr(hwnd,DWLP_USER,lParam);
      SendDlgItemMessage(hwnd,IDC_INFOBOXICON,STM_SETICON,
        (WPARAM)LoadIcon(NULL,IDI_EXCLAMATION),0);
      SetDlgItemText(hwnd,IDC_INFOBOXTEXT,lpib->lpstrMessage);
      LocalFree(lpib->lpstrMessage);
      CenterDlgInParent(hwnd);
      return TRUE;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
        case IDYES:
        case IDNO:
          lpib = (LPINFOBOX)GetWindowLongPtr(hwnd,DWLP_USER);
          if (IsDlgButtonChecked(hwnd,IDC_INFOBOXCHECK))
            IniSetInt(L"Suppressed Messages",lpib->lpstrSetting,1);
          EndDialog(hwnd,LOWORD(wParam));
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  InfoBox()
//
//
int InfoBox(int iType,LPCWSTR lpstrSetting,int uidMessage,...)
{

  HWND hwnd;
  int idDlg = IDD_INFOBOX;
  INFOBOX ib;
  WCHAR wchFormat[512];
  va_list args;

  if (IniGetInt(L"Suppressed Messages",lpstrSetting,0))
    return (iType == MBYESNO) ? IDYES : IDOK;

  if (!GetString(uidMessage,wchFormat,COUNTOF(wchFormat)))
    return(-1);

  ib.lpstrMessage = LocalAlloc(LPTR,1024 * sizeof(WCHAR));
  va_start(args, uidMessage);
  wvsprintf(ib.lpstrMessage,wchFormat,args);
  va_end(args);

  ib.lpstrSetting = (LPWSTR)lpstrSetting;

  if (iType == MBYESNO)
    idDlg = IDD_INFOBOX2;
  else if (iType == MBOKCANCEL)
    idDlg = IDD_INFOBOX3;

  if (!(hwnd = GetFocus()))
    hwnd = hwndMain;

  MessageBeep(MB_ICONEXCLAMATION);

  return ThemedDialogBoxParam(
           g_hInstance,
           MAKEINTRESOURCE(idDlg),
           hwnd,
           InfoBoxDlgProc,
           (LPARAM)&ib);

}



//  End of Dialogs.c
