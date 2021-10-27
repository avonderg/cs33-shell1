#include <stddef.h>
#include <string.h>

int main() {
    //repl (read eval print loop)
    while (1) {
    // initializing
    char buf[1024];
    int n;
    int fd;
    size_t count;
    int to_read;
    // reading system call to get user input
    to_read = read(fd, buf, count);
    // error-checking
    if (to_read == -1) { 
        perror("read");
        return 0;
    }
    else if (to_read == 0) { // if they hit return, end program / restart
        return 0;
    }
    buf[to_read] = NULL; // since the read function does not null-terminate the buffer
    char *tokens[512];
    char *argv[512];
    char *w_sym[512];
    char *inputs[512];
    char *outputs[512];
    int result = parse(buf,tokens,argv, w_sym,inputs,outputs);
    if (result == 0) {
        continue;
    }
    // first elt in = command



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
// returns 0 if it failed, 1 otherwise
int parse(char buffer[1024], char *tokens[512], char *argv[512], char *w_sym[512], char *inputs[512], char *outputs[512]) {
    int i = 0; // index for tokens
    int j =0; // index for input/output arrays
    int k = 0; // index for argv array
    int flag1 = 0;
    int flag2 = 0;
    const char r1[2] = {' ','\t'}; // characters to tokenize
    parse_helper(buffer,tokens,w_sym,r1);
    while (tokens[i] != NULL) {
        // check redirect followed by a token
        // no redirect file
        if (strcmp(tokens[i],'<') == 0) {
            // error check first
            flag1++; // set flag to 1- meaning that it was found
            if (flag1 >1) { // if input redirect appeared 2x
            fprintf("Can’t have two input redirects on one line.");
            return 0; 
            }
            if (tokens[i+1] == NULL) {
            fprintf("No redirection file specified.");
            return 0;
            // return a value showing it succeeded 
            // continue;
            }
            if (tokens[i+1] == '>') {
            fprintf("No redirection file specified.");
            return 0;
            }
            if (tokens[i+1] == '>>') {
            fprintf("No redirection file specified.");
            return 0;
            }
            // after error checking is complete
            inputs[j] = i; // inputs index of the redirect
            inputs[j+1] = tokens[i+1]; // followed by filename
            j++;
        }
        else if (strcmp(tokens[i],'>') == 0) { 
            // error check first
            flag2++; // set flag to 1- meaning that it was found
            if (flag2 >1) { // if output redirect appeared 2x
            fprintf("Can’t have two output redirects on one line.");
            return 0;
            }
            if (tokens[i+1] == NULL) {
            fprintf("No redirection file specified.");
            return 0;
            }
            // after error checking is complete
            outputs[j] = i; // inputs index of the redirect
            outputs[j+1] = tokens[i+1]; // followed by filename
            j++;
        }
        else if (strcmp(tokens[i],'>>') == 0) {
            // error check first
            flag2++; // set flag to 1- meaning that it was found
            if (flag2 >1) { // if output redirect appeared 2x
            fprintf("Can’t have two output redirects on one line.");
            return 0;
            }
            if (tokens[i+1] == NULL) {
            fprintf("No redirection file specified.");
            return 0;
            }
            // after error checking is complete
            outputs[j] = i; // inputs index of the redirect
            outputs[j+1] = tokens[i+1]; // followed by filename]
            j++;
        }
        else {  // otherwise, then add in element to argv
            argv[k] = w_sym[i];
            k++;
        }
        i++;
    }



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


void built_in() {
    ch
}