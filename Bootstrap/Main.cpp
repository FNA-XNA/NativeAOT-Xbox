// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include <stdint.h>
#include <Windows.h>
#include <gxdk.h>
#include "xmem.h"

//
// This is the mechanism whereby multiple linked modules contribute their global data for initialization at
// startup of the application.
//
// ILC creates sections in the output obj file to mark the beginning and end of merged global data.
// It defines sentinel symbols that are used to get the addresses of the start and end of global data
// at runtime. The section names are platform-specific to match platform-specific linker conventions.
//
#if defined(_MSC_VER)

#pragma section(".modules$A", read)
#pragma section(".modules$Z", read)
extern "C" __declspec(allocate(".modules$A")) void* __modules_a[];
extern "C" __declspec(allocate(".modules$Z")) void* __modules_z[];

__declspec(allocate(".modules$A")) void* __modules_a[] = { nullptr };
__declspec(allocate(".modules$Z")) void* __modules_z[] = { nullptr };

//
// Each obj file compiled from managed code has a .modules$I section containing a pointer to its ReadyToRun
// data (which points at eager class constructors, frozen strings, etc).
//
// The #pragma ... /merge directive folds the book-end sections and all .modules$I sections from all input
// obj files into .rdata in alphabetical order.
//
#pragma comment(linker, "/merge:.modules=.rdata")

//
// Unboxing stubs need to be merged, folded and sorted. They are delimited by two special sections (.unbox$A
// and .unbox$Z). All unboxing stubs are in .unbox$M sections.
//
#pragma comment(linker, "/merge:.unbox=.text")

char _bookend_a;
char _bookend_z;

//
// Generate bookends for the managed code section.
// We give them unique bodies to prevent folding.
//

#pragma code_seg(".managedcode$A")
void* __managedcode_a() { return &_bookend_a; }
#pragma code_seg(".managedcode$Z")
void* __managedcode_z() { return &_bookend_z; }
#pragma code_seg()

//
// Generate bookends for the unboxing stub section.
// We give them unique bodies to prevent folding.
//

#pragma code_seg(".unbox$A")
void* __unbox_a() { return &_bookend_a; }
#pragma code_seg(".unbox$Z")
void* __unbox_z() { return &_bookend_z; }
#pragma code_seg()

#endif // _MSC_VER

extern "C" bool RhInitialize();
extern "C" void RhSetRuntimeInitializationCallback(int (*fPtr)());

extern "C" bool RhRegisterOSModule(void* pModule,
    void* pvManagedCodeStartRange, uint32_t cbManagedCodeRange,
    void* pvUnboxingStubsStartRange, uint32_t cbUnboxingStubsRange,
    void** pClasslibFunctions, uint32_t nClasslibFunctions);

extern "C" void* PalGetModuleHandleFromPointer(void* pointer);

extern "C" void GetRuntimeException();
extern "C" void FailFast();
extern "C" void AppendExceptionStackFrame();
extern "C" void GetSystemArrayEEType();
extern "C" void OnFirstChanceException();
extern "C" void OnUnhandledException();
extern "C" void IDynamicCastableIsInterfaceImplemented();
extern "C" void IDynamicCastableGetInterfaceImplementation();

typedef void(*pfn)();

static const pfn c_classlibFunctions[] = {
    &GetRuntimeException,
    &FailFast,
    nullptr, // &UnhandledExceptionHandler,
    &AppendExceptionStackFrame,
    nullptr, // &CheckStaticClassConstruction,
    &GetSystemArrayEEType,
    &OnFirstChanceException,
    &OnUnhandledException,
    &IDynamicCastableIsInterfaceImplemented,
    &IDynamicCastableGetInterfaceImplementation,
};

#ifndef _countof
#define _countof(_array) (sizeof(_array)/sizeof(_array[0]))
#endif

extern "C" void InitializeModules(void* osModule, void** modules, int count, void** pClasslibFunctions, int nClasslibFunctions);

#define NATIVEAOT_ENTRYPOINT __managed__Main
extern "C" int __managed__Main(int argc, wchar_t* argv[]);

static int InitializeRuntime()
{
    if (!RhInitialize())
        return -1;

    // Transfer additional memory to kernel pool
    // If you receive a "Title has exhausted kernel pool quota" crash, increase this size
    // Must be multiples of 2MB sizes
    const size_t additionalKernelPoolMemory = 40 * 1048576; // 2 was changed to 40 for October 2024 GDK
    if (!XMemTransferMemory(XMemTransferTitleToSystem, additionalKernelPoolMemory))
    {
        return -1;
    }

    // RhpEnableConservativeStackReporting();

    void* osModule = PalGetModuleHandleFromPointer((void*)&NATIVEAOT_ENTRYPOINT);

    // TODO: pass struct with parameters instead of the large signature of RhRegisterOSModule
    if (!RhRegisterOSModule(
        osModule,
        (void*)&__managedcode_a, (uint32_t)((char*)&__managedcode_z - (char*)&__managedcode_a),
        (void*)&__unbox_a, (uint32_t)((char*)&__unbox_z - (char*)&__unbox_a),
        (void**)&c_classlibFunctions, _countof(c_classlibFunctions)))
    {
        return -1;
    }

    InitializeModules(osModule, __modules_a, (int)((__modules_z - __modules_a)), (void**)&c_classlibFunctions, _countof(c_classlibFunctions));

    return 0;
}

// Declaring this manually so we don't have to #include Windows.h and shellapi.h
extern "C" wchar_t** __stdcall CommandLineToArgvW(_In_ wchar_t* lpCmdLine, _Out_ int* pNumArgs);

int __cdecl wWinMain(void* hInst, void* hInstPrev, wchar_t *pCmdLine, int nCmdShow)
{
    int initval = InitializeRuntime();
    if (initval != 0)
        return initval;

    int argc;
    wchar_t **wargv = CommandLineToArgvW(pCmdLine, &argc);

    return __managed__Main(argc, wargv);
}
