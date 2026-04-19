#include "lib.h"

// test function
int add(int a, int b) { return a + b; }

bool starts_with(const char *str, const char *prefix) {
  if (str == NULL || prefix == NULL) {
    return false;
  }
  bool x = strncmp(str, prefix, strlen(prefix));
  if (x == 0) {
    return true;
  } else {
    return false;
  }
}

char *get_substring(const char *str, int start, int length) {
  if (str == NULL) {
    return NULL;
  }

  size_t str_len = strlen(str);

  if (start < 0 || start >= str_len || length <= 0) {
    return NULL;
  }

  // Adjust length if it exceeds the remaining string length
  if (start + length > str_len) {
    length = str_len - start;
  }

  char *substring = (char *)malloc((length + 1) * sizeof(char));
  if (substring == NULL) {
    return NULL;
  }

  strncpy(substring, str + start, length);
  substring[length] = '\0'; // Null-terminate

  return substring;
}

void log_message(const char *filename, const char *format, ...) {
  FILE *fp = fopen(filename, "a");
  if (fp == NULL) {
    perror("Error opening log file");
    return; // Exit if file cannot be opened
  }

  va_list args;
  va_start(args, format);
  vfprintf(fp, format, args);
  va_end(args);

  fprintf(fp, "\n");

  fclose(fp);
}

// Function to find the first file ending with a specified extension in a
// directory
char *find_first_file_with_extension(const char *directory,
                                     const char *extension) {
  DIR *dir;
  struct dirent *ent;
  struct stat file_stat;
  char filepath[256]; // Buffer for the full file path (adjust as needed)
  char *found_file = NULL;
  size_t ext_len;

  // Check for NULL inputs
  if (directory == NULL || extension == NULL) {
    fprintf(stderr, "Error: NULL directory or extension provided.\n");
    return NULL;
  }

  // Calculate the length of the extension
  ext_len = strlen(extension);

  // Open the directory
  dir = opendir(directory);

  if (dir == NULL) {
    perror("Error opening directory");
    return NULL;
  }

  // Read directory entries
  while ((ent = readdir(dir)) != NULL) {
    // Ignore "." and ".." entries
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
      continue;
    }

    // Construct the full file path
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, ent->d_name);

    // Get file information
    if (stat(filepath, &file_stat) == 0) {
      // Check if it's a regular file
      if (S_ISREG(file_stat.st_mode)) {
        // Check if the filename ends with the specified extension
        size_t len = strlen(ent->d_name);
        if (len > ext_len &&
            strcmp(ent->d_name + len - ext_len, extension) == 0) {
          // Found a file ending with the specified extension
          found_file = strdup(ent->d_name);
          if (found_file == NULL) {
            perror("strdup failed");
            closedir(dir);
            return NULL;
          }
          closedir(dir);     // Close directory before returning
          return found_file; // Return the filename
        }
      }
    } else {
      fprintf(stderr, "Error getting file information for: %s\n", filepath);
    }
  }

  // Close the directory
  closedir(dir);
  return NULL; // No file found
}

int file_exists(const char *filename) {
  // Check if the filename is NULL
  if (filename == NULL) {
    fprintf(stderr, "Error: Filename is NULL\n");
    return 0; // Indicate that the file does not exist
  }

#ifdef _WIN32
  // Windows implementation using _access
  if (access(filename, F_OK) == 0) {
    // File exists
    return 1;
  } else {
    // File does not exist or there was an error
    return 0;
  }
#else
  // POSIX (Linux, macOS, etc.) implementation using stat
  struct stat buffer;
  if (stat(filename, &buffer) == 0) {
    // File exists
    return 1;
  } else {
    // File does not exist or there was an error
    return 0;
  }
#endif
}

// Function to create a directory if it doesn't already exist
int create_directory(const char *directory_name) {
  if (mkdir(directory_name, 0777) == 0) {
    // printf("Directory created successfully: %s\n", directory_name);
    return 0; // Success
  } else {
    // Check if the directory already exists
    if (errno == EEXIST) {
      // printf("Directory already exists: %s\n", directory_name);
      return 0; // Success (directory already exists)
    } else {
      // perror("Error creating directory");
      return 1; // Indicate an error
    }
  }
}

char *concat_strings(const char *str1, const char *str2) {
  if (str1 == NULL && str2 == NULL) {
    return NULL; // Or return an empty string, depending on desired behavior
  }

  if (str1 == NULL) {
    return strdup(str2); // Duplicate str2
  }

  if (str2 == NULL) {
    return strdup(str1); // Duplicate str1
  }

  size_t len1 = strlen(str1);
  size_t len2 = strlen(str2);
  size_t total_len = len1 + len2 + 1; // +1 for the null terminator

  char *result = (char *)malloc(total_len * sizeof(char));
  if (result == NULL) {
    perror("malloc failed");
    return NULL; // Indicate memory allocation failure
  }

  strcpy(result, str1); // Copy the first string
  strcat(result, str2); // Append the second string

  return result;
}

// Function to trim leading and trailing whitespace from a string
void trim(char *str) {
  char *start = str;
  while (isspace((unsigned char)*start))
    start++;

  char *end = str + strlen(str) - 1;
  while (end > start && isspace((unsigned char)*end))
    end--;

  end[1] = '\0';

  memmove(str, start, (end - start) + 2);
}

char *url_encode_utf8(const char *str) {
  size_t len = strlen(str);
  char *encoded =
      malloc(3 * len + 1); // Max possible size (each byte could become %XX)
  if (!encoded)
    return NULL;

  char *p = encoded;
  for (size_t i = 0; i < len; i++) {
    unsigned char c = str[i]; // Treat each byte as unsigned

    if (is_safe_utf8(c)) {
      // If it's a safe ASCII character, just copy it
      *p++ = c;
    } else if ((c & 0x80) == 0) {
      // If it's an ASCII character (0-127) that's not safe, encode it
      sprintf(p, "%%%02X", c);
      p += 3;
    } else {
      // Handle UTF-8 multi-byte characters:  Encode each byte individually
      sprintf(p, "%%%02X", c);
      p += 3;
    }
  }
  *p = 0; // Null terminate the encoded string
  return encoded;
}

int is_safe_utf8(unsigned char c) {
  return isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}
