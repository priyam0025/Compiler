global _start
_start:
    mov rax, 4
    push rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    sub rax, rbx
    push rax
    mov rax, 2
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    push QWORD [rsp + 0]

    mov rax, 2
    push rax
    pop rbx
    pop rax
    cqo
    idiv rbx
    push rax
    mov rax, 3
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
