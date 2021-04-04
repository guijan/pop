.POSIX:
.PHONY: $(PROJ)

SOURCES = test_my_memset.c my_memset.c
PROJ = test_my_memset


all: $(PROJ)

clean:
	rm $(PROJ)

$(PROJ):
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

