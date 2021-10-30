#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

// function declarations
void parse_helper(char buffer[1024], char *tokens[512], char *argv[512], char r[20]);
int parse(char buffer[1024], char *tokens[512], char *argv[512], char *w_sym[512], char** input_file, char** output_file, int output_flags, char** path);
int built_in(char *argv[512]);
int cd(char *dir);
int ln(char *src, char *dest);
int rm(char *file);
int file_redirect(char buffer[1024], char** input_file, char** output_file, int output_flags);
int set_path(char *tokens[512], char** path);
// close stdin
// open given file
// if no input/output file
// after that you close stdin
// open , if its an input, that file using read only
// need to be able to READ it to allow execv
// if its a write file (output)
// depending on which output, set the flags one way
//o append o trunc
// set the flags
// open thouse
// open files AFTER you run fork
// between running fork and execv
// fork into child and open child so child can run commands
// call parse in the main (parent)
// in the child is where you read/write
// execv is going to take in argv array and pointers to input file and output file
int main() {
    //repl (read eval print loop)
    while (1) {
    #ifdef PROMPT 
    if (printf("33sh> ") < 0) { 
        fprintf(stderr, "error: unable to write");
        return 1;
    }
    if (fflush(stdout) < 0) {
        perror("fflush");
        return 1; // only return zero when it works!
    }
    #endif

    // initializing
    char buf[1024];
    int fd = STDIN_FILENO;
    size_t count = 1024;
    ssize_t to_read;
    // reading system call to get user input
    to_read = read(fd, buf, count);
    // error-checking
    // check if its equal to \n or spaces 
    // check argv array (case where there are no tokens)
    if (to_read == -1) { 
        perror("error: read");
        return 1;
    }
    else if (to_read == 0) { // restart program
        return 1; // 0 means end of file
    }
    buf[to_read] = '\0'; // since the read function does not null-terminate the buffer
    char *tokens[512];
    char *argv[512];
    char *w_sym[512];
    char *path = NULL;
    char *input_file = NULL;
    char *output_file = NULL;
    int output_flags; // flag is set to 2 if flag = O_APPEND, and 1 if flag = O_TRUNC
    int parse_result = parse(buf,tokens,argv,w_sym, &input_file, &output_file, output_flags, &path);
    if (argv[0] == NULL) {
        return 1;
    }
    if (parse_result == 0) {
        continue;
    }
    int built_ins = built_in(argv);
    // if (built_ins == -1) {
    //     continue; // dont fork or execv, would fail automaticallly and exit out
    //     // p error out
    // } // don't need this
    if (built_ins == 0) {
    pid_t pid;
    if ((pid = fork()) == 0) { // enters child process
        int redirects = file_redirect(buf, &input_file, &output_file, output_flags);
        if (redirects == -1) { // if an error has occured
            continue; 
        }
        int exec = execv(path, argv);
        if (exec == -1) {
            perror("execv");
        }
        perror("child process could not do execv");
    }
    else if (pid > 0) { // enters wait mode
        wait(NULL);
    }
    else { // if an error has ocurred
        perror("error calling function fork()");
        exit(0);
    }
    }
    }
   return 0;
}

// write descr
void parse_helper(char buffer[1024], char *tokens[512], char *argv[512], char r[20]) {
    char *temp; // temp string to hold values
    int n = 0;
    temp = strtok(buffer, r); // tokenizes temp char, only returns first token
    while (temp !=NULL) { // loop through the temp string in order to find all tokens
        tokens[n] = temp; // value stored in tokens array
        temp = strtok(NULL,r); // goes to next character in string that is not whitespace
        n++;
    }
    if (tokens[0] == NULL) {
        argv[0] = NULL;
    }
    char *first = strtok(buffer, r); // gets first token (binary name)
      if (first == NULL) { // base case
        argv[0]= NULL;
      }
      else {
      const char slash = '/'; 
      char *last = strrchr(first,slash); // returns pointer to the last occurence of the slash 
      if (last == NULL){ // if there are no slashes in the first arg
        argv[0] = first;
      }
      else {
        last++; // otherwise, goes to the part of the token that follows the last slash
        if (last == NULL) {
            argv[0] = ""; // if there is no other arg, put in an empty string
          }
      else {
        argv[0]= last; //otherwise, set first argument equal to the following one
      }
      }
      int index = 1;
      while (tokens[index]!= NULL) { // looping through tokens string
        argv[index] = tokens[index]; // sets arguments equal to the appropriate tokens
        index++; // increments index
        }
      }
    }

// write descr
// returns 0 when done
int set_path(char *tokens[512], char** path) {
    int i = 0;
    while (tokens[i] != NULL) {
        if ((strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],">>") != 0)) {
            *path = tokens[i];
            return 0;
        }
        else { // if the current index is a symbol
            i++; // skip over an index (the file)
        }
    i++;
    }
    return 0;
}


// write descr
// returns 0 if it failed, 1 otherwise
int parse(char buffer[1024], char *tokens[512], char *argv[512], char *w_sym[512], char** input_file, char** output_file, int output_flags, char** path) {
    int i = 0; // index for tokens
    int k = 0; // index for argv array
    int flag1 = 0;
    int flag2 = 0;
    char r1[3] = {' ','\t','\n'}; // characters to tokenize
    parse_helper(buffer,tokens,w_sym,r1);
    while (tokens[i] != NULL) { // looping through tokens array
        if (strcmp(tokens[i],"<") == 0) {
            // error check first
            flag1++; // set flag to 1- meaning that it was found
            if (flag1 >1) { // if input redirect appeared 2x
                fprintf(stderr, "Can’t have two input redirects on one line.");
                return 0; 
            }
            if (tokens[i+1] == NULL) {
                fprintf(stderr, "No redirection file specified.");
                return 0;
            }
            if (strcmp(tokens[i+1],">") == 0) {
                fprintf(stderr, "No redirection file specified.");
                return 0;
            }
            if (strcmp(tokens[i+1],">>") == 0) { 
                fprintf(stderr, "No redirection file specified.");
                return 0;
            }
            // after error checking is complete
            // input_index = i;
            *input_file = tokens[i+1];
            argv[k] = w_sym[i+2];
            k++;
            // cd /usr/bin/something 
            // get stuff after forward slash
        }
        else if (strcmp(tokens[i],">") == 0) { 
            // error check first
            flag2++; // set flag to 1- meaning that it was found
            output_flags = 2; // O_TRUNC
            if (flag2 >1) { // if output redirect appeared 2x
            fprintf(stderr, "Can’t have two output redirects on one line.");
            return 0;
            }
            if (tokens[i+1] == NULL) {
            fprintf(stderr, "No redirection file specified.");
            return 0;
            }
            // after error checking is complete
            // output_index = i;
            *output_file = tokens[i+1];
            argv[k] = w_sym[i+2];
            k++;
        }
        else if (strcmp(tokens[i],">>") == 0) {
            // error check first
            flag2++; // set flag to 1- meaning that it was found
            output_flags = 2; // O_APPEND
            if (flag2 >1) { // if output redirect appeared 2x
            fprintf(stderr, "Can’t have two output redirects on one line.");
            return 0;
            }
            if (tokens[i+1] == NULL) {
            fprintf(stderr, "No redirection file specified.");
            return 0;
            }
            // after error checking is complete
            // output_index = i;
            *output_file = tokens[i+1];
            argv[k] = w_sym[i+2];
            k++;
        }
        else {  // otherwise, then add in element to argv
            argv[k] = w_sym[i]; 
            k++;
        }
        i++;
    }
    if (flag1 != 1 && flag2 != 1) {
        *path = tokens[0];
    }
    else  {
        set_path(tokens, path);
    }
// check whether you have mult of teh same input/output redirection


    // go through tokens array and redirectit
    // tokens has echo hello > output.txt test
    // record the symbol is ther so we know that something occured there (get index of symbol so we know wehre it is)
    // argv array and ignore the redirect
    // have 3 variables representing input/output append indices
    // if index exists then update it
    //-1 -7
    // if token at index i == redirect, then do output index = i
    // just want to record where in token array they appear
    // once we have them, only include indices that aren't redirects
    // token after redirect is filename we want to redirect to- dont include that
    // do for each input/output index
    // in order to get indices of wherr tokens are >> use strcmp , but not when making argv array


    return 1;
}

// write descr
// returns -1 if an error occured, 0 otherwise
int file_redirect(char buffer[1024], char** input_file, char** output_file, int output_flags) {
    if (input_file != NULL) { // if there is an input file
    int closed = close(STDIN_FILENO);
    if (closed != 0) {
        perror("error: close");
        return -1;
    }
        int open_descr = open(*input_file, O_RDONLY); // open file to read
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        // ssize_t read_descr = read(open_descr, buffer, 1024);
        // if (read_descr == -1) {
        //     perror("error: read");
        //     return -1;
        // }
        // what do i do after I call read?
    }
    if ((output_file != NULL) && (output_flags == 1)) { // if there is an output file to truncate
        int closed = close(STDOUT_FILENO);
        if (closed != 0) {
        perror("error: close");
        return -1;
        } 
        int open_descr = open(*output_file, O_CREAT|O_WRONLY|O_TRUNC, 0644); // open file to read
        // is the mode correct
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        // ssize_t write_descr = write(open_descr, buffer, 1024); 
        // // close stdin
        // // next file has same #
        // if (write_descr == -1) {
        //     perror("error: read");
        //     return -1;
        // }
    }
    if ((output_file != NULL) && (output_flags == 2)) { // if there is an output file to append
        int closed = close(STDOUT_FILENO);
        if (closed != 0) {
        perror("error: close");
        return -1;
        }
        int open_descr = open(*output_file, O_WRONLY|O_CREAT|O_APPEND, 0666); // open file to read
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        // ssize_t write_descr = write(open_descr, buffer, 1024);
        // if (write_descr == -1) {
        //     perror("error: read");
        //     return -1;
        // }
    }
    return 0;
}

//write descr
// returns -1 if error, 1 if successful, 0 if none given
int built_in(char *argv[512]) {
    if (strcmp(argv[0], "cd") ==0) { // if the command is cd
        // char *dir = argv[1];
         // pass in elt after 'cd'
        if (argv[1] == NULL) {
            fprintf(stderr, "error: no directory");
        }
        else if (chdir(argv[1]) == -1) {
            perror(argv[0]);
        }
        return 1;
    }
    else if (strcmp(argv[0], "ln") ==0) { // if the command is ln
        if (argv[1] == NULL)  {
            fprintf(stderr, "error: no source");
        }
        else if (argv[2] == NULL) {
            fprintf(stderr, "error: no output");
        }
        else if (link(argv[1], argv[2]) != 0) { // error checking
            perror("error: failed to link");
            return -1;
        }
        return 1;
    }
    else if (strcmp(argv[0], "rm") ==0) { // if the command is rm
        if (argv[1] == NULL)  {
            fprintf(stderr, "error: no source");
        }
        else if (unlink(argv[1]) != 0) {
            perror("error: unable to delete the file");
            return -1;
        }
        return 1;
    }
    else if (strcmp(argv[0], "exit") ==0) { // if the command is exit
        exit(0);
    }
    return 0;
}