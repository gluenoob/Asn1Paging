CC = gcc
# Add -Ilib/ to CFLAGS to include the lib directory for header file searches
CFLAGS = -w -lpthread -Ilib/ -g

SRCDIR = src
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

# List all .c files in the lib directory, replace with actual files as needed
LIB_SOURCES = $(wildcard $(LIBDIR)/*.c)
# Convert .c files to .o files in the obj directory
LIB_OBJECTS = $(patsubst $(LIBDIR)/%.c,$(OBJDIR)/%.o,$(LIB_SOURCES))

GNB_SRC = $(SRCDIR)/gnb.c
UE_SRC = $(SRCDIR)/ue.c

# Update object file paths to use OBJDIR
GNB_OBJ = $(OBJDIR)/gnb.o
UE_OBJ = $(OBJDIR)/ue.o

all: directories $(BINDIR)/gnb $(BINDIR)/ue

$(BINDIR)/gnb: $(GNB_OBJ) $(LIB_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(BINDIR)/ue: $(UE_OBJ) $(LIB_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(GNB_OBJ): $(GNB_SRC)
	$(CC) -c -o $@ $< $(CFLAGS)

$(UE_OBJ): $(UE_SRC)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile lib .c files to .o files
$(OBJDIR)/%.o: $(LIBDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# New target to ensure the obj/ directory exists
directories:
	mkdir -p $(OBJDIR) $(BINDIR)

clean:
	rm -f $(BINDIR)/gnb $(BINDIR)/ue $(GNB_OBJ) $(UE_OBJ) $(LIB_OBJECTS)

.PHONY: all clean directories