# -*- Mode: Makefile; -*-
#
# See COPYRIGHT in top-level directory.
#

.PHONY: stencil stencil_nopack

CC=mpicc
CFLAGS= -g -O3 -Wall
LDFLAGS=-lhwloc
STENCIL_COMMON_SRC=printarr_par.c
STENCIL_CFLAGS=$(CFLAGS) -I.
BINS=stencil stencil_nopack

all: $(BINS)

stencil: stencil.c $(STENCIL_COMMON_SRC)
	$(CC) $(STENCIL_CFLAGS) $^ -o $@ -lm $(LDFLAGS)

stencil_nopack: stencil_nopack.c $(STENCIL_COMMON_SRC)
	$(CC) $(STENCIL_CFLAGS) $^ -o $@ -lm $(LDFLAGS)

clean:
	rm -f $(BINS)
	rm -f output*tmp
