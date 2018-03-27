;  tls QWORD 8*100 + 1480h
;  inc gs:[14F8h]

.code

EXTERN PrepareNewCounterStorage : proc;

ASM_Break PROC

  int 3
  ret

ASM_Break ENDP

ASM_IncrementCounter PROC

  mov rax, qword ptr gs:[17A0h]
  inc qword ptr [rax + rcx]
  ret

ASM_IncrementCounter ENDP


ASM_IncrementCounter2 PROC

  mov rax, qword ptr gs:[17A0h]
  add qword ptr [rax + rcx], rdx
  ret

ASM_IncrementCounter2 ENDP


ASM_InitializeThread PROC

  sub rsp, 20h
  call PrepareNewCounterStorage
  add rsp, 20h
  mov qword ptr gs:[17A0h], rax
  ret

ASM_InitializeThread ENDP


END