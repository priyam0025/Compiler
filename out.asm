global _start
_start:
    mov rax, 5
    push rax
    push QWORD [rsp + 0]

    mov rax, 3
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov QWORD [rsp + 0], rax
    push QWORD [rsp + 0]

    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
