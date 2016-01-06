#include <time.h>
#include <stdio.h>
#include <windows.h>
#include "resource.h"
typedef void (WINAPI* HOOKDLL)();    // ��һ��dll����������ʽ������
HOOKDLL ins, uns;                    // ��һ��dll����������ʽ������
void    (WINAPI* HookDll[2])();      // ����ָ������ķ�ʽ�������Ա��ʹ�÷�ʽͬ����ins,uns
static const char* key_name[256] = { // ��֪��û���ֳɵ�ת������...
    "","LBUTTON","RBUTTON","CANCEL","MBUTTON","XBUTTON1","XBUTTON2","","BackSpace","Tab","","","Clear","Enter","","",
    "Shift","Ctrl","Alt","Pause","CapsLock","HANGUL","","JUNJA","FINAL","HANJA","","Esc","CONVERT","NONCONVERT","ACCEPT","MODECHANGE",
    "Space","PageUp","PageDown","End","Home","��","��","��","��","Select","Print","Execute","PrtScr","Insert","Delete","Help",
    "0","1","2","3","4","5","6","7","8","9","","","","","","",
    "","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O",
    "P","Q","R","S","T","U","V","W","X","Y","Z","LWin","RWin","APPS","","SLEEP",
    "0","1","2","3","4","5","6","7","8","9","*","+","SEPARATOR","-",".","��",
    "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","F13","F14","F15","F16",
    "F17","F18","F19","F20","F21","F22","F23","F24","","","","","","","","",
    "NumLock","ScrLock","","","","","","","","","","","","","","",
    "LShift","RShift","LCtrl","RCtrl","LAlt","RAlt","","","","","","","","","","",
    "","","","","","","","","","",";","=",",","-",".","/",
    "`","","","","","","","","","","","","","","","",
    "","","","","","","","","","","","[","\\","]","'","",
    "","","","","","","","","","","","","","","","",
    "","","","","","","","","","","","","","","","Fn",
};
bool      notCatch = false;
unsigned  keep[2];
HINSTANCE hInst;
HWND      CaphWnd;
HBRUSH    hbrBkgnd;
time_t    Time;
char      c[64];
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch (message){
        case KEY_RECEIVE: // �Զ��������Ϣ
            if(notCatch) break;
            //printf("|%08x|,|%08x|\n",wParam,lParam);
            Time = time(NULL);
            if(lParam & 0x80000000) { // ��31λΪ1
                if(lParam & 0x40000000) { // ��30λΪ1
                    PostMessage(CaphWnd, WM_KEYUP, wParam, lParam);
                    sprintf(c,"%s - Key  Up  [%s].\n",_strtime(ctime(&Time)),key_name[wParam & 0xff]);
                }
            } else { // ��31Ϊ0
                if(lParam & 0x40000000) { // ��30λΪ1
                    if(keep[0]==wParam && keep[1]==lParam) break;
                    { keep[0] = wParam; keep[1] = lParam; }
                    sprintf(c,"%s - Key Keep [%s].\n",_strtime(ctime(&Time)),key_name[wParam & 0xff]);
                } else { // ��30λΪ0
                    PostMessage(CaphWnd, WM_KEYDOWN, wParam, lParam);
                    sprintf(c,"%s - Key Down [%s].\n",_strtime(ctime(&Time)),key_name[wParam & 0xff]);
                }
            }
            //SetFocus(GetDlgItem(hWnd,IDC_EDIT)); // ����˵Ҫ���趨���㣬����ò�Ʋ���Ҳ����
            SendMessage(GetDlgItem(hWnd,IDC_EDIT), EM_SETSEL, -2, -1); // ��λ��ĩβ
            SendMessage(GetDlgItem(hWnd,IDC_EDIT), EM_REPLACESEL, 0, (LPARAM)(LPCTSTR)c);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)){
                case IDC_BUTTON:
                    if(notCatch) {
                        notCatch = false;
                        SetDlgItemText(hWnd, IDC_BUTTON, "ֹͣ��¼");
                    } else {
                        notCatch = true;
                        SetDlgItemText(hWnd, IDC_BUTTON, "��ʼ��¼");
                    }
                    break;
                case IDC_EDIT:
                    DestroyCaret(); // HideCaret(GetDlgItem(hWnd,IDC_EDIT));
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CTLCOLORSTATIC: // ֻ������ñ༭�ؼ�����WM_CTLCOLORSTATIC��Ϣ������WM_CTLCOLOREDIT��Ϣ
            if((HWND)lParam == GetDlgItem(hWnd, IDC_EDIT)) { // ���Ӧ�ó��������Ϣ�������뷵��һ����ˢ�ľ����ϵͳ����ˢ��Ϳˢ�༭�ؼ��ı�����
                SetTextColor((HDC)wParam, RGB(0,248,0));
                SetBkColor((HDC)wParam, BLACK_BRUSH);
                if(hbrBkgnd == NULL) hbrBkgnd = GetSysColorBrush(1); // ��CreateSolidBrush �� CreateBrushIndirect���������Ļ�ˢ��Ҫ�ͷ�,GetStockObject �� GetSysColorBrush�����ͷ�
                return (INT_PTR)hbrBkgnd;
            }
            break;
        case WM_DESTROY:
            uns();
            FreeLibrary(hInst);
            PostQuitMessage(0);
            break;
        default:
            ;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
bool LoadDllFunc(HINSTANCE hInstance) {
    hInstance = LoadLibrary("hook.dll");
    if(hInstance == NULL) {
        MessageBox(NULL, "Load DynamicLinkLibrary Failed.", "LoadLibrary()", MB_OK);
        return false;
    }
    ins = (HOOKDLL)GetProcAddress(hInstance, "_InstallLaunchEv@0");
    uns = (HOOKDLL)GetProcAddress(hInstance, "_UninstallLaunchEv@0");
    if(NULL == ins || NULL == uns) {
        MessageBox(NULL, "Find Function Address Failed.", "GetProcAddress()", MB_OK);
        FreeLibrary(hInstance);
        return false;
    }
    return true;
}
bool InitInstance(HINSTANCE hInstance, int nCmdShow){
    HWND hWnd = CreateDialog(hInstance, (LPCTSTR)IDD_MAIN, NULL, NULL);
    if(!hWnd) return false;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return true;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    WNDCLASSEX wc={sizeof(WNDCLASSEX),CS_CLASSDC,MainWndProc ,0L,DLGWINDOWEXTRA,hInstance,NULL,NULL,(HBRUSH)CreateSolidBrush(RGB(232,232,232)),NULL,"Main",NULL};
    RegisterClassEx(&wc);
    if(!InitInstance (hInstance, nCmdShow)) return false;
    if(!LoadDllFunc(hInst)) return false;
    CaphWnd = FindWindowEx(FindWindow("Notepad", NULL), NULL, "Edit", "");
#ifdef _DEBUGno
    AllocConsole();
    freopen("conin$" , "r+t", stdin);
    freopen("conout$", "w+t", stdout);
    freopen("conout$", "w+t", stderr);
#endif
    ins();
    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
