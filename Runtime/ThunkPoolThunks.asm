LEAF_ENTRY macro Name, Section
    Section segment para 'CODE'
    align   16
    public  Name
    Name    proc
endm

LEAF_END macro Name, Section
    Name    endp
    Section ends
endm

THUNK macro PageIndex, ThunkIndex, ThunkPoolPageEnd
    align THUNK_CODESIZE
    mov r10, ((PageIndex * PAGE_SIZE) + (ThunkIndex * THUNK_CODESIZE))
    jmp ThunkPoolPageEnd
    int 3
    int 3
    int 3
    int 3
endm

THUNK_PAGE macro PageIndex
    local ThunkIndex, ThunkPoolPageEnd
    align PAGE_SIZE
    ThunkIndex = 0
    repeat THUNK_POOL_NUM_THUNKS_PER_PAGE
        THUNK PageIndex, ThunkIndex, ThunkPoolPageEnd
        thunkIndex = thunkIndex + 1
    endm

ThunkPoolPageEnd:
    mov r11, [g_pThunkStubData]
    add r10, r11
    jmp qword ptr[r11+PageIndex * PAGE_SIZE + PAGE_SIZE - POINTER_SIZE]
endm

THUNK_CODESIZE                          equ 10h    ; 7-byte mov, 5-byte jmp, 4 bytes of debug break
THUNK_DATASIZE                          equ 10h    ; 2 qwords
THUNK_POOL_NUM_THUNKS_PER_PAGE          equ 250     ; 250 thunks per 4K page
THUNK_POOL_NUM_THUNKS_PER_MAPPING       equ 4
PAGE_SIZE                               equ 4*1024  ; the system only works with 4K pages. So we lie
PAGE_COUNT                              equ 32
POINTER_SIZE                            equ 8
EXTERN g_pThunkStubData: PTR

THUNK_SEGMENT SEGMENT ALIGN(PAGE_SIZE) 'CODE'
ThunkPool:
    PageIndex = 0
repeat PAGE_COUNT
    THUNK_PAGE PageIndex
    PageIndex = PageIndex + 1
endm

;
; IntPtr RhpGetThunksBase()
;
LEAF_ENTRY RhpGetThunksBase, _TEXT
    ; Return the address of the first thunk pool to the caller (this is really the base address)
    lea rax, [ThunkPool]
    ret
LEAF_END RhpGetThunksBase, _TEXT

;
; int RhpGetNumThunksPerBlock()
;
LEAF_ENTRY RhpGetNumThunksPerBlock, _TEXT
    mov     eax, THUNK_POOL_NUM_THUNKS_PER_PAGE
    ret
LEAF_END RhpGetNumThunksPerBlock, _TEXT

;
; int RhpGetThunkSize()
;
LEAF_ENTRY RhpGetThunkSize, _TEXT
    mov     eax, THUNK_CODESIZE
    ret
LEAF_END RhpGetThunkSize, _TEXT

;
; int RhpGetNumThunkBlocksPerMapping()
;
LEAF_ENTRY RhpGetNumThunkBlocksPerMapping, _TEXT
    mov     eax, THUNK_POOL_NUM_THUNKS_PER_MAPPING
    ret
LEAF_END RhpGetNumThunkBlocksPerMapping, _TEXT

;
; int RhpGetThunkBlockSize
;
LEAF_ENTRY RhpGetThunkBlockSize, _TEXT
    mov    eax, PAGE_SIZE
    ret
LEAF_END RhpGetThunkBlockSize, _TEXT

;
; int RhpGetThunkBlockCount()
;
LEAF_ENTRY RhpGetThunkBlockCount, _TEXT
    mov     eax, PAGE_COUNT
    ret
LEAF_END RhpGetThunkBlockCount, _TEXT

;
; IntPtr RhpGetThunkDataBlockAddress
;
LEAF_ENTRY RhpGetThunkDataBlockAddress, _TEXT
    lea rax, [ThunkPool]
    sub rcx, rax
    and rcx, not (PAGE_SIZE-1)
    mov rax, [g_pThunkStubData]
    add rax, rcx
    ret
LEAF_END RhpGetThunkDataBlockAddress, _TEXT

;
; IntPtr RhpGetThunkStubsBlockAddress
;
LEAF_ENTRY RhpGetThunkStubsBlockAddress, _TEXT
    mov rax, [g_pThunkStubData]
    sub rcx, rax
    and rcx, not (PAGE_SIZE-1)
    lea rax, [ThunkPool]
    add rax, rcx
    ret
LEAF_END RhpGetThunkStubsBlockAddress, _TEXT

end