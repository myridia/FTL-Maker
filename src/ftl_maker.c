#include <ctype.h>
#include <curl/curl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 256
#define MAX_MESSAGE_ID_LENGTH 64
#define MAX_MESSAGE_VALUE_LENGTH 256

// Structure to represent a message
typedef struct {
  char id[MAX_MESSAGE_ID_LENGTH];
  char value[MAX_MESSAGE_VALUE_LENGTH];
} FTLMessage;

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

// Function to parse an FTL file
int parse_ftl_file(const char *filename, FTLMessage *messages,
                   int *num_messages) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Error opening file");
    return 1;
  }

  char line[MAX_LINE_LENGTH];
  *num_messages = 0;

  while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
    trim(line);

    // Skip comments and empty lines
    if (line[0] == '#' || line[0] == '\0') {
      continue;
    }

    char *equals_pos = strchr(line, '=');
    if (equals_pos != NULL) {
      // Extract message ID
      strncpy(messages[*num_messages].id, line, equals_pos - line);
      messages[*num_messages].id[equals_pos - line] = '\0';
      trim(messages[*num_messages].id);

      // Extract message value
      strncpy(messages[*num_messages].value, equals_pos + 1,
              MAX_MESSAGE_VALUE_LENGTH - 1);
      messages[*num_messages].value[MAX_MESSAGE_VALUE_LENGTH - 1] = '\0';
      trim(messages[*num_messages].value);

      (*num_messages)++;

      if (*num_messages >= 1000) {
        fprintf(stderr, "Warning.\n");
        break;
      }
    }
  }

  fclose(file);
  return 0;
}

// Structure to hold the response data
struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb,
                                    void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (ptr == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

// Function to translate a single string
void *fill_ftl(char ftl[104][6]) {
  CURL *curl;
  CURLcode res;
  struct MemoryStruct chunk;
  char url[256];

  chunk.memory = malloc(1);
  chunk.size = 0;
  snprintf(url, sizeof(url), "https://mtranslate.myridia.com/ftl");
  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
      goto cleanup;
    } else {
      json_error_t error;
      json_t *root = json_loads(chunk.memory, 0, &error);

      if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        goto cleanup;
      } else {
        json_t *ftl_array = json_object_get(root, "ftl");
        size_t array_size = json_array_size(ftl_array);
        printf("Numbers: %zu\n", array_size);
        if (!json_is_array(ftl_array)) {
          printf("...is not array");
        }

        for (size_t i = 0; i < array_size; i++) {
          json_t *string_value = json_array_get(ftl_array, i);

          if (!json_is_string(string_value)) {
            fprintf(stderr, "Error:  %zu is not a string.\n", i);
            continue;
          }

          const char *str = json_string_value(string_value);

          if (str) {
            // strcpy(ftl[i], str);
            strncpy(ftl[i], str, 6 - 1);
            ftl[i][6 - 1] = '\0';
            // printf("String at index %zu: %s\n", i, str);
          } else {
            fprintf(stderr, "Error:  %zu.\n", i);
          }
        }

      cleanup_json:
        json_decref(root);
      }
    }

  cleanup:
    curl_easy_cleanup(curl);
  } else {
    fprintf(stderr, "curl_easy_init() failed\n");
  }

  free(chunk.memory);
}

int main(int argc, char *argv[]) {
  printf("Arguments: %d \n", argc);
  char ftl[104][6];
  fill_ftl(ftl);

  for (int i = 0; i < 104; i++) {
    printf("ftl[%d]: %s\n", i, ftl[i]);
  }

  FTLMessage messages[104]; // Array to store the messages
  int num_messages = 0;

  if (parse_ftl_file("base.ftl", messages, &num_messages) == 0) {
    printf("Parsed %d messages:\n", num_messages);
    for (int i = 0; i < num_messages; i++) {
      printf("ID: %s, Value: %s\n", messages[i].id, messages[i].value);
    }
  }

  return 0;
}
