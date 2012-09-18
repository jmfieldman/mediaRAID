CC=gcc
CFLAGS= -std=c99 -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse
LDFLAGS=-lfuse
EXECUTABLE=mediaRAIDd
SRCDIR=../src/

SOURCES=  $(SRCDIR)data_structs.c     \
          $(SRCDIR)exlog.c            \
          $(SRCDIR)files.c            \
          $(SRCDIR)fuse_multiplex.c   \
          $(SRCDIR)httpd.c            \
          $(SRCDIR)mediaRAID.c        \
          $(SRCDIR)replication.c      \
          $(SRCDIR)simplehash.c       \
          $(SRCDIR)volumes.c

OBJECTS=$(SOURCES:.c=.o)


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.c 
	$(CC) $(CFLAGS) $< -c -o $@


clean:
	rm -rf $(SRCDIR)*.o

