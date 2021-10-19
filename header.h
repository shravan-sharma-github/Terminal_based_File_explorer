#include <iostream>
#include <vector>
#include <queue>
#include <cerrno>
#include <stack>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sstream>
//#include <bits/stdc++.h>
#include <pwd.h>
#include <grp.h>
#include <filesystem>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>//for window size
#include <fcntl.h>
#include <csignal>//for window resize
#include <iomanip>//for formatting dir content on screen

#define clearLine() printf("\033[K")
#define ENT 10
#define BKSP 127
#define UP 65
#define LEFT 68
#define K 107
#define H 104
#define DOWN 66
#define RIGHT 67
#define L 108
#define SPC 32
#define ESC 27

//colors
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define NORMAL "\033[0m"


/*============================= Global Functions =========================================*/
//on error it print 's' with errno value
void die(const char *s);
void disableRawMode();
void enableRawMode();
void clear_screen();

