#include <windows.h>
#include "..\loader\resource.h"
HHOOK      hook;
HWND       hWnd;
HINSTANCE  hDllInstance;
LRESULT CALLBACK LauncherHook(int nCode,WPARAM wParam,LPARAM lParam) {
    if(nCode==HC_ACTION) {
        if(NULL == hWnd) MessageBox(NULL, "\'Hook\' not found.", "FindWindow()", MB_OK);
	PostMessage(hWnd, KEY_RECEIVE, wParam, lParam);
    }
    return CallNextHookEx(hook,nCode,wParam,lParam);   //返回下一个钩子句柄
}
extern "C" __declspec(dllexport) void WINAPI InstallLaunchEv() {
    hook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)LauncherHook, hDllInstance, NULL);
    if(NULL == hook) MessageBox(NULL, "SetWindowsHookEx()", "ERROR", MB_OK);
}
extern "C" __declspec(dllexport) void WINAPI UninstallLaunchEv() {
    UnhookWindowsHookEx(hook);
}
int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved ) {
    hDllInstance = hInstance;
    hWnd = FindWindow(NULL, "Hook");
    return true;
}
