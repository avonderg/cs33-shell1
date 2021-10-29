#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

// function declarations
void parse_helper(char buffer[1024], char *tokens[512], char *argv[512], char r[20], const char path[30]);
int parse(char buffer[1024], char *tokens[512], char *argv[512], char *w_sym[512], char input_file[30], char output_file[30], int output_flags, const char path[30]);
int built_in(char *argv[512]);
int cd(char *dir);
int ln(char *src, char *dest);
int rm(char *file);
int file_redirect(char buffer[1024], char input_file[30], char output_file[30], int output_files);

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
    char *buf[1024];
    int fd = STDIN_FILENO;
    size_t count = 1024;
    ssize_t to_read;
    // reading system call to get user input
    to_read = read(fd, buf, count);
    // error-checking
    if (to_read == -1) { 
        perror("error: read");
        return 1;
    }
    else if (to_read == 0) { // restart program
        return 0;
    }
    buf[to_read] = '\0'; // since the read function does not null-terminate the buffer
    char *tokens[512];
    char *argv[512];
    char *w_sym[512];
    // int input_index;
    // int output_index;
    const char *path[30];
    char *input_file[30];
    char *output_file[30];
    int output_flags; // flag is set to 2 if flag = O_APPEND, and 1 if flag = O_TRUNC
    int parse_result = parse(*buf,tokens,argv,w_sym, *input_file, *output_file, output_flags, *path);
    if (parse_result == 0) {
        continue;
    }
    int built_ins = built_in(argv);
    if (built_ins != 0) {
        continue; // dont fork or execv, would fail automaticallly and exit out
    }
    int redirects = file_redirect(*buf, *input_file, *output_file, output_flags);
    if (redirects == -1) { // if an error has occured
        continue; 
    }
    pid_t pid;
    if ((pid = fork()) == 0) { // enters child process
        int exec = execv(*path, argv);
        if (exec == -1) {
            perror("execv");
        }
        perror("child process could not do execv");
    }
    else if ((pid = fork()) != -1) { // enters wait mode
        wait(NULL);
    }
    else { // if an error has ocurred
        perror("error calling function fork()");
    }
    }
   return 0;
}

// write descr
void parse_helper(char buffer[1024], char *tokens[512], char *argv[512], char r[20], const char path[30]) {
    char *temp; // temp string to hold values
    int n = 0;
    temp = strtok(buffer, r); // tokenizes temp char, only returns first token
    while (temp !=NULL) { // loop through the temp string in order to find all tokens
        tokens[n] = temp; // value stored in tokens array
        temp = strtok(NULL,r); // goes to next character in string that is not whitespace
        n++;
    }
    char *first = strtok(buffer, r); // gets first token (binary name)
    path = first; // sets path equal to the first token
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
// returns 0 if it failed, 1 otherwise
int parse(char buffer[1024], char *tokens[512], char *argv[512], char *w_sym[512], char input_file[30], char output_file[30], int output_flags, const char path[30]) {
    int i = 0; // index for tokens
    int k = 0; // index for argv array
    int flag1 = 0;
    int flag2 = 0;
    char r1[2] = {' ','\t'}; // characters to tokenize
    input_file = "\0";
    output_file = "\0";
    parse_helper(buffer,tokens,w_sym,r1, path);
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
            input_file = tokens[i+1];
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
            output_file = tokens[i+1];
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
            output_file = tokens[i+1];
        }
        else {  // otherwise, then add in element to argv
            argv[k] = w_sym[i];
            k++;
        }
        i++;
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
int file_redirect(char buffer[1024], char input_file[30], char output_file[30], int output_flags) {
    int closed = close(STDIN_FILENO);
    if (closed != 0) {
        perror("error: close");
        return -1;
    }
    if (strcmp(input_file, "\0") !=0) { // if there is an input file
        int open_descr = open(input_file, O_RDONLY); // open file to read
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        ssize_t read_descr = read(open_descr, buffer, 1024);
        if (read_descr == -1) {
            perror("error: read");
            return -1;
        }
        // what do i do after I call read?
    }
    if ((strcmp(output_file, "\0") !=0) && (output_flags == 1)) { // if there is an output file to truncate
        int open_descr = open(output_file, O_CREAT|O_WRONLY|O_TRUNC, 0644); // open file to read
        // is the mode correct
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        ssize_t write_descr = write(open_descr, buffer, 1024);
        if (write_descr == -1) {
            perror("error: read");
            return -1;
        }
    }
    if ((strcmp(output_file, "\0") !=0) && (output_flags == 2)) { // if there is an output file to append
        int open_descr = open(output_file, O_WRONLY|O_CREAT|O_APPEND, 0666); // open file to read
        if (open_descr == -1) {
            perror("error: open");
            return -1;
        }
        ssize_t write_descr = write(open_descr, buffer, 1024);
        if (write_descr == -1) {
            perror("error: read");
            return -1;
        }
        // do i do anything after or do i have to write differently for this redirect
    }
    return 0;
}

//write descr
// returns -1 if error, 1 if no dir given, 0 if successful
int built_in(char *argv[512]) {
    if (strcmp(argv[0], "cd") ==0) { // if the command is cd
        char *dir = argv[1];
        if (dir == NULL) {
            cd(getenv("HOME")); // go to home directory if no path given
            return 1;
        }
        int cd_res = cd(dir); // pass in elt after 'cd'
        if (cd_res<0) { // error checking
            perror("error: no such directory");
            return -1;
        }
    }
    else if (strcmp(argv[0], "ln") ==0) { // if the command is ln
        char *src = argv[1];
        char *dest = argv[2];
        int ln_res = ln(src,dest); // pass in args 1,2
        if (ln_res != 0) { // error checking
            perror("error: failed to link");
            return -1;
        }
    }
    else if (strcmp(argv[0], "rm") ==0) { // if the command is rm
        char *file = argv[1];
        int rm_res = rm(file); // pass in arg1
        if (rm_res != 0) { // error checking
            perror("error: unable to delete the file");
            return -1;
        }
    }
    else if (strcmp(argv[0], "exit") ==0) { // if the command is exit
        exit(0);
    }
    return 0;
}

// given a pointer to the input directory, changes the current working directory 
int cd(char *dir) {
    return chdir(dir); // returns 0 if change of directory was successful, -1 otherwise
}

// given a source and destination, makes a hard link to a file
int ln(char *src, char *dest) {
   return link(src,dest);
}

// removes given input file from a directory using a pointer to the file
int rm(char *file) {
    return remove(file); // unlink or remove???
}

// write descr
void handle_child() {

}