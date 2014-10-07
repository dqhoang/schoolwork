typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NULL     0
#define NPROC    9
#define SSIZE 1024

#define Umode    1
#define Kmode    2

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define BLOCK    3
#define SLEEP    4
#define ZOMBIE   5

#define READ_PIPE  4
#define WRITE_PIPE 5

#define NOFT      20
#define NFD       10

typedef struct Oft{
  int   mode;
  int   refCount;
  struct pipe *pipe_ptr;
} OFT;

#define PSIZE 10
#define NPIPE 10

// Timer stuff
int tick,sec,min,hr;

typedef struct pipe{
  char  buf[PSIZE];
  int   head, tail, data, room;
  int   nreader, nwriter;
  int   busy;
}PIPE;

typedef struct proc{
    struct proc *next;
    int    *sp;
    int    uss, usp;
    
    int    inkmode;            // added for interrupt processing

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];
    int    time;

    OFT    *fd[NFD];    

    int    kstack[SSIZE];      // per proc stack area
}PROC;

/*****************************************************
 *  Video Drivers
 ******************************************************/

#define VDC_INDEX      0x3D4
#define VDC_DATA       0x3D5
#define CUR_SIZE          10	/* cursor size register */
#define VID_ORG           12	/* start address register */
#define CURSOR            14	/* cursor position register */

#define LINE_WIDTH        80	/* # characters on a line */
#define SCR_LINES         25	/* # lines on the screen */
#define SCR_BYTES	    4000	/* bytes of ONE screen=25*80 */

#define CURSOR_SHAPE      15    /* block cursor for MDA/HGC/CGA/EGA/VGA... */

// attribute byte: 0x0HRGB, H=highLight; RGB determine color
#define HGREEN          0x0A
#define HCYAN           0x0B
#define HRED            0x0C
#define HPURPLE         0x0D
#define HYELLOW         0x0E
#define HWHITE          0x0F

#define DGREEN          0xFA
#define DBLACK          0xF0

u16 base     = 0xB800;    // VRAM base address
u16 vid_mask = 0x3FFF;    // mask=Video RAM size - 1

u16 offset;               // offset from VRAM base
int org;                  // current display origin r.e.VRAM base
int row, column;          // logical row, col position

/*****************************************************
 *  keyboard stuff
 ******************************************************/


/****************************************************
 * Serial 
 ****************************************************/
