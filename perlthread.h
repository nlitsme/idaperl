#ifndef _PERLTHREAD_H__
#define _PERLTHREAD_H__
/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * 'perlthread' is an attempt to run perl in a seperate thread,
 * the main ida-thread will just display a 'cancel' button,
 * the perlthread is interruptable.
 *
 *
 */
#include "perlinterp.h"
#include <string>


CRITICAL_SECTION g_uilock;
class xclusive {
    public:
    xclusive() {
        InitializeCriticalSection(&g_uilock);
    }
    ~xclusive() {
        DeleteCriticalSection(&g_uilock);
    }
};

xclusive  g_initlock;
#if 1
void xmsg(const char *msg, ...)
{
    va_list ap;
    EnterCriticalSection(&g_uilock);
    FILE *f= fopen("msg.log", "a+");
    va_start(ap, msg);
    vfprintf(f, msg, ap);
    va_end(ap);
    fclose(f);
    LeaveCriticalSection(&g_uilock);
}
#else
#define xmsg while(0)
#endif
class perlthread {
public:
    perlthread(Perl* perl, const std::string& code) : _h(NULL), _perl(perl), _code(code), _sema(NULL), _finished(false), _cancelled(false), _hDlg(NULL)
    {
        InitializeCriticalSection(&_lock);
        _sema= CreateSemaphore(0,0,LONG_MAX,0);
        if (_sema==NULL)
            xmsg("error creating semaphore: %08lx\n", GetLastError());
        DWORD id;
        _h= CreateThread(0,0,reinterpret_cast<LPTHREAD_START_ROUTINE>(&perlthread::staticproc), this, 0, &id);
        if (_h==NULL)
            xmsg("error creating thread: %08lx\n", GetLastError());
        xmsg("%08lx: perlthread created\n", GetTickCount());
    }
    ~perlthread()
    {
        xmsg("%08lx: perlthread destructor\n", GetTickCount());
        EnterCriticalSection(&_lock);
        bool docancel= !_finished;
        LeaveCriticalSection(&_lock);
        if (docancel) {
            xmsg("%08lx: perlthread destructor: cancelling\n", GetTickCount());
            cancel();
            //msg("%08lx: perlthread destructor: waiting for thread\n", GetTickCount());
            //WaitForSingleObject(_h,INFINITE);
        }
        closedialog();

        CloseHandle(_h);
        _h= NULL;
        CloseHandle(_sema);
        _sema= NULL;
        DeleteCriticalSection(&_lock);
        xmsg("%08lx: perlthread destructor end\n", GetTickCount());
    }
    static DWORD WINAPI staticproc(LPVOID param)
    {
        return reinterpret_cast<perlthread*>(param)->threadproc();
    }
    DWORD threadproc()
    {
        DWORD t0= GetTickCount();
        xmsg("%08lx: executing perl code in thread %08lx\n", GetTickCount(), GetCurrentThreadId());
        char errbuf[1024];
        if (!_perl->exec(_code.c_str(), errbuf, 1024))
            msg("IDAPERL ERROR: %s\n", errbuf);
        xmsg("%08lx: perlcode done executing - %d msec\n", GetTickCount(), GetTickCount()-t0);
        EnterCriticalSection(&_lock);
        _finished= true;
        ReleaseSemaphore(_sema, 1, NULL);
        LeaveCriticalSection(&_lock);
        xmsg("%08lx: perlthread finished - signalled\n", GetTickCount());
        return 0;
    }
    void cancel()
    {
        xmsg("%08lx: cancelling\n", GetTickCount());
        _perl->cancel();
    }
    bool wait(DWORD timeout)
    {
        xmsg("%08lx: perlthread: waiting\n", GetTickCount());
        // wait until timeout ( true )
        // or until thread has finished ( false )
        DWORD t0= GetTickCount();
        EnterCriticalSection(&_lock);
        while (!_finished && !_cancelled && (GetTickCount()-t0) < timeout) {
            LeaveCriticalSection(&_lock);
            int rc= MsgWaitForMultipleObjects(1, &_sema, false, 1000, 0);
            EnterCriticalSection(&_lock);
            if (rc==WAIT_OBJECT_0+1) {
                // MsgWaitForMultipleObjects
                //   returns WAIT_OBJECT_0+nCount, when a systemevent happened during the wait
                MSG msg;
                while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                    xmsg("systemevent %08lx(%08lx %08lx)\n", msg.message, msg.lParam, msg.wParam);
                    DispatchMessage(&msg);
                }
            }
            else if (rc!=WAIT_OBJECT_0 && rc!=WAIT_TIMEOUT) {
                xmsg("wait error: rc=%08lx le=%08lx\n", rc, GetLastError());
                break;
            }
        }
        bool result= !_finished && !_cancelled;
        LeaveCriticalSection(&_lock);
        xmsg("%08lx: perlthread: wait result: f=%d,c=%d ->%d,  %d msec\n", GetTickCount(), _finished, _cancelled, result, GetTickCount()-t0);

        return result;
    }
    bool cancelwait()
    {
        xmsg("%08lx: perlthread::cancelwait\n", GetTickCount());
        // show cancel button
        // wait until cancelled ( true)
        // or until thread has finished ( false )
        EnterCriticalSection(&_lock);
        if (!_finished && !_cancelled) {
            createdialog();
            xmsg("%08lx: dialog created: %08lx\n", GetTickCount(), _hDlg);
        }
        MSG wmsg;
        while (!_finished && !_cancelled && GetMessage(&wmsg, 0, 0, 0)>0)
        {
            TranslateMessage(&wmsg);
            DispatchMessage(&wmsg);

            LeaveCriticalSection(&_lock);
            int rc= WaitForSingleObject(_sema, 0);
            EnterCriticalSection(&_lock);
            if (rc!=WAIT_OBJECT_0 && rc!=WAIT_TIMEOUT) {
                xmsg("wait error: rc=%08lx le=%08lx\n", rc, GetLastError());
                break;
            }
        }
        bool result= _cancelled;

        LeaveCriticalSection(&_lock);
        xmsg("%08lx: cancelwait: f=%d,c=%d -> %d\n", GetTickCount(), _finished, _cancelled, result);
        return result;
    }
    INT_PTR dlgproc(UINT wMsg, WPARAM wParam, LPARAM lParam)
    {
        if (wMsg==WM_COMMAND && wParam==IDCANCEL)  {
            xmsg("%08lx: dlgproc: cancel\n", GetTickCount());
            EndDialog(_hDlg, IDCANCEL);
            EnterCriticalSection(&_lock);
            _cancelled= true;
            ReleaseSemaphore(_sema, 1, NULL);
            LeaveCriticalSection(&_lock);
        }
        return 0;
    }

private:
    HANDLE _h;
    Perl *_perl;
    const std::string& _code;
    HANDLE _sema;
    CRITICAL_SECTION _lock;
    bool _finished;
    bool _cancelled;
    HWND _hDlg;

    static INT_PTR FAR PASCAL staticdlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
    {
        if (wMsg==WM_INITDIALOG) {
            SetWindowLong(hDlg, GWL_USERDATA, lParam);
        }

        perlthread* self= reinterpret_cast<perlthread*>(GetWindowLong(hDlg, GWL_USERDATA));
        if (self==NULL) {
            xmsg("%08lx: unknown window: %08lx\n", GetTickCount(), hDlg);
            return 0;
        }
        return self->dlgproc(wMsg, wParam, lParam);
    }
    bool createdialog()
    {
        BYTE *buf= new BYTE[1024];
        WCHAR *name;
        WORD *cdata;
        DLGITEMTEMPLATE *item;
        DLGTEMPLATE *dlg= (DLGTEMPLATE *)buf;
        buf += sizeof(DLGTEMPLATE);
        dlg->style=WS_CAPTION|DS_CENTER;
        dlg->dwExtendedStyle=0;
        dlg->cdit=0;
        dlg->x=CW_USEDEFAULT; dlg->y=CW_USEDEFAULT; dlg->cx=100; dlg->cy=30;

        // menu
        name= (WCHAR*)buf;
        buf += sizeof(WCHAR);
        *name= 0;

        // class
        name= (WCHAR*)buf;
        buf += sizeof(WCHAR);
        *name= 0;

        // title
        name= (WCHAR*)buf;
        buf += sizeof(WCHAR);
        *name= 0;

        // align
        if ((DWORD)buf&3) {
            buf += 4-((DWORD)buf&3);
        }
        dlg->cdit++;
        item= (DLGITEMTEMPLATE *)buf;
        buf += sizeof(DLGITEMTEMPLATE);
        item->style=BS_PUSHBUTTON|BS_CENTER|WS_VISIBLE;
        item->dwExtendedStyle=0;
        item->x=30; item->y=15; item->cx=40; item->cy=10;
        item->id=2;

        // item class
        name= (WCHAR*)buf;
        buf += 2*sizeof(WCHAR);
        name[0]= 0xffff;
        name[1]= 0x80;  // button

        // item title
        name = (WCHAR*)buf;
        wcscpy(name, L"Cancel");
        buf += 7*sizeof(WCHAR);

        // creation data
        cdata= (WORD*)buf;
        buf += sizeof(WORD);
        *cdata= 0;

        // align
        if ((DWORD)buf&3) {
            buf += 4-((DWORD)buf&3);
        }
        dlg->cdit++;
        item= (DLGITEMTEMPLATE *)buf;
        buf += sizeof(DLGITEMTEMPLATE);
        item->style=SS_CENTER|WS_VISIBLE;
        item->dwExtendedStyle=0;
        item->x=5; item->y=4; item->cx=90; item->cy=10;
        item->id=1;

        // item class
        name= (WCHAR*)buf;
        buf += 2*sizeof(WCHAR);
        name[0]= 0xffff;
        name[1]= 0x82;  // static

        // item title
        name = (WCHAR*)buf;
        wcscpy(name, L"Script running");
        buf += 15*sizeof(WCHAR);

        // creation data
        cdata= (WORD*)buf;
        buf += sizeof(WORD);
        *cdata= 0;

        _hDlg= CreateDialogIndirectParam(0, dlg, 0, reinterpret_cast<DLGPROC>(&staticdlgproc), reinterpret_cast<LPARAM>(this));
        if (_hDlg==NULL)
            msg("error creating dialog: %08lx\n", GetLastError());

        ShowWindow(_hDlg, SW_SHOW);

        return _hDlg!=NULL;
    }
    void closedialog()
    {
        if (_hDlg)
            DestroyWindow(_hDlg);
        _hDlg= NULL;
    }

};
#endif
