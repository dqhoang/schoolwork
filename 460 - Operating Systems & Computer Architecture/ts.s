                MTXSEG = 0x1000

!               IMPORTS and EXPORTS
.globl _main                               
.globl _tswitch,_running,_scheduler
.globl _proc, _procSize

.globl _int80h,_kcinth,_goUmode
.globl _get_byte,_put_byte

.globl _lock, _unlock,_int_on,_int_off,_in_byte, _out_byte
.globl _tinth, _thandler

!.globl _kbinth, _kbhandler

                                      
		jmpi	start,MTXSEG
start:
        mov     ax,cs                   ! establish segments 
        mov     ds,ax                   ! we know ES,CS=0x1000. Let DS=CS  
        mov     ss,ax                   ! SS = CS ===> all point to 0x1000
        mov     es,ax

        mov     sp,#_proc               ! SP -> proc[0].kstack HIGH end
        add     sp,_procSize

        mov     ax,#0x0003
        int     #0x10

        call _main                      ! call main[] in C  
	
_tswitch:
          push   ax
          push   bx
          push   cx
          push   dx
          push   bp
          push   si
          push   di
	  pushf
	  mov	 bx, _running
 	  mov	 2[bx], sp

find:     call	 _scheduler

resume:	  mov	 bx, _running
	  mov	 sp, 2[bx]
	  popf
	  pop    di
          pop    si
          pop    bp
          pop    dx
          pop    cx
          pop    bx
          pop    ax
          ret

USS =  4
USP =  6
INK =  8

! as86 macro: parameters are ?1 ?2, etc 
! as86 -m -l listing src (generates listing with macro expansion)

         MACRO INTH
          push ax
          push bx
          push cx
          push dx
          push bp
          push si
          push di
          push es
          push ds

          push cs
          pop  ds

          mov bx,_running      !ready to access proc
          inc INK[bx]           ! enter Kmode : ++inkmode
          cmp INK[bx],#1        ! if inkmode == 1 ==> interrupt was in Umode
          jg  ?1                ! imode>1 : was in Kmode: bypass saving uss,usp

          ! was in Umode: save interrupted (SS,SP) into proc
	  mov si,_running   	! ready to access proc
          mov USS[si],ss        ! save SS  in proc.USS
          mov USP[si],sp        ! save SP  in proc.USP

          ! change DS,ES,SS to Kernel segment
          mov  di,ds            ! stupid !!        
          mov  es,di            ! CS=DS=SS=ES in Kmode
          mov  ss,di

          mov  sp, _running     ! sp -> runnings kstack[] high end
          add  sp, _procSize

?1:       call  _?1             ! call handler in C

          br    _ireturn        ! return to interrupted point

         MEND

_int80h: INTH kcinth
_tinth:  INTH thandler
!_kbinth: INTH kbhandler


!*===========================================================================*
!*		_ireturn  and  goUmode()       				     *
!*===========================================================================*
! ustack contains    flag,ucs,upc, ax,bx,cx,dx,bp,si,di,es,ds
! uSS and uSP are in proc
_ireturn:
_goUmode:
        cli
        mov bx, _running
        dec INK[bx]             ! --inkmode
        cmp INK[bx],#0         ! inkmode==0 means was in Umode
        jg  xkmode

! restore uSS, uSP from running PROC
	mov si,_running 	! si -> proc
        mov ax,USS[si]
        mov ss,ax               ! restore SS
        mov sp,USP[si]          ! restore SP
xkmode:                         
	pop ds
	pop es
	pop di
        pop si
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax 
        iret

!*===========================================================================*
!*			 old_flag=lock()				     *
!*===========================================================================*
! Disable CPU interrupts.
_lock:  
	pushf			! save flags on stack
	cli			! disable interrupts
	pop ax   		! pop saved flag into ax
	ret			! return old_flag


!*===========================================================================*
!*				int_on					   								     *
!*===========================================================================*
! Enable CPU interrupts.
_int_on:
	sti			! enable interrupts
	ret			! return to caller
	
!*===========================================================================*
!*				int_off					   								     *
!*===========================================================================*
! Disable CPU interrupts.
_int_off:
	cli			! disables interrupts
	ret			! return to caller

!*===========================================================================*
!*				unlock(old_flag)       		     *
!*===========================================================================*
! Restore enable/disable bit to the value it had before last lock.
_unlock:
        push bp
        mov  bp,sp
       
        push 4[bp]
	popf			! restore old_flag

        mov  sp,bp
        pop  bp
	ret			! return to caller


!*===========================================================================*
!*				in_byte					     *
!*===========================================================================*
! PUBLIC unsigned in_byte[port_t port];
! Read an [unsigned] byte from the i/o port  port  and return it.

_in_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
	in      ax,dx			! input 1 byte
	subb	ah,ah		! unsign extend
        pop     bp
        ret

!*===========================================================================*
!*				out_byte				     *
!*==============================================================
! out_byte[port_t port, int value];
! Write  value  [cast to a byte]  to the I/O port  port.

_out_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
        mov     ax,6[bp]
	outb	dx,al		! output 1 byte
        pop     bp
        ret
        
