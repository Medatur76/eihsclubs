.globl git_pull
.type git_pull,%function
.section .bss
    status:
        .skip 4        // int status
.section .text
git_pull:
    mov x8, #220
    mov x0, #17
    mov x1, #0
    mov x2, #0
    mov x3, #0
    mov x4, #0
    svc 0

    cmp x0, #0
    b.eq git_process

    mov x8, #260
    adrp x1, status
    add x1, x1, :lo12:status
    mov x2, #0          // options: 0 (blocking)
    mov x3, #0          // rusage: NULL

    svc #0

    

    adrp x1, status
    add  x1, x1, :lo12:status
    ldr  w0, [x1]

    // if (WIFEXITED(status))
    and  w2, w0, #0x7f
    cbnz w2, not_normal_exit

    // exit_code = (status >> 8) & 0xff
    lsr  w0, w0, #8
    and  w0, w0, #0xff

    ret

not_normal_exit:
    // Child was killed by signal
    // Conventionally return 128 + signal
    and  w0, w0, #0x7f
    add  w0, w0, #128

    ret
git_process:
    mov x8, #221
    adr x0, path
    adr x1, argv
    mov x2, #0
    svc #0

    mov x0, #1
    mov x8, #93
    svc #0
.section .data
    path:
        .asciz "/usr/bin/git"
    arg0:
        .asciz "git"
    arg1:
        .asciz "pull"
    arg2: 
        .asciz "--quiet"
    argv:
        .quad arg0
        .quad arg1
        .quad arg2
        .quad 0
