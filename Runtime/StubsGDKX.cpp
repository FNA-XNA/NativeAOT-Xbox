#pragma once

#include <windows.h>

// GDKX does not include this Win32 function referenced directly in the Runtime,
// so stub it out with the raw tick count times 10,000 (per TimerQueue.Windows.cs). -caleb
extern "C" BOOL __stdcall QueryUnbiasedInterruptTime(PULONGLONG UnbiasedTime)
{
	*UnbiasedTime = GetTickCount64() * 10000;
	return true;
}

extern "C" BOOL __stdcall ImpersonateLoggedOnUser(_In_ HANDLE hToken)
{
	return 0; // Always fail
}

extern "C" BOOL __stdcall RevertToSelf()
{
	return TRUE; // Always succeed
}

extern "C" BOOL __stdcall GetTokenInformation(
	_In_ HANDLE TokenHandle,
	_In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
	_Out_ LPVOID TokenInformation,
	_In_ DWORD TokenInformationLength,
	_Out_ PDWORD ReturnLength
)
{
	return FALSE; // Always fail
}

extern "C" BOOL __stdcall OpenThreadToken(
	_In_ HANDLE  ThreadHandle,
	_In_ DWORD   DesiredAccess,
	_In_ BOOL    OpenAsSelf,
	_Out_ PHANDLE TokenHandle
)
{
	return FALSE;
}

extern "C" BOOL __stdcall GetSystemTimes(
	PFILETIME lpIdleTime,
	PFILETIME lpKernelTime,
	PFILETIME lpUserTime
)
{
	return FALSE;
}

extern "C" BOOL GetThreadIOPendingFlag(
	HANDLE hThread,
	PBOOL  lpIOIsPending
)
{
	// Should be ok, since this is hardcoded in PortableThreadPool.Unix.cs
	return FALSE;
}

extern "C" BOOL SetThreadErrorMode(
	_In_  DWORD   dwNewMode,
	_Out_ LPDWORD lpOldMode
) {
	return FALSE;
}