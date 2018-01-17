#include <unistd.h>

#define IS_UTF8_TRAIL(c) (((c)&0xC0) == 0x80)
#define IS_UTF8_HEAD(c) ((c)&0x80)

enum TermColor {
  COLOR_BLACK,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BLUE,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_WHITE,
  COLOR_COUNT
};

static int read_timeout(unsigned char *buf, int n, int ms);

static int read_char(unsigned char *c) {
  int err;

  for (;;) {
    err = read_timeout(c, 1, 0);
    if (err == 1)
      return 0;
    if (err == -1 && errno != EAGAIN)
      return 1;
  }
}

typedef enum {
  KEY_NONE = 0,
  KEY_UNKNOWN = 1,
  KEY_ESCAPE = '\x1b',
  KEY_RETURN = '\r',
  KEY_TAB = '\t',
  KEY_BACKSPACE = 127,
  KEY_ARROW_UP,
  KEY_ARROW_DOWN,
  KEY_ARROW_LEFT,
  KEY_ARROW_RIGHT,
  KEY_END,
  KEY_HOME
} SpecialKey;

typedef struct Utf8Char {
	char code[4];
} Utf8Char;

typedef struct {
	int is_special_key;
  Utf8Char character;
  SpecialKey special_key;
} Key;

static int read_key(Key *result, int escape_code_wait_ms /* = 1 */) {
	Utf8Char character;
  SpecialKey special_key = KEY_NONE;

  /* get input */

  {
    int nread;

    if (read_char(character.code))
      return 1;

    if (IS_UTF8_HEAD(*character.code)) {
      unsigned char *c = character.code+1;
      unsigned char x;

      for (x = *character.code << 1; x&0x80; x <<= 1, ++c)
        if (read_char(c))
          return 1;
    }

    /* some special chars */
    switch (*character.code) {
      case '\t':
        special_key = KEY_TAB;
        break;
      case '\r':
        special_key = KEY_RETURN;
        break;
      case KEY_BACKSPACE:
        special_key = KEY_BACKSPACE;
        break;
      default:
        break;
    }

    if (*character.code != '\x1b')
      goto done;

    /* escape sequence? */
    /* read '[' */
    nread = read_timeout(character.code, 1, escape_code_wait_ms);
    if (nread == -1 && errno != EAGAIN)
      return 1;
    if (nread == 0 || *character.code != '[') {
      special_key = KEY_ESCAPE;
      goto done;
    }

    /* read actual character */
    nread = read_timeout(character.code, 1, escape_code_wait_ms);
    if (nread == -1 && errno != EAGAIN)
      return 1;
    if (nread == 0) {
      special_key = KEY_UNKNOWN;
      goto done;
    }

    if (*character.code >= '0' && *character.code <= '9') {
      switch (*character.code) {
        case '1': special_key = KEY_HOME; break;
        case '4': special_key = KEY_END; break;
      }

      nread = read_timeout(character.code, 1, escape_code_wait_ms);
      if (nread == -1 && errno != EAGAIN)
        return 1;
      if (nread == 0 && *character.code != '~') {
        special_key = KEY_UNKNOWN;
        goto done;
      }
    } else {
      switch (*character.code) {
        case 'A': special_key = KEY_ARROW_UP; break;
        case 'B': special_key = KEY_ARROW_DOWN; break;
        case 'C': special_key = KEY_ARROW_RIGHT; break;
        case 'D': special_key = KEY_ARROW_LEFT; break;
        case 'F': special_key = KEY_END; break;
        case 'H': special_key = KEY_HOME; break;
        default: special_key = KEY_UNKNOWN; break;
      }
    }
  }

	done:
	if (special_key != KEY_NONE) {
		result->is_special_key = 1;
		result->special_key = special_key;
	} else {
		result->is_special_key = 0;
		result->character = character;
	}

	result->is_special_key = special_key != KEY_NONE;
  return 0;
}


typedef struct {const char *str; int len;} TermCommand;
static TermCommand term_fcolors[] = {
  {"\x1b[30m", 5}, /* COLOR_BLACK  */
  {"\x1b[31m", 5}, /* COLOR_RED    */
  {"\x1b[32m", 5}, /* COLOR_GREEN  */
  {"\x1b[33m", 5}, /* COLOR_YELLOW */
  {"\x1b[34m", 5}, /* COLOR_BLUE   */
  {"\x1b[35m", 5}, /* COLOR_MAGENTA*/
  {"\x1b[36m", 5}, /* COLOR_CYAN   */
  {"\x1b[37m", 5}, /* COLOR_WHITE  */
};
static TermCommand term_bcolors[] = {
  {"\x1b[40m", 5}, /* COLOR_BLACK  */
  {"\x1b[41m", 5}, /* COLOR_RED    */
  {"\x1b[42m", 5}, /* COLOR_GREEN  */
  {"\x1b[43m", 5}, /* COLOR_YELLOW */
  {"\x1b[44m", 5}, /* COLOR_BLUE   */
  {"\x1b[45m", 5}, /* COLOR_MAGENTA*/
  {"\x1b[46m", 5}, /* COLOR_CYAN   */
  {"\x1b[47m", 5}, /* COLOR_WHITE  */
};
STATIC_ASSERT(ARRAY_LEN(term_fcolors) == COLOR_COUNT, all_term_font_colors_defined);
STATIC_ASSERT(ARRAY_LEN(term_bcolors) == COLOR_COUNT, all_term_background_colors_defined);

static int term_bold() {
  return write(stdout, "\x1b[1m", 4) != 4;
}

static int term_inverse_video() {
	return write(stdout, "\x1b[7m", 4) != 4;
}

static int term_clear_screen() {
  return write(stdout, "\x1b[2J", 4) != 4;
}

static int term_hide_cursor() {
  return write(stdout, "\x1b[?25l", 6) != 6;
}

static int term_clear_line() {
  return write(stdout, "\x1b[2K", 4) != 4;
}

static int term_show_cursor() {
  return write(stdout, "\x1b[?25h", 6) != 6;
}

static int term_cursor_move(int x, int y) {
  char buf[32];
  unsigned int n = sprintf_s(buf, sizeof(buf), "\x1b[%i;%iH", y+1, x+1);
  return write(stdout, buf, n+1) != n+1;
}

static int term_reset_video() {
  return write(stdout, "\x1b[0m", 4) != 4;
}

static int term_get_dimensions(int *w, int *h) {
  struct winsize ws;
  int res = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  if (res == -1 || !ws.ws_col) {
    /* fall back to moving the cursor to the bottom right and querying its position */
    printf("%s", "\x1b[999C\x1b[999B\x1b[6n");
    fflush(stdout);
    res = scanf("\x1b[%d;%dR", h, w);
    if (res != 2 || !*w || !*h) return 1;
  } else {
    *w = ws.ws_col;
    *h = ws.ws_row;
  }
  return 0;
}

static void term_enable_raw_mode(struct termios *orig_termios) {
  struct termios new_termios;
  tcgetattr(STDIN_FILENO, orig_termios);
  new_termios = *orig_termios;

  new_termios.c_iflag &= ~(IXON /* Disable Control-s and Control-q */
                           | ICRNL /* Let Control-m and Enter be read as carriage returns ('\r') */ );
  new_termios.c_oflag &= ~(OPOST /* Disable that '\n' output is turned into "\r\n" */ );
  new_termios.c_lflag &= ~(ECHO /* Don't echo the input */
                           | ICANON /* Get input byte by byte, instead of waiting for endline */
                           /*| ISIG */ /* Don't catch control-c */
                           | IEXTEN /* Disable Control-v */);

  /* These should already be set, but we set them anyway just in case */
  new_termios.c_cflag |= (CS8); /* set character to be 8 bits, should already be set */
  new_termios.c_iflag &= ~(INPCK /* No parity checking */
                           | ISTRIP /* don't strip the 8th bit of each input byte */);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
  term_hide_cursor();
}

static int term_reset_to_default_settings(struct termios *orig_termios) {
	int err = 0;
  err |= term_clear_screen();
  err |= term_show_cursor();
  err |= term_cursor_move(0,0);
  err |= term_reset_video();
  err |= tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
  return err;
}

/* returns:
 * -1 on error
 * 0 on timedout
 * number of bytes read on success
 */
static int read_timeout(unsigned char *buf, int n, int ms) {
  fd_set files;
  struct timeval timeout = {0};
  int res;
  FD_ZERO(&files);
  FD_SET(STDIN_FILENO, &files);

  timeout.tv_sec = 0;
  timeout.tv_usec = ms*1000;

  res = select(STDIN_FILENO+1, &files, 0, 0, ms ? &timeout : 0);
  if (res == -1) return res; /* error */
  else if (res == 0) return res; /* timeout */
  else return read(STDIN_FILENO, buf, n);
}
