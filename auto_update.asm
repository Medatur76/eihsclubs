.globl git_pull
.type git_pull,%function
.section .text
git_pull:
    mov x8, #220
    mov x0, #17
    mov x1, #0
    mov x2, #0
    mov x3, #0
    mov x4, #0
    svc 0

    mov x8, #64
    mov x0, #1
    ldr x1, =path
    mov x2, #13
    svc #0

    cmp x0, #0
    b.eq git_process

    mov x8, #260
    sub sp, sp, #16     // Make space on stack for status int
    mov x1, sp          // stat_addr: address of status on stack
    mov x2, #0          // options: 0 (blocking)
    mov x3, #0          // rusage: NULL

    mov x8, #64
    mov x1, x0
    mov x0, #1
    add x1, x1, #30
    mov x2, #2
    svc #0

    ret
git_process:
    mov x8, #221
    adr x0, path
    adr x1, args
    mov x2, #0
    svc #0

    mov x0, #1
    mov x8, #93
    svc #0
.section .data
    path: .ascii "/usr/bin/git\0"
    args: .quad arg0
    arg0: .ascii "pull"
