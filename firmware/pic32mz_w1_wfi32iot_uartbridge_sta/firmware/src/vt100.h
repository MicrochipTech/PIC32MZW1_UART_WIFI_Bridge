
#ifndef _VT100_H    
#define _VT100_H

//Reset
#define VT100_TERMINAL_RESET			"\x1b""c" 

//Erasing
#define VT100_CLR_LINE_AFTER_CURSOR     "\x1b""[K"
#define VT100_CLR_LINE_TO_CURSOR		"\x1b""[1K"
#define VT100_CLR_LINE					"\x1b""[2K"
#define	VT100_CLR_SCREEN				"\x1b""[2J"
#define VT100_CLR_ALL					"\x1b""[1;1H\x1b""[2J"

//Cursor
#define VT100_CURSOR_OFF				"\x1b""[?25l"
#define VT100_CURSOR_ON                 "\x1b""[?25h"
#define VT100_CURSOR_HOME				"\x1b""[H"
#define VT100_CURSOR_SAVE				"\x1b""7"
#define VT100_CURSOR_RESTORE			"\x1b""8"
#define VT100_CURSOR_UP                 "\x1b""[A"
#define VT100_CURSOR_DOWN				"\x1b""[B"
#define VT100_CURSOR_RIGHT				"\x1b""[C"
#define VT100_CURSOR_LEFT				"\x1b""[D"

//Text
#define VT100_TEXT_DEFAULT				"\x1b""[0m"
#define VT100_TEXT_BRIGHT				"\x1b""[1m"
#define VT100_TEXT_DIM					"\x1b""[2m"
#define VT100_TEXT_UNDERSCORE			"\x1b""[4m"
#define VT100_TEXT_BLINK				"\x1b""[5m"
#define VT100_TEXT_REVERSE				"\x1b""[7m"
#define VT100_TEXT_HIDDEN				"\x1b""[8m"

//Text colors
#define VT100_FOREGROUND_BLACK			"\x1b""[30m"
#define VT100_FOREGROUND_RED			"\x1b""[31m"
#define VT100_FOREGROUND_GREEN			"\x1b""[32m"
#define VT100_FOREGROUND_YELLOW         "\x1b""[33m"
#define VT100_FOREGROUND_BLUE			"\x1b""[34m"
#define VT100_FOREGROUND_MAGNETA		"\x1b""[35m"
#define VT100_FOREGROUND_CYAN			"\x1b""[36m"
#define VT100_FOREGROUND_WHITE			"\x1b""[37m"
#define VT100_FOREGROUND_DEFAULT		"\x1b""[39m"

//Background colors
#define VT100_BACKGROUND_BLACK			"\x1b""[40m"
#define VT100_BACKGROUND_RED			"\x1b""[41m"
#define VT100_BACKGROUND_GREEN			"\x1b""[42m"
#define VT100_BACKGROUND_YELLOW         "\x1b""[43m"
#define VT100_BACKGROUND_BLUE			"\x1b""[44m"
#define VT100_BACKGROUND_MAGNETA		"\x1b""[45m"
#define VT100_BACKGROUND_CYAN			"\x1b""[46m"
#define VT100_BACKGROUND_WHITE			"\x1b""[47m"
#define VT100_BACKGROUND_DEFAULT		"\x1b""[49m"


#endif

/* *****************************************************************************
 End of File
 */
