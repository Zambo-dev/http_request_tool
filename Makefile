# Compiler settings
CC = clang
CCFLAGS := -lssl -lcrypto

# Files
CFILE = $(wildcard ./*.c)
EXE = httpreq

all: $(EXE)


$(EXE):
	@ $(CC) -o $@ $(CFILE) $(CCFLAGS)

clean:
	@ rm ./httpreq
