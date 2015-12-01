######################################################
# Makefile for libslnc
# Ye Li
# leeyee.seu@gmail.com
######################################################

TOP = .
SRCDIR := src
OBJDIR := src
INCLUDEDIR = include src
INC_PARMS = $(INCLUDEDIR:%=-I%)

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	SED = gsed
	CC  = gcc-5
endif
ifeq ($(UNAME), Linux)
	SED = sed
	CC  = gcc
endif

#CC = gcc
CFLAGS0 = -Winline -std=c99 -lm
CFLAGS1 = -O3 -DNDEBUG $(INC_PARMS)  -mssse3 -DINTEL_SSSE3
#CFLAGS2 = -lm
#CFLAGS = -O3 -I$(INCLUDEDIR) -mssse3 -DINTEL_SSSE3
#CFLAGS = -std=c99 -g -lm

vpath %.h src include
vpath %.c src examples

DEFS   := common.h bipartite.h slncEncoder.h slncDecoder.h galois.h decoderGG.h decoderOA.h decoderBD.h decoderCBD.h
GNCENC  := $(OBJDIR)/common.o $(OBJDIR)/bipartite.o $(OBJDIR)/slncEncoder.o $(OBJDIR)/galois.o $(OBJDIR)/gaussian.o
RECODER := $(OBJDIR)/slncRecoder.o 
DECODER := $(OBJDIR)/slncDecoder.o
GGDEC   := $(OBJDIR)/decoderGG.o 
OADEC   := $(OBJDIR)/decoderOA.o $(OBJDIR)/pivoting.o
BDDEC   := $(OBJDIR)/decoderBD.o $(OBJDIR)/pivoting.o
CBDDEC  := $(OBJDIR)/decoderCBD.o

.PHONY: all
all: decoder.example 

libslnc.so: $(GNCENC) $(GGDEC) $(OADEC) $(BDDEC) $(CBDDEC) $(RECODER) $(DECODER)
	$(CC) -shared -o libslnc.so $^
	
#deocder.example
decoder.example: libslnc.so test.decoders.c 
	$(CC) -L. -lslnc -o $@ $(CFLAGS0) $(CFLAGS1) $^
#deocde.file.example
decode.file: libslnc.so test.file.decoders.c 
	$(CC) -L. -lslnc -o $@ $(CFLAGS0) $(CFLAGS1) $^
band.OA.static: $(GNCENC) $(OADEC) examples/test.OAdecoder.c
	$(SED) -i 's/[^ ]*_SLNC/BAND_SLNC/' examples/test.OAdecoder.c
	$(CC) -o $@ $(CFLAGS0) $(CFLAGS1) $^
#Recoder with band code and CBD decoder, TRIV_SCHED
recoder.CBD.trivSched: libslnc.so test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SLNC/BAND_SLNC/' examples/test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SCHED/TRIV_SCHED/' examples/test.2hopRecoder.CBD.c
	$(CC) -L. -lslnc -o $@ $(CFLAGS0) $(CFLAGS1) $^
#Recoder with band code and CBD decoder, RAND_SCHED
recoder.CBD.randSched: libslnc.so test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SLNC/BAND_SLNC/' examples/test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SCHED/RAND_SCHED/' examples/test.2hopRecoder.CBD.c
	$(CC) -L. -lslnc -o $@ $(CFLAGS0) $(CFLAGS1) $^
#Recoder with band code and CBD decoder, MLPI_SCHED
recoder.CBD.mlpiSched: libslnc.so test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SLNC/BAND_SLNC/' examples/test.2hopRecoder.CBD.c
	$(SED) -i 's/[^ ]*_SCHED/MLPI_SCHED/' examples/test.2hopRecoder.CBD.c
	$(CC) -L. -lslnc -o $@ $(CFLAGS0) $(CFLAGS1) $^

$(OBJDIR)/%.o: $(OBJDIR)/%.c $(DEFS) $(GGDEFS)
	$(CC) -c -fpic -o $@ $< $(CFLAGS0) $(CFLAGS1)

.PHONY: clean
clean:
	rm -f *.o $(OBJDIR)/*.o *.example *.static decode.file recoder.CBD.randSched recoder.CBD.trivSched recoder.CBD.mlpiSched libslnc.so

.PHONY: install
install:
	cp libslnc.so /usr/lib/

.PHONY: uninstall
uninstall:
	rm -f /usr/lib/libslnc.so
