#include <curl/curl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
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
char *get_ftl() {
  CURL *curl;
  CURLcode res;
  struct MemoryStruct chunk;
  char url[256];
  char *translation = NULL;
  char ftl[104][5];

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
            strcpy(ftl[i], str);
            printf("String at index %zu: %s\n", i, str);
          } else {
            fprintf(stderr, "Error:  %zu.\n", i);
          }
        }

      cleanup_json:
        json_decref(root); // Release the JSON object
      }
    }

  cleanup:
    // Clean up curl
    curl_easy_cleanup(curl);
  } else {
    fprintf(stderr, "curl_easy_init() failed\n");
    translation = NULL; // Indicate failure
  }

  // Free the memory allocated for the response
  free(chunk.memory);

  return translation;
  //  return ftl;
}

int main(int argc, char *argv[]) {
  printf("Arguments: %d \n", argc);
  // char *translation = get_ftl();
  char *a = get_ftl();

  return 0;
}
