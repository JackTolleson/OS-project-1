#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// function declarations
char* find_executable(char *, char **);

int main() {
    // char input = user input 
  
    char *cmd;
    char *path;
    // "Your initial shell path should contain one directory: /bin"
    char *path_directories[] = {"/bin", NULL};
  
    while {
      // variable path finding
      cmd = input;
      path = find_executable(cmd, path_dirs);
  
      if (path) {
        // format like: "Found: /bin/ls"
          printf("Found: %s\n", path);
      } else {
          printf("Command not found\n");
      }
    { 
  
    return 0;
}

char* find_executable(char *cmd, char **path_dirs) {
    // run until path directory "string" (character) array is done
    for (int i = 0; path_dirs[i] != NULL; i++) {
        // gets length of full "string"
        int len = strlen(path_dirs[i]) + strlen(cmd) + 2;
        // fixed-size buffer
        char full_path[1024];  
        // writes text into a buffer (format: snprintf(buffer_variable, size of buffer, format of text, first string, second string) )
        snprintf(full_path, sizeof(full_path), "%s/%s", path_dirs[i], cmd);

        // check if it's executable
        if (access(full_path, X_OK) == 0) {
            return full_path;
        }
    }
    // if command path is not found
    return NULL;
}
