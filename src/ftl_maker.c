#include "lib.h"

int main(int argc, char *argv[]) {
  char *basefile = NULL;
  // printf("filename: %s \n", filename);
  // printf("Arguments: %d \n", argc);
  // char *base = "-b";
  for (int i = 0; i < argc; i++) {
    if (strncmp(argv[i], "-b", 2) == 0) {
      // printf("  %b\n", strncmp(argv[i], base, 2));
      if (argv[i + 1]) {
        if (file_exists(argv[i + 1])) {
          basefile = argv[i + 1];
          // printf("  argv[%d] = %s\n", i, argv[i + 1]);
        }
      }
    }

    if (strncmp(argv[i], "-a", 2) == 0) {

      if (argv[i + 1]) {
        if (file_exists(argv[i + 1])) {
          basefile = argv[i + 1];
          // printf("  argv[%d] = %s\n", i, argv[i + 1]);
        }
      }
    }
  }
  if (basefile == NULL) {
    basefile = find_first_file_with_extension(".", ".ftl");
  }
  if (basefile != NULL) {
    printf("basefile: %s \n", basefile);
    // return 0;
    // const char *directory_name2 = "i18n/xx";
    // int result2 = create_directory(directory_name2);
    // fill the ftl possible translations
    char ftl[104][6];
    fill_ftl(ftl);
    char folder_path[] = "i18n/";
    int result = create_directory(folder_path);

    for (int x = 0; x < 104; x++) {
      // const char *folder = ftl[x];
      char *code = get_substring(ftl[x], 0, 2);

      if (strlen(ftl[x]) == 5) {
        printf("size: %d code: %s folder: %s  \n", strlen(ftl[x]), code,
               ftl[x]);

        char *folder_path2 = concat_strings(folder_path, ftl[x]);
        int result2 = create_directory(folder_path2);
        char *file_path = concat_strings(folder_path2, "/app.ftl");

        FILE *fp = fopen(file_path, "w");
        FTLMessage messages[104];
        int num_messages = 0;
        /*
        if (parse_ftl_file(basefile, messages, &num_messages) == 0) {
          for (int i = 0; i < num_messages; i++) {
            char *translation = NULL;
            // if (same_code == 1) {
            if (!starts_with(messages[i].id, "!")) {
              // printf("code: %s ID: %s, Value: %s\n", code,
              messages[i].id,
                  //        messages[i].value);
                  translation = translate("en", code, messages[i].value);
              fprintf(fp, "%s = %s \n", messages[i].id, translation);
            } else {
              fprintf(fp, "%s = %s \n", messages[i].id, messages[i].value);
            }
            if (translation != NULL) {
              free(translation);
            }
            //}
          }
        }
        */

        fclose(fp);
      }
    }

    return 0;
  } else {
    printf("Missing baseline, have *.ftl file in your folder or point one to "
           "it\n");
    printf("Example: ftl_make -b my_basefile.ftl");
  }
}
