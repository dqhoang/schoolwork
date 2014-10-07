        .globl _main,_syscall,_exit,_getcs, auto_start
        
auto_start:
        call _main
	
! if ever return, exit(0)
	push  #0
        call  _exit

_getcs:
        mov   ax, cs
        ret

_syscall:
        int    80
        ret

