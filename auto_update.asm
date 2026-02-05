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

    cmp x0, #0
    je git_process

    mov x8, #260
    sub sp, sp, #16     // Make space on stack for status int
    mov x1, sp          // stat_addr: address of status on stack
    mov x2, #0          // options: 0 (blocking)
    mov x3, #0          // rusage: NULL

    ret
git_process:
    mov x8, #221
    ldr x0, =path
.section .data
    path: .ascii "/usr/bin/git"
