.POSIX:
.PHONY: clean

PROJ = my_string
LIBPROJ = lib$(PROJ).so
MYCFLAGS = --std=c89 -O2 -Wall -Wextra -Wpedantic -shared


all: $(LIBPROJ)

clean:
	rm lib$@.so

$(LIBPROJ): my_string.c
	$(CC) $(MYCFLAGS) $(CFLAGS) $(MYLDFLAGS) $(LDFLAGS) -o $@ $?
