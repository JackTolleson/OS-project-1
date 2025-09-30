#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// function declarations
char* find_executable(char *, char **);

int main() {
    // char input = user input             The input variable should be a char array Ex:char *input[] ... which is parsed by spaces
  
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

      if (strcmp(input[0], "exit") == 0) {
    if (input[1] != NULL) {
        fprintf(stderr, "exit: too many arguments\n");
    } else {
        //this will exit the wish program
        exit(0);
    }
}
else if (strcmp(input[0], "cd") == 0) {
    if (input[1] == NULL || input[2] != NULL) {
        fprintf(stderr, "cd: expected exactly one argument\n");
        //below will change the directory and give an error if it could not do that
    } else if (chdir(input[1]) != 0) {
        perror("cd");
    }
}

} 
  
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
