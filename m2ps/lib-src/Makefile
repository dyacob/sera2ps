# Generated automatically from Makefile.in.in by configure.
# Makefile for lib-src subdirectory in GNU Emacs.
# Copyright (C) 1985, 1987, 1988, 1993, 1994 Free Software Foundation, Inc.

# This file is part of GNU Emacs.

# GNU Emacs is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# GNU Emacs is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with GNU Emacs; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

# Avoid trouble on systems where the `SHELL' variable might be
# inherited from the environment.
SHELL = /bin/sh

# ==================== Things `configure' will edit ====================

CC=gcc -static
CFLAGS= -O 
ALLOCA=
ARFLAGS = rcv
version=19.28
configuration=


MULELIBDIR=../etc
LIBDIR=/usr/local/lib
BINDIR=/usr/local/bin
INCDIR=/usr/local/include
CODINGS_PATH=${MULELIBDIR}
CHARSETS_PATH=${MULELIBDIR}
srcdir=.

INSTALLABLES= m2ps coco

# ========================== start of cpp stuff =======================
LOADLIBES=   
ALL_CFLAGS =     -Demacs -DHAVE_CONFIG_H    -I. -I../src -I${srcdir} -I${srcdir}/../src ${LDFLAGS} ${CPPFLAGS} ${CFLAGS}
LINK_CFLAGS =     -Demacs -DHAVE_CONFIG_H    -I. -I../src -I${srcdir} -I${srcdir}/../src ${LDFLAGS} ${CFLAGS}
CPP_CFLAGS =     -Demacs -DHAVE_CONFIG_H    -I. -I../src -I${srcdir} -I${srcdir}/../src ${CPPFLAGS} ${CFLAGS}
ALLOCA_CFLAGS =     -DHAVE_CONFIG_H    -I. -I../src -I${srcdir} -I${srcdir}/../src ${CPPFLAGS} ${CFLAGS}
.c.o:
	${CC} -c ${CPP_CFLAGS} $<
 all:  ${INSTALLABLES}




bdf.o: ${srcdir}/bdf.c ${srcdir}/bdf.h ${srcdir}/mulelib.h Makefile		../src/config.h ../src/paths.h ../src/charset.h ../src/coding.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/bdf.c
m2ps.o: ${srcdir}/m2ps.c ${srcdir}/bdf.h ${srcdir}/mulelib.h		../src/config.h ../src/paths.h ../src/charset.h ../src/coding.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/m2ps.c
fakelisp.o: ${srcdir}/fakelisp.c ${srcdir}/fakelisp.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/fakelisp.c
charset.o: ${srcdir}/../src/charset.c ../src/charset.h ${srcdir}/fakelisp.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/../src/charset.c
coding.o: ${srcdir}/../src/coding.c ../src/coding.h ../src/charset.h		${srcdir}/fakelisp.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/../src/coding.c
ccl.o: ${srcdir}/../src/ccl.c ../src/charset.h ../src/coding.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/../src/ccl.c
mulelib.o: ${srcdir}/mulelib.c ${srcdir}/mulelib.h ${srcdir}/fakelisp.h		../src/config.h ../src/paths.h ../src/charset.h ../src/coding.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/mulelib.c
m2ps: bdf.o m2ps.o fakelisp.o charset.o coding.o ccl.o mulelib.o
	$(CC) $(LINK_CFLAGS) bdf.o m2ps.o fakelisp.o charset.o coding.o ccl.o mulelib.o		$(LOADLIBES) -o m2ps
coco.o: ${srcdir}/coco.c ${srcdir}/mulelib.h		../src/config.h ../src/paths.h ../src/charset.h ../src/coding.h
	$(CC) -c $(ALLOCA_CFLAGS) ${srcdir}/coco.c
coco: coco.o fakelisp.o charset.o coding.o ccl.o mulelib.o
	$(CC) $(LINK_CFLAGS) coco.o fakelisp.o charset.o coding.o ccl.o mulelib.o		$(LOADLIBES) -o coco

clean:
	(rm ./*.o)

cleanall:
	(rm ./*.o $(INSTALLABLES))
