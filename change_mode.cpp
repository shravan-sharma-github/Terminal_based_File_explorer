#include "header.h"
struct termios orig_termios;

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");    
}
void enableRawMode()
{
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1)die("tcgetattr");
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(ISTRIP);
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}