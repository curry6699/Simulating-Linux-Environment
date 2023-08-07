////////////////////////////////////////////////////////////////////////
// Written by Prithvi Sajit  on 4/08/2021.
//

#include <sys/types.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// [[ TODO: put any extra `#include's here ]]

#include <spawn.h>
#include <limits.h>

// [[ TODO: put any `#define's here ]]


//
// Interactive prompt:
//     The default prompt displayed in `interactive' mode --- when both
//     standard input and standard output are connected to a TTY device.
//
static const char *const INTERACTIVE_PROMPT = "shuck& ";

//
// Default path:
//     If no `$PATH' variable is set in Shuck's environment, we fall
//     back to these directories as the `$PATH'.
//
static const char *const DEFAULT_PATH = "/bin:/usr/bin";

//
// Default history shown:
//     The number of history items shown by default; overridden by the
//     first argument to the `history' builtin command.
//     Remove the `unused' marker once you have implemented history.
//
static const int DEFAULT_HISTORY_SHOWN __attribute__((unused)) = 10;

//
// Input line length:
//     The length of the longest line of input we can read.
//
static const size_t MAX_LINE_CHARS = 1024;

//
// Special characters:
//     Characters that `tokenize' will return as words by themselves.
//
static const char *const SPECIAL_CHARS = "!><|";

//
// Word separators:
//     Characters that `tokenize' will use to delimit words.
//
static const char *const WORD_SEPARATORS = " \t\r\n";

// [[ TODO: put any extra constants here ]]


// [[ TODO: put any type definitions (i.e., `typedef', `struct', etc.) here ]]


static void execute_command(char **words, char **path, char **environment);
static void do_exit(char **words);
static int is_executable(char *pathname);
static char **tokenize(char *s, char *separators, char *special_chars);
static void free_tokens(char **tokens);

// [[ TODO: put any extra function prototypes here ]]
void posix_function(char *str, char **words, char **environment);
void history (int numbers, char **words);

int main (void)
{
    // Ensure `stdout' is line-buffered for autotesting.
    setlinebuf(stdout);

    // Environment variables are pointed to by `environ', an array of
    // strings terminated by a NULL value -- something like:
    //     { "VAR1=value", "VAR2=value", NULL }
    extern char **environ;

    // Grab the `PATH' environment variable for our path.
    // If it isn't set, use the default path defined above.
    char *pathp;
    if ((pathp = getenv("PATH")) == NULL) {
        pathp = (char *) DEFAULT_PATH;
    }
    char **path = tokenize(pathp, ":", "");

    // Should this shell be interactive?
    bool interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);

    // Main loop: print prompt, read line, execute command
    while (1) {
        // If `stdout' is a terminal (i.e., we're an interactive shell),
        // print a prompt before reading a line of input.
        if (interactive) {
            fputs(INTERACTIVE_PROMPT, stdout);
            fflush(stdout);
        }

        char line[MAX_LINE_CHARS];
        if (fgets(line, MAX_LINE_CHARS, stdin) == NULL)
            break;

        // Tokenise and execute the input line.
        char **command_words =
            tokenize(line, (char *) WORD_SEPARATORS, (char *) SPECIAL_CHARS);
        execute_command(command_words, path, environ);
        free_tokens(command_words);
    }

    free_tokens(path);
    return 0;
}


//
// Execute a command, and wait until it finishes.
//
//  * `words': a NULL-terminated array of words from the input command line
//  * `path': a NULL-terminated array of directories to search in;
//  * `environment': a NULL-terminated array of environment variables.
//
static void execute_command(char **words, char **path, char **environment)
{
    assert(words != NULL);
    assert(path != NULL);
    assert(environment != NULL);

    char *program = words[0];

    if (program == NULL) {
        // nothing to do
        return;
    }

    if (strcmp(program, "exit") == 0) {
        do_exit(words);
        // `do_exit' will only return if there was an error.
        return;
    }

    // [[ TODO: add code here to implement subset 0 ]]
    // code for cd command
    // Check if user is executing cd
    if (strcmp(program, "cd") == 0) {
        // Check if there are no arguments
        if (words[1] == NULL) {
            // Change to directory HOME since there are no arguments
            chdir(getenv("HOME"));

        } else if(words[2] != NULL) {
            // If cd has more than 1 argument it is invalid, print this to standard error
            fprintf(stderr, "%s: too many arguments\n", program);

        } else if(chdir(words[1]) == 0) {
            // cd has been successfully executed

        } else {
            // If any other error occurs print to standard error
            fprintf(stderr, "cd: %s: ", words[1]);
            perror("");

        }
        
        return;
        // End of cd code
    }
    
    // code for pwd command
    if (strcmp(program, "pwd") == 0) {
        char pathname[PATH_MAX];

        // Check if pwd has any arguments
        if (words[1] != NULL) {
            // Print to standard error since pwd should have no arguments
            fprintf(stderr, "%s: Incorrect usage, too many arguments\n", program);

        } else if(getcwd(pathname, sizeof(pathname)) == NULL) {
            // If any other error occurs use perror
            perror("pwd:");

        } else {
            // If there are no errors print the current directory
            printf("current directory is '%s'\n", pathname);

        }
        
        return;
        // End of pwd code
    }


    if (strcmp(program, "history") == 0) {
        int number = 10;
        if (words[1] == NULL) {
            history(number, words); 
        } else if (words[2] != NULL) {
            fprintf(stderr, "%s: too many arguments\n", program);
        } else if (words[1] != NULL) {
            number = atoi(words[1]);
            history(number, words);
        }
        return;
    }

    // [[ TODO: change code below here to implement subset 1 ]]
    // Check if / or . is used
    if (program[0] == '/' || program[0] == '.' ) {
        // bool to determine if executable
        bool exec = is_executable(program);
        
        if(exec) {
            // If it is executable, run it
            posix_function(program, words, environment);

        } else {
            // If the command is not found print to standard error
            fprintf(stderr, "%s: command not found\n", words[0]);

        }
        
        return;

    } else {
        // If l or . is not used find the path of input
        int i = 0;
        while(path[i] != NULL) {
            // Calculate total length
            int path_length = (strlen(path[i]) + strlen(words[0]) + 2);

            // Create string which contains the path
            char str[path_length];
            snprintf(str, sizeof(str), "%s/%s", path[i], words[0]);

            // Check if executable
            bool exec = is_executable(str);

            // If it is executable use the function posix_function to execute 
            if(exec) {
                posix_function(str, words, environment);
                
                return;
            }
            i++;
        }
        
        // Print to standard error if command is not found
        fprintf(stderr, "%s: command not found\n", words[0]); 
    }

}
// Uses posix_spawn to execute a command 
// Returns nothing

void posix_function(char *str, char **words, char **environment) {
    pid_t pid;
    int exit_status;
    // Calling posix_spawn, if there is an error use perror
    if (posix_spawn(&pid, str, NULL, NULL, words, environment) != 0) {
            perror("spawn");
    }

    // Waiting till proccess is completed, if there is an error use perror
    if (waitpid(pid, &exit_status, 0) == -1) {
        perror("waitpid");
    }

    // Print the exit status 
    printf("%s exit status = %d\n", str, WEXITSTATUS(exit_status));
    return;
}


// Did not finish implementing
// Was supposed to save history

void history (int number, char **words) {
    
    char *envi = getenv("HOME");
        
    // Calculate total length
    int path_length = (strlen(envi) + strlen("/.shuck_history") + 1);
    char str[path_length];

    snprintf(str, sizeof(str), "%s/.shuck_history", envi);
    printf("%s", str);

    if (number == -1) {
        FILE *stream = fopen(str, "r+");
        int i = 0;
        while(words[i] != NULL) {
            fprintf(stream, "%s ", words[i]);
            i++;
        }
        fprintf(stream, "\n");
        fclose(stream);
        return;
    } else {
        FILE *stream = fopen(str, "r+");
        /*
        int i = 0;
        while (words[i] != NULL) {
            fprintf(stream, "%s ", words[i]);
            i++;
        }
        fprintf(stream, "\n");
        */
        int total_lines = 0;
        char total[100];
        while (fgets(total, sizeof(total), stream) != NULL) {
            total_lines++;
        }
        

        if (number != 10) {
            int nth_line = total_lines - number;
            fseek(stream, 0, SEEK_SET);
            int i = 0;
            while (fgets(total, sizeof(total), stream) != NULL) {
                
                char ifstr[BUFSIZ];
                fgets(ifstr, sizeof(ifstr), stream);
                if (i >= nth_line) {
                    printf("here");
                    printf("%d: %s", i, ifstr);
                }
                i++;
            } 
        } else {
            int nth_line = total_lines - 10;
            
            fseek(stream, 0, SEEK_SET);
            int i = 0;
            while (fgets(total, sizeof(total), stream) != NULL) {
                
                char elsestr[BUFSIZ];
                fgets(elsestr, sizeof(elsestr), stream);
                if (i >= nth_line) {
                    printf("here");
                    printf("%d: %s", i, elsestr);
                }
                i++;
            }
        }
        fclose(stream);
        return;
    }
    
}


/*void print_history(int number) {
    char *envi = getenv("HOME");
        
    // Calculate total length
    int path_length = (strlen(envi) + strlen("/.shuck_history") + 1);
    char str[path_length];

    snprintf(str, sizeof(str), "%s/.shuck_history", envi);

    FILE *stream = fopen(str, "r");
    if (stream == NULL) {
        // error opening 
    }
    int i = 0;
    while (words[i] != NULL) {
        fprintf(stream, "%s ", words[i]);
        i++;
    }
    fprintf(stream, "\n");


    int total_lines = 0;
    char total[BUFSIZ];
    while (fgets(total, sizeof(total), stream) != NULL) {
        total_lines++;
    }

    int nth_line = total_lines - number;
    fseek(stream, 0, SEEK_SET);
    
    
    if (number != 10) {
        int i = 0;
        while (fgets(total, sizeof(total), stream) != NULL) {
            char ifstr[BUFSIZ];
            fgets(ifstr, sizeof(ifstr), stream);
            if (i >= nth_line) {
                printf("%d: %s", i, ifstr);
            }
            i++;
        } 
    } else {
        nth_line = total_lines - 10;
        fseek(stream, 0, SEEK_SET);
        int i = 0;
        while (fgets(total, sizeof(total), stream) != NULL) {
            char elsestr[BUFSIZ];
            fgets(elsestr, sizeof(elsestr), stream);
            if (i >= nth_line) {
                printf("%d: %s", i, elsestr);
            }
            i++;
        }
    }
    fclose(stream);
    return;
}*/

/*void history (char **words) {
    char *envi = getenv("HOME");
        
    // Calculate total length
    int path_length = (strlen(envi) + strlen("/.shuck_history") + 1);
    char str[path_length];

    snprintf(str, sizeof(str), "%s/.shuck_history", envi);

    FILE *stream = fopen(str, "r+"); 

    int i = 0;
    while (words[i] != NULL) {
        fprintf(stream, "%s ", words[i]);
        i++;
    }
    fprintf(stream, "\n");
    fclose(stream);
} */
//
// Implement the `exit' shell built-in, which exits the shell.
//
// Synopsis: exit [exit-status]
// Examples:
//     % exit
//     % exit 1
//
static void do_exit(char **words)
{
    assert(words != NULL);
    assert(strcmp(words[0], "exit") == 0);

    int exit_status = 0;

    if (words[1] != NULL && words[2] != NULL) {
        // { "exit", "word", "word", ... }
        fprintf(stderr, "exit: too many arguments\n");

    } else if (words[1] != NULL) {
        // { "exit", something, NULL }
        char *endptr;
        exit_status = (int) strtol(words[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "exit: %s: numeric argument required\n", words[1]);
        }
    }

    exit(exit_status);
}


//
// Check whether this process can execute a file.  This function will be
// useful while searching through the list of directories in the path to
// find an executable file.
//
static int is_executable(char *pathname)
{
    struct stat s;
    return
        // does the file exist?
        stat(pathname, &s) == 0 &&
        // is the file a regular file?
        S_ISREG(s.st_mode) &&
        // can we execute it?
        faccessat(AT_FDCWD, pathname, X_OK, AT_EACCESS) == 0;
}


//
// Split a string 's' into pieces by any one of a set of separators.
//
// Returns an array of strings, with the last element being `NULL'.
// The array itself, and the strings, are allocated with `malloc(3)';
// the provided `free_token' function can deallocate this.
//
static char **tokenize(char *s, char *separators, char *special_chars)
{
    size_t n_tokens = 0;

    // Allocate space for tokens.  We don't know how many tokens there
    // are yet --- pessimistically assume that every single character
    // will turn into a token.  (We fix this later.)
    char **tokens = calloc((strlen(s) + 1), sizeof *tokens);
    assert(tokens != NULL);

    while (*s != '\0') {
        // We are pointing at zero or more of any of the separators.
        // Skip all leading instances of the separators.
        s += strspn(s, separators);

        // Trailing separators after the last token mean that, at this
        // point, we are looking at the end of the string, so:
        if (*s == '\0') {
            break;
        }

        // Now, `s' points at one or more characters we want to keep.
        // The number of non-separator characters is the token length.
        size_t length = strcspn(s, separators);
        size_t length_without_specials = strcspn(s, special_chars);
        if (length_without_specials == 0) {
            length_without_specials = 1;
        }
        if (length_without_specials < length) {
            length = length_without_specials;
        }

        // Allocate a copy of the token.
        char *token = strndup(s, length);
        assert(token != NULL);
        s += length;

        // Add this token.
        tokens[n_tokens] = token;
        n_tokens++;
    }

    // Add the final `NULL'.
    tokens[n_tokens] = NULL;

    // Finally, shrink our array back down to the correct size.
    tokens = realloc(tokens, (n_tokens + 1) * sizeof *tokens);
    assert(tokens != NULL);

    return tokens;
}


//
// Free an array of strings as returned by `tokenize'.
//
static void free_tokens(char **tokens)
{
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
