/* FIXME: This file should not need to exist.
 * This feature flag: `--feature : System.Diagnostics.Tracing.EventSource.IsSupported = false`
 * is supposed to automatically trim out the use of this method.
 * But for some reason it doesn't, so here we are...
 */

#include <gcenv.h>

EXTERN_C NATIVEAOT_API void __cdecl NativeRuntimeEventSource_LogExceptionThrown(const WCHAR* exceptionTypeName, const WCHAR* exceptionMessage, void* faultingIP, HRESULT hresult)
{
}