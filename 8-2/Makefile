.POSIX:
.SUFFIXES:
.PHONY: lib$(PROJ).so test testbin


PROJ = manipcr
SOURCES = addcr.c removecr.c misc.c
TESTSOURCES = testcr.c $(SOURCES)
MYCFLAGS = -D _POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic
COMPILER = $(CC) $(MYCFLAGS) $(CFLAGS) $(MYLDFLAGS) $(LDFLAGS) -o $@


all: lib$(PROJ).so

lib$(PROJ).so:
	$(COMPILER) $(SOURCES)

test: testbin
	./testbin

testbin:
	$(COMPILER) $(TESTSOURCES)
