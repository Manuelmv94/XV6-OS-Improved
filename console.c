// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define BOOL_TRUE   1
#define BOOL_FALSE  0

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  struct spinlock lock;
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

#define UP_ARROW_KEY    226
#define DOWN_ARROW_KEY  227
#define LEFT_ARROW_KEY  228
#define RIGHT_ARROW_KEY 229
#define COMMAND_HISTORY_SIZE  10

char buffs[COMMAND_HISTORY_SIZE][INPUT_BUF];
unsigned char activebuffs;
unsigned char mrecentbuff;
unsigned char nextbuff;
unsigned char lastbuff;
unsigned char equalBuffers;

#define C(x)  ((x)-'@')  // Control-x
extern int killproc(void);

void
consoleintr(int (*getc)(void))
{
  int c;

  acquire(&input.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('C'):  // Kill running process
      killproc();
      break;
    case C('P'):  // Process listing.
      procdump();
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
      }
      break;

    case UP_ARROW_KEY:
      if(activebuffs>0){//Check if the command History is empty

        //Update the position of the most recent buffer from the command history to an older one, unless it reaches the oldest one
        if(lastbuff==BOOL_TRUE){
          lastbuff=BOOL_FALSE;
        }
        else{
          if(activebuffs<COMMAND_HISTORY_SIZE){
            if(mrecentbuff>0){
              mrecentbuff--;
            }
          }else{
            if(mrecentbuff!=nextbuff){
              if(mrecentbuff>0){
                mrecentbuff--;
              }else{
                mrecentbuff=COMMAND_HISTORY_SIZE-1;
              }
            }
          }
        }

        //Clean the input buffer and console
        while(input.e != input.w && input.buf[(input.e-1) % INPUT_BUF] != '\n'){
          input.e--;
          consputc(BACKSPACE);
        }

        //Place into the input buffer and console the next Command history content
        while(buffs[mrecentbuff][input.e-input.r] != '\n'){
          input.buf[input.e]=buffs[mrecentbuff][input.e-input.r];
          consputc(buffs[mrecentbuff][(input.e++)-input.r]);
        }

      }
      break;

    case DOWN_ARROW_KEY:
      if(activebuffs>0){//Check if the command history is empty

        //Update the position of the most recent buffer from the command history to a recent one, unless it reaches the most recent command
        if(activebuffs<COMMAND_HISTORY_SIZE){
          if(mrecentbuff<activebuffs-1){
            mrecentbuff++;
            lastbuff=BOOL_FALSE;
          }else{
            lastbuff=BOOL_TRUE;
          }
        }else{
          if(mrecentbuff<COMMAND_HISTORY_SIZE-1){
            if(mrecentbuff!=nextbuff-1){
              mrecentbuff++;
              lastbuff=BOOL_FALSE;
            }else{
              lastbuff=BOOL_TRUE;
            }
          }else{
            if(nextbuff!=0){
              mrecentbuff=0;
              lastbuff=BOOL_FALSE;
            }else{
              lastbuff=BOOL_TRUE;
            }
          }
        }

        //Clean the input buffer and console
        while(input.e != input.w && input.buf[(input.e-1) % INPUT_BUF] != '\n'){
          input.e--;
          consputc(BACKSPACE);
        }

        //Place into the input buffer and console the next Command history content, unless the most recent command was reached once before
        if(lastbuff==BOOL_FALSE){
          while(buffs[mrecentbuff][input.e-input.r] != '\n'){
            input.buf[input.e]=buffs[mrecentbuff][input.e-input.r];
            consputc(buffs[mrecentbuff][(input.e++)-input.r]);
          }
        }

      }
      break;
    case LEFT_ARROW_KEY:

      break;
    case RIGHT_ARROW_KEY:

      break;
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          input.w = input.e;

          if(input.e!=input.r+1){//Save buffer only if it isn't empty

            //Check if the new command is the same that the last one saved
            equalBuffers=BOOL_TRUE;
            for(input.e=input.r;input.e<input.w;input.e++){
              if(buffs[mrecentbuff][input.e-input.r]!=input.buf[input.e]){
                equalBuffers=BOOL_FALSE;
                break;
              }
            }

            //Save the new command to the command History only if it's different from the last command saved
            if(equalBuffers==BOOL_FALSE){
              for(input.e=input.r;input.e<input.w;input.e++){
                buffs[nextbuff][input.e-input.r]=input.buf[input.e];
              }

              mrecentbuff = nextbuff;
              nextbuff=(nextbuff+1) % COMMAND_HISTORY_SIZE;
              lastbuff=BOOL_TRUE;

              if(activebuffs<COMMAND_HISTORY_SIZE){
                activebuffs++;
              }
            }

          }
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&input.lock);
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&input.lock);
  while(n > 0){
    while(input.r == input.w){
      if(proc->killed){
        release(&input.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &input.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&input.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");
  initlock(&input.lock, "input");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}
