//use gcc -Wall -Wextra -o wish shell.c to compile then ./wish to get into the shell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_ARGS 64

// added path variables
char **path_dirs = NULL; 
void free_path_dirs();    
void init_default_path(); 

//functions
int handle_builtin(char **args);

// function declarations
char* find_executable(char *, char **);
void execute_command(char **args, char *outputFile);
void print_error();


//helper functions for path
void free_path_dirs() {
    if (path_dirs == NULL) return;
    for (int i = 0; path_dirs[i] != NULL; i++) {
        if (path_dirs[i] != NULL) {
            free(path_dirs[i]);
            path_dirs[i] = NULL;
        }
    }
    free(path_dirs);
    path_dirs = NULL;
}


void init_default_path() {
    path_dirs = malloc(sizeof(char*) * 2);
    path_dirs[0] = strdup("/bin");
    path_dirs[1] = NULL;
}

// all of the 3 functions (Jack's Section)
int handle_builtin(char **args) {
    
    if (args[0] == NULL) return 0;

    // exit
    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) { print_error(); return 1; }
        exit(0);
    }

    // cd
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL) { print_error(); return 1; }
        if (chdir(args[1]) != 0) print_error();
        return 1;
    }

    // path
    if (strcmp(args[0], "path") == 0) {
        free_path_dirs();
        int count = 0;
        for (int i = 1; args[i] != NULL; i++) count++;
        path_dirs = malloc(sizeof(char*) * (count + 1));
        for (int i = 0; i < count; i++)
            path_dirs[i] = strdup(args[1 + i]);
        path_dirs[count] = NULL;
        return 1;
    }

    return 0;
}


// Print an error message
void print_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Split input line into arguments
int parse_line(char *line, char **args) {
    int argc = 0;
    char *token = strtok(line, " \t\n"); // Split by space/tab/newline
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL; // Null terminate for execvp
    return argc;
}

// Execute a command
void execute_command(char **args, char *outputFile) {
    if (args[0] == NULL) {
		return; // Empty command
	}

	//args[0] is the first pointer in the array that will point to a single command/arguments
	char *resolved_path = find_executable(args[0], path_dirs);
    if (resolved_path == NULL) {
        print_error();
        return;
    }

    //Used to handle the output redirection
    if (outputFile != NULL) {
        FILE *fp = fopen(outputFile, "w");
        if (fp == NULL) { // only runs when file cannot be opened/created
            print_error();
            free(resolved_path);
            return;
        }

        int fd = fileno(fp); 
        dup2(fd, STDOUT_FILENO); // redirect stdout
        dup2(fd, STDERR_FILENO); // redirect stderr
        close(fd);
    }

	
	if (execv(resolved_path, args) == -1) {
		print_error();
	}

	// free space created by command path checking
	free(resolved_path);
}

// Process one line of input
void process_line(char *line) {
	char *args[MAX_ARGS]; // an array of pointers to C strings, each string is a command or argument
    char *command_str; // holds each individual command string, separated by &
    int argc; // the number of arguments per command

	// stores state
    char *saveptr;
	// this splits the command line input into multiple commands, using the delimiter "&"
    command_str = strtok_r(line, "&", &saveptr);
	// stores child process IDs to make sure all commands are checked
    pid_t pids[MAX_ARGS];
	// child process counter
    int pid_count = 0;

	// runs into all commands are processed
    while (command_str != NULL) {
        // trims whitespace at start of command line inputs
		while (isspace(*command_str)) {
		    command_str++;
		}

		// trims whitespace at end of command line inputs
        char *end = command_str + strlen(command_str) - 1;
        while (end > command_str && isspace(*end)) {
            *end = '\0';
            end--;
        }

		// this checks for empty commands after trimming (indicates a syntax error)
        if (*command_str == '\0') {
            print_error();
            return;
        }

		// splits commands into individual arguments
        argc = parse_line(command_str, args);
        // skips empty commands
		if (argc == 0) {
		    print_error();
		    return;
		}

        //Will check for the redirection operation
        char *outputFile = NULL;
        for(int i =0; args[i] != NULL; i++){
            if(strcmp(args[i], ">") == 0){
                if(args[i+1] == NULL || args[i+2] != NULL){
                    print_error();
                    return;
                }
                outputFile = args[i+1];
                args[i] = NULL; // will terminate args before '>'
                break;
            }
        }

        if (handle_builtin(args)) {
            command_str = strtok_r(NULL, "&", &saveptr);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            print_error(); // Fork failed
        } else if (pid == 0) {
            execute_command(args, outputFile); // creates child process to execute command
            _exit(0);
        } else {
			 // store child pid for later
            pids[pid_count++] = pid;
        }

		// move to next command in user input/text file
        command_str = strtok_r(NULL, "&", &saveptr);
    }

    // this waits for all child processes to finish before returning back to shell
    for (int i = 0; i < pid_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}


// Interactive mode: read commands from user
void interactive_mode() {
    //line is a pointer to a character (a string in C), doesn't point to anywhere
    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("wish> ");               // Shell prompt
        //getline uses address of line to update the pointer to point to the memory where the user string is stored and uses &len to know the current buffer size that the string is going to be stored in an if more is needed will allocate
        if (getline(&line, &len, stdin) == -1) break; // EOF
	
	//direct to process the line
        process_line(line);
    }

    free(line);
}

// Batch mode: read commands from a file
//initializes a pointer called filename that holds characters of the actual file name
void batch_mode(char *filename) {

    //FILE is a c type representing an open file
    //*file is a pointer to a FILE object
    
    FILE *file = fopen(filename, "r");
    
    //if not a file then error displays
    if (!file) {
        print_error();
        exit(1);
    }

    
    char *line = NULL;
    size_t len = 0;
    
    //reads the contents of file
    while (getline(&line, &len, file) != -1) {
        process_line(line);
    }

    free(line);
    fclose(file);
}

//argc stores the total number of command line arguments when the program is executed
//argv (argument vector) is an array of pointers to an array of characters (that make up a string), each string represents a separate command line argument
//char* indicates that each element in argv is a pointer to the beginning of a c style string 

// Main function
int main(int argc, char *argv[]) {
    //initate path 
    init_default_path();

    if (argc == 1) {
        interactive_mode();     // No arguments -> interactive. this is when argc = 1 because the only thing you are running is ./wish which is the first argument (the program name)
    } else if (argc == 2) {
        batch_mode(argv[1]);    // One argument when running ./wish batch.txt enters batch mode
    } else {
        print_error();          // Too many arguments
        exit(1);
    }
    //cleanup
    free_path_dirs();
    return 0;
}

char* find_executable(char *cmd, char **path_dirs) {
	// run until path directory "string" (character) array is done
    for (int i = 0; path_dirs[i] != NULL; i++) {
		// gets length of full "string" ("+ 2" included for '/' + '\0')
        int len = strlen(path_dirs[i]) + strlen(cmd) + 2; 
        // allocate space for buffer creation
		char *full_path = malloc(len);
        // if buffer is empty
		if (!full_path){ 
			return NULL;
		}
		
		// writes text into a buffer (format: snprintf(buffer_variable, size of buffer, format of text, first string, second string) )
        snprintf(full_path, len, "%s/%s", path_dirs[i], cmd);

		// check if it's executable
        if (access(full_path, X_OK) == 0) {
            return full_path;
        }

		 // if not executable, free up the allocated space
        free(full_path);
    }
	// if command path is not found
    return NULL;
}
