// ProcessLister.cpp
// Copyright (C) 2018 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
// This file is public domain software.

#include "targetver.h"
#include "MProcessListBox.hpp"
#include "MResizable.hpp"
#include "resource.h"

#define MAX_WINDOW_TEXT 32

struct MProcessInfoEx : MProcessInfo
{
    HWND m_hwnd;
    MString m_window_text;
    MString m_fullpath;
    virtual void from_entry(const PROCESSENTRY32& entry)
    {
        MProcessInfo::from_entry(entry);
        m_hwnd = get_window();
        m_window_text = MWindowBase::GetWindowText(m_hwnd);
        m_fullpath = get_full_path();
    }
    virtual MString get_text() const
    {
        MString window_text = m_window_text;
        if (window_text.size() >= MAX_WINDOW_TEXT)
        {
            window_text.resize(MAX_WINDOW_TEXT);
            window_text += TEXT("...");
        }
        TCHAR szText[MAX_PATH * 2];
        StringCbPrintf(szText, sizeof(szText), TEXT("PID %08X hwnd %p %s %s"),
            th32ProcessID, m_hwnd, window_text.c_str(), m_fullpath.c_str());
        return szText;
    }
};


class MMainDlg : public MDialogBase
{
public:
    HINSTANCE   m_hInst;        // the instance handle
    HICON       m_hIcon;        // the main icon
    HICON       m_hIconSm;      // the small icon
    MProcessListBox     m_lst1;
    DWORD   m_pid;
    MResizable m_resizable;

    MMainDlg(INT argc, TCHAR **targv, HINSTANCE hInst)
        : MDialogBase(IDD_MAIN), m_hInst(hInst),
          m_hIcon(NULL), m_hIconSm(NULL)
    {
        m_pid = 0;
    }

    virtual ~MMainDlg()
    {
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        SendMessageDx(WM_SETICON, ICON_BIG, LPARAM(m_hIcon));
        SendMessageDx(WM_SETICON, ICON_SMALL, LPARAM(m_hIconSm));

        SubclassChildDx(m_lst1, lst1);
        m_lst1.refresh<MProcessInfoEx>();

        m_resizable.OnParentCreate(hwnd);

        m_resizable.SetLayoutAnchor(lst1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
        m_resizable.SetLayoutAnchor(psh1, mzcLA_BOTTOM_LEFT);
        m_resizable.SetLayoutAnchor(IDOK, mzcLA_BOTTOM_RIGHT);
        m_resizable.SetLayoutAnchor(IDCANCEL, mzcLA_BOTTOM_RIGHT);

        return TRUE;
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case IDOK:
            m_lst1.get_selected_pid(m_pid);
            EndDialog(IDOK);
            break;
        case IDCANCEL:
            EndDialog(IDCANCEL);
            break;
        case psh1:
            m_lst1.refresh<MProcessInfoEx>();
            break;
        }
    }

    void OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
        m_resizable.OnSize();
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
        default:
            return DefaultProcDx();
        }
    }

    BOOL StartDx(INT nCmdShow)
    {
        m_hIcon = LoadIconDx(IDI_MAIN);
        m_hIconSm = LoadSmallIconDx(IDI_MAIN);

        return TRUE;
    }

    INT RunDx()
    {
        if (IDOK == DialogBoxDx(NULL))
        {
            TCHAR szText[64];
            StringCbPrintf(szText, sizeof(szText), TEXT("pid: 0x%08lX"), m_pid);
            MsgBoxDx(szText, TEXT("Selected Process"), MB_ICONINFORMATION);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
// Win32 main function

extern "C"
INT APIENTRY _tWinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    INT         nCmdShow)
{
    int ret = -1;

    {
        MMainDlg app(__argc, __targv, hInstance);

        ::InitCommonControls();

        if (app.StartDx(nCmdShow))
        {
            ret = app.RunDx();
        }
    }

#if (WINVER >= 0x0500)
    HANDLE hProcess = GetCurrentProcess();
    DebugPrintDx(TEXT("Count of GDI objects: %ld\n"),
                 GetGuiResources(hProcess, GR_GDIOBJECTS));
    DebugPrintDx(TEXT("Count of USER objects: %ld\n"),
                 GetGuiResources(hProcess, GR_USEROBJECTS));
#endif

#if defined(_MSC_VER) && !defined(NDEBUG)
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
