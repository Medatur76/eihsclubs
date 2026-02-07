.globl git_pull
.type git_pull,%function
.section .bss
    .align 4
    status:
        .skip 4        // int status
    output_fd:
        .word 0
.section .text
git_pull:

    //Open/trunicate file
    mov x8, #56
    mov x0, #-100
    adr x1, output_file
    mov x2, #577
    mov x3, #0644
    svc #0
    
    adrp x1, output_fd
    add x1, x1, :lo12:output_fd
    str w0, [x1]      // Use str w0 for 32-bit fd

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

_cleanup:

    mov x2, x0

    mov x8, #57
    adr x0, output_fd
    svc #0

    mov x0, x2

    ret
not_normal_exit:
    // Child was killed by signal
    // Conventionally return 128 + signal
    and  w0, w0, #0x7f
    add  w0, w0, #128

    b _cleanup
git_process:
    //Replace STDOUT with fd
    adrp x4, output_fd
    add x4, x4, :lo12:output_fd
    
    mov x8, #24       // dup2 syscall
    ldr x0, [x4]
    mov x1, #1        // STDOUT_FILENO
    svc #0

    mov x8, #24       // dup2 syscall
    mov x1, #2        // STDERR_FILENO
    svc #0

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
    argv:
        .quad arg0
        .quad arg1
        .quad 0
    output_file:
        .asciz "web/api/.hidden/pushEvent"
