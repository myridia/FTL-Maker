#include "lib.h"
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <errno.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define access _access
#define F_OK 0
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

#define MAX_LINE_LENGTH 256
#define MAX_MESSAGE_ID_LENGTH 64
#define MAX_MESSAGE_VALUE_LENGTH 256

// test function
int add(int a, int b) { return a + b; }

int is_safe_utf8(unsigned char c) {
  return isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}
