
.code

extern any_hook_proxy64 :PROC

;stack top is rcx->rflags->ret_addr
any_hook_stub64 proc
mov [rsp-100h],rcx
pop rcx
sub rsp, 100h;
mov [rsp],	   rax
mov [rsp+8h], rbx
mov [rsp+10h],rcx
mov [rsp+18h],rdx
mov [rsp+20h],rsi
mov [rsp+28h],rdi
mov [rsp+30h],rbp
mov [rsp+38h],rsp
mov [rsp+40h],r8
mov [rsp+48h],r9
mov [rsp+50h],r10
mov [rsp+58h],r11
mov [rsp+60h],r12
mov [rsp+68h],r13
mov [rsp+70h],r14
mov [rsp+78h],r15
;get rflags
mov rax, [rsp+100h];
mov [rsp+80h],rax

;get context pointer
mov rcx, [rsp-8h];



mov rdx, rsp
;rbp save by callee
mov rbp, rsp
;16 bit align
and rsp, -10h

call any_hook_proxy64
;restore rsp
mov rsp, rbp

;ret to origion
mov [rsp+108h],rax
;get changed rflags
mov rax, [rsp+80h]
mov [rsp+100h],rax

mov rax, [rsp]  
mov rbx, [rsp+8h]
mov rcx, [rsp+10h]
mov rdx, [rsp+18h]
mov rsi, [rsp+20h]
mov rdi, [rsp+28h]
mov rbp, [rsp+30h]
mov rsp, [rsp+38h]
mov r8, [rsp+40h]
mov r9, [rsp+48h]
mov r10, [rsp+50h]
mov r11, [rsp+58h]
mov r12, [rsp+60h]
mov r13, [rsp+68h]
mov r14, [rsp+70h]
mov r15, [rsp+78h]
add rsp, 100h
;sub or add changed flags, so restore flags last
popfq
ret

any_hook_stub64 endp
end