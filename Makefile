CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror

PROMPT = -DPROMPT
# variable declarations
CC = gcc
MAIN = sh.c
EXECS = 33sh 33noprompt

.PHONY: all clean
all: $(EXECS)
33sh: $(MAIN)
	$(CC) $(CFLAGS) -o -DPROMPT 33sh
33noprompt: $(MAIN)
	$(CC) $(CFLAGS) -o 33noprompt
clean:
	rm -f $(EXECS)
