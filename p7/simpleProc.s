.globl main
.data
gbl_func: .quad 0
gbl_main: .quad 0
str_0: .asciz "string"
.align 8
.text
#Fn prologue func
fun_func: pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $96, %rsp
#Fn body func
#            getarg 1 [cond]
#            getarg 2 [cond2]
#            [a] := 4
movq $4, %rax
movq %rax, -40(%rbp)
#            [a] := [a] ADD64 1
movq -40(%rbp), %rax
movq $1, %rbx
addq %rbx, %rax
movq %rax, -40(%rbp)
#            [a] := [a] SUB64 1
movq -40(%rbp), %rax
movq $1, %rbx
subq %rbx, %rax
movq %rax, -40(%rbp)
#            [tmp0] := [a] DIV64 2
movq -40(%rbp), %rax
movq $2, %rbx
movq $0, %rdx
idivq %rbx
movq %rax, -72(%rbp)
#            [a] := [tmp0]
movq -72(%rbp), %rax
movq %rax, -40(%rbp)
#            [tmp1] := [a] GT64 1
movq -40(%rbp), %rax
movq $1, %rbx
movq $0, %rcx
cmpq %rbx, %rax
setg %cl
movq %rcx, -80(%rbp)
#            IFZ [tmp1] GOTO lbl_2
movq -80(%rbp), %rax
cmpq $0, %rax
je lbl_2
#            [a] := [a] ADD64 1
movq -40(%rbp), %rax
movq $1, %rbx
addq %rbx, %rax
movq %rax, -40(%rbp)
lbl_2: #lbl_2:      nop
nop
#            [tmp2] := [a] LT64 1
movq -40(%rbp), %rax
movq $1, %rbx
movq $0, %rcx
cmpq %rbx, %rax
setl %cl
movq %rcx, -88(%rbp)
#            IFZ [tmp2] GOTO lbl_3
movq -88(%rbp), %rax
cmpq $0, %rax
je lbl_3
#            [a] := [a] ADD64 1
movq -40(%rbp), %rax
movq $1, %rbx
addq %rbx, %rax
movq %rax, -40(%rbp)
#            goto lbl_4
jmp lbl_4
lbl_3: #lbl_3:      nop
nop
#            [a] := [a] SUB64 1
movq -40(%rbp), %rax
movq $1, %rbx
subq %rbx, %rax
movq %rax, -40(%rbp)
lbl_4: #lbl_4:      nop
nop
#            [tmp3] := [a] EQ64 2
movq -40(%rbp), %rax
movq $2, %rbx
movq $0, %rcx
cmpq %rbx, %rax
sete %cl
movq %rcx, -96(%rbp)
#            IFZ [tmp3] GOTO lbl_5
movq -96(%rbp), %rax
cmpq $0, %rax
je lbl_5
#            [a] := 5
movq $5, %rax
movq %rax, -40(%rbp)
lbl_5: #lbl_5:      nop
nop
#            WRITE str_0
movq $str_0, %rdi
callq printString
#            WRITE [a]
movq -40(%rbp), %rdi
callq printInt
#            [tmp4] := NOT64 1
movq $1, %rax
notq %rax
movq %rax, -104(%rbp)
#            IFZ [tmp4] GOTO lbl_6
movq -104(%rbp), %rax
cmpq $0, %rax
je lbl_6
#            [b] := [b] SUB64 1
movq -48(%rbp), %rax
movq $1, %rbx
subq %rbx, %rax
movq %rax, -48(%rbp)
lbl_6: #lbl_6:      nop
nop
#            IFZ [cond] GOTO lbl_7
movq -24(%rbp), %rax
cmpq $0, %rax
je lbl_7
#            [t] := 5
movq $5, %rax
movq %rax, -56(%rbp)
#            goto lbl_8
jmp lbl_8
lbl_7: #lbl_7:      nop
nop
#            [t] := 0
movq $0, %rax
movq %rax, -64(%rbp)
lbl_8: #lbl_8:      nop
nop
lbl_9: #lbl_9:      nop
nop
#            IFZ [cond2] GOTO lbl_10
movq -32(%rbp), %rax
cmpq $0, %rax
je lbl_10
#            [cond2] := 0
movq $0, %rax
movq %rax, -32(%rbp)
#            goto lbl_9
jmp lbl_9
lbl_10: #lbl_10:     nop
nop
#            setret [a]
movq -40(%rbp), %rax
#            goto lbl_1
jmp lbl_1
#Fn epilogue func
lbl_1: addq $96, %rsp
popq %rbp
retq
#Fn prologue main
main: pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $32, %rsp
#Fn body main
#            setarg 1 1
movq $1, %rdi
#            setarg 2 0
movq $0, %rsi
#            call func
callq fun_func
#            magic [tmp1]
movq -40(%rbp), %rax
callq magic
#            [magic] := [tmp1]
movq -40(%rbp), %rax
movq %rax, -24(%rbp)
#            WRITE [magic]
movq -24(%rbp), %rdi
callq printBool
#Fn epilogue main
lbl_11: addq $32, %rsp
popq %rbp
retq
