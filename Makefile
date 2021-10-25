CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror

PROMPT = -DPROMPT
# variable declarations
CC = gcc
33sh = sh.c
33noprompt = sh.c
EXECS = $(33sh) $(33noprompt)

.PHONY: all clean
all: $(EXECS)
33sh: $(EXECS)
	$(CC) $(CFLAGS) -o PROMPT $(33sh)
33noprompt: $(EXECS)
	$(CC) $(CFLAGS) -o $(33noprompt)
clean:
	rm -f $(EXECS)
