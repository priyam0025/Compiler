global _start
_start:
    mov rax, 0
    push rax
    push QWORD [rsp + 0]

    pop rax
    test rax, rax
    jz .L_else_0
    mov rax, 10
    push rax
    mov rax, 60
    pop rdi
    syscall
    jmp .L_end_0
.L_else_0:
    mov rax, 20
    push rax
    mov rax, 60
    pop rdi
    syscall
.L_end_0:
    mov rax, 60
    mov rdi, 0
    syscall
