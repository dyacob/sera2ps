/* CCL -- Code Conversion Language Interpreter
   Copyright (C) 1992 Free Software Foundation, Inc.

This file is part of Mule (MULtilingual Enhancement of GNU Emacs).

Mule is free software distributed in the form of patches to GNU Emacs.
You can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

Mule is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

/* 93.5.14  created for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp> */
/* 93.7.26  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	enum ccl_code is changed from xxx to CCLxxx. */
/* 93.11.22 modified for Mule Ver.1.1 by K.Handa <handa@etl.go.jp>
	Bug in interpreting CCLexch fixed. */
/* 94.6.18  modified for Mule Ver.2.0 by K.Handa <handa@etl.go.jp>
	Completely re-written. */
/* 94.8.10  modified for Mule Ver.2.0 by K.Handa <handa@etl.go.jp>
	Add standalone routine. */

#include <stdio.h>
#ifdef emacs
#include <config.h>
#include "lisp.h"
#include "charset.h"
#include "coding.h"
#else  /* not emacs */
#include "mulelib.h"
#endif /* not emacs */

/* CCL operators */
#define CCL_SetCS		0x00
#define CCL_SetCL		0x01
#define CCL_SetR		0x02
#define CCL_SetA		0x03
#define CCL_Jump		0x04
#define CCL_JumpCond		0x05
#define CCL_WriteJump		0x06
#define CCL_WriteReadJump	0x07
#define CCL_WriteCJump		0x08
#define CCL_WriteCReadJump	0x09
#define CCL_WriteSJump		0x0A
#define CCL_WriteSReadJump	0x0B
#define CCL_WriteAReadJump	0x0C
#define CCL_Branch		0x0D
#define CCL_Read1		0x0E
#define CCL_Read2		0x0F
#define CCL_ReadBranch		0x10
#define CCL_Write1		0x11
#define CCL_Write2		0x12
#define CCL_WriteC		0x13
#define CCL_WriteS		0x14
#define CCL_WriteA		0x15
#define CCL_End			0x16
#define CCL_SetSelfCS		0x17
#define CCL_SetSelfCL		0x18
#define CCL_SetSelfR		0x19
#define CCL_SetExprCL		0x1A
#define CCL_SetExprR		0x1B
#define CCL_JumpCondC		0x1C
#define CCL_JumpCondR		0x1D
#define CCL_ReadJumpCondC	0x1E
#define CCL_ReadJumpCondR	0x1F

#define CCL_PLUS	0x00
#define CCL_MINUS	0x01
#define CCL_MUL		0x02
#define CCL_DIV		0x03
#define CCL_MOD		0x04
#define CCL_AND		0x05
#define CCL_OR		0x06
#define CCL_XOR		0x07
#define CCL_LSH		0x08
#define CCL_RSH		0x09
#define CCL_LSH8	0x0A
#define CCL_RSH8	0x0B
#define CCL_DIVMOD	0x0C
#define CCL_LS		0x10
#define CCL_GT		0x11
#define CCL_EQ		0x12
#define CCL_LE		0x13
#define CCL_GE		0x14
#define CCL_NE		0x15

/* Header of CCL compiled code */
#define CCL_HEADER_BUF_MAG	0
#define CCL_HEADER_EOF		1
#define CCL_HEADER_MAIN		2

#define CCL_STAT_SUCCESS	0
#define CCL_STAT_SUSPEND	1
#define CCL_STAT_INVALID_CMD	2
#define CCL_STAT_OVERFLOW	3

#define CCL_SUCCESS \
  ccl->status = CCL_STAT_SUCCESS; \
  goto ccl_finish
#define CCL_SUSPEND \
  ccl->ic = --ic; \
  ccl->status = CCL_STAT_SUSPEND; \
  goto ccl_finish
#define CCL_INVALID_CMD \
  ccl->status = CCL_STAT_INVALID_CMD; \
  goto ccl_error_handler
#define CCL_OVERFLOW \
  ccl->status = CCL_STAT_OVERFLOW; \
  goto ccl_error_handler

#define CCL_WRITE_CHAR(ch) \
{\
  if (!src) {\
    CCL_INVALID_CMD;\
  } else if (d + 4 <= d_end) {\
    d += CHARtoSTR (ch, d);\
  } else {\
    CCL_OVERFLOW;\
  }\
}

#define CCL_WRITE_STRING(len) \
{\
  if (!src) {\
    CCL_INVALID_CMD;\
  } else if (d + len <= d_end) {\
    for (j = 0; j < len; j++) *d++ = XFASTINT (prog[ic + 1 + j]);\
  } else {\
    CCL_OVERFLOW;\
  }\
}

#define CCL_READ_CHAR(r) \
{\
  if (!src) {\
    CCL_INVALID_CMD;\
  } else if (s < s_end)\
    r = *s++;\
  else if (end_flag) {\
    ic = XFASTINT (prog[CCL_HEADER_EOF]);\
    continue;\
  } else {\
    CCL_SUSPEND;\
  }\
}

ccl_driver(ccl, src, dst, n, end_flag)
     CCL_PROGRAM *ccl;
     unsigned char *src, *dst;
     int n, end_flag;
{
  register int *reg = ccl->reg;
  register int ic = ccl->ic;
  register int code, op, rrr, cc, i, j;
  register Lisp_Object *prog = ccl->prog;
  register unsigned char *s, *d, *s_end, *d_end;

  if (src) {
    s = src;
    s_end = s + n;
    d = dst;
    d_end = d + n * ccl->buf_mag + 4;
  }

  while (1) {
    code = XFASTINT (prog[ic++]);
    op = code & 0x1F;
    rrr = (code >> 5) & 0x7;
    cc = code >> 8;

    switch (op) {
    case CCL_SetCS:
      reg[rrr] = cc; continue;
    case CCL_SetCL:
      reg[rrr] = XFASTINT (prog[ic++]); continue;
    case CCL_SetR:
      reg[rrr] = reg[cc]; continue;
    case CCL_SetA:
      cc = reg[cc];
      i = XFASTINT (prog[ic++]);
      if (cc >= 0 && cc < i)
	reg[rrr] = XFASTINT (prog[ic + cc]);
      ic += i;
      continue;
    case CCL_Jump:
      ic = cc; continue;
    case CCL_JumpCond:
      if (!reg[rrr])
	ic = cc;
      continue;
    case CCL_WriteJump:
      CCL_WRITE_CHAR (reg[rrr]);
      ic = cc;
      continue;
    case CCL_WriteReadJump:
      if (ccl->status != CCL_STAT_SUSPEND) {
	CCL_WRITE_CHAR (reg[rrr]);
      } else
	ccl->status = CCL_STAT_SUCCESS;
      CCL_READ_CHAR (reg[rrr]);
      ic = cc;
      continue;
    case CCL_WriteCJump:
      CCL_WRITE_CHAR (XFASTINT (prog[ic]));
      ic = cc;
      continue;
    case CCL_WriteCReadJump:
      if (ccl->status != CCL_STAT_SUSPEND) {
	CCL_WRITE_CHAR (XFASTINT (prog[ic]));
      } else
	ccl->status = CCL_STAT_SUCCESS;
      CCL_READ_CHAR (reg[rrr]);
      ic = cc;
      continue;
    case CCL_WriteSJump:
      i = XFASTINT (prog[ic]);
      CCL_WRITE_STRING (i);
      ic = cc;
      continue;
    case CCL_WriteSReadJump:
      if (ccl->status != CCL_STAT_SUSPEND) {
	i = XFASTINT (prog[ic]);
	CCL_WRITE_STRING (i);
      } else
	ccl->status = CCL_STAT_SUCCESS;
      CCL_READ_CHAR (reg[rrr]);
      ic = cc;
      continue;
    case CCL_WriteAReadJump:
      if (ccl->status != CCL_STAT_SUSPEND) {
	i = XFASTINT (prog[ic]);
	if (reg[rrr] >= 0 && reg[rrr] < i)
	  CCL_WRITE_CHAR (XFASTINT (prog[ic + 1 + reg[rrr]]));
      } else
	ccl->status = CCL_STAT_SUCCESS;
      CCL_READ_CHAR (reg[rrr]);
      ic = cc;
      continue;
    case CCL_ReadBranch:
      CCL_READ_CHAR (reg[rrr]);
    case CCL_Branch:
      ic = XFASTINT (prog[ic + (((unsigned int) reg[rrr] < cc)
				? reg[rrr] : cc)]);
      continue;
    case CCL_Read1:
      CCL_READ_CHAR (reg[rrr]);
      continue;
    case CCL_Read2:
      CCL_READ_CHAR (reg[rrr]);
      CCL_READ_CHAR (reg[cc]);
      continue;
    case CCL_Write1:
      CCL_WRITE_CHAR (reg[rrr]);
      continue;
    case CCL_Write2:
      CCL_WRITE_CHAR (reg[rrr]);
      CCL_WRITE_CHAR (reg[cc]);
      continue;
    case CCL_WriteC:
      i = XFASTINT (prog[ic++]);
      CCL_WRITE_CHAR (i);
      continue;
    case CCL_WriteS:
      cc = XFASTINT (prog[ic]);
      CCL_WRITE_STRING (cc);
      ic += cc + 1;
      continue;
    case CCL_WriteA:
      i = XFASTINT (prog[ic++]);
      cc = reg[rrr];
      if (cc >= 0 && cc < i)
	CCL_WRITE_CHAR (XFASTINT (prog[ic + cc]));
      ic += i;
      continue;
    case CCL_End:
      CCL_SUCCESS;
    case CCL_SetSelfCS:
      i = cc;
      op = XFASTINT (prog[ic++]);
      goto ccl_set_self;
    case CCL_SetSelfCL:
      i = XFASTINT (prog[ic++]);
      op = XFASTINT (prog[ic++]);
      goto ccl_set_self;
    case CCL_SetSelfR:
      i = reg[cc];
      op = XFASTINT (prog[ic++]);
    ccl_set_self:
      switch (op) {
      case CCL_PLUS: reg[rrr] += i; break;
      case CCL_MINUS: reg[rrr] -= i; break;
      case CCL_MUL: reg[rrr] *= i; break;
      case CCL_DIV: reg[rrr] /= i; break;
      case CCL_MOD: reg[rrr] %= i; break;
      case CCL_AND: reg[rrr] &= i; break;
      case CCL_OR: reg[rrr] |= i; break;
      case CCL_XOR: reg[rrr] ^= i; break;
      case CCL_LSH: reg[rrr] <<= i; break;
      case CCL_RSH: reg[rrr] >>= i; break;
      case CCL_LSH8: reg[rrr] <<= 8; reg[rrr] |= i; break;
      case CCL_RSH8: reg[7] = reg[rrr] & 0xFF; reg[rrr] >>= 8; break;
      case CCL_DIVMOD: reg[7] = reg[rrr] % i; reg[rrr] /= i; break;
      case CCL_LS: reg[rrr] = reg[rrr] < i; break;
      case CCL_GT: reg[rrr] = reg[rrr] > i; break;
      case CCL_EQ: reg[rrr] = reg[rrr] == i; break;
      case CCL_LE: reg[rrr] = reg[rrr] <= i; break;
      case CCL_GE: reg[rrr] = reg[rrr] >= i; break;
      case CCL_NE: reg[rrr] = reg[rrr] != i; break;
      default: CCL_INVALID_CMD;
      }
      continue;
    case CCL_SetExprCL:
      i = reg[cc];
      j = XFASTINT (prog[ic++]);
      op = XFASTINT (prog[ic++]);
      cc = 0;
      goto ccl_set_expr;
    case CCL_SetExprR:
      i = reg[cc];
      j = reg[XFASTINT (prog[ic++])];
      op = XFASTINT (prog[ic++]);
      cc = 0;
      goto ccl_set_expr;
    case CCL_ReadJumpCondC:
      CCL_READ_CHAR (reg[rrr]);
    case CCL_JumpCondC:
      i = reg[rrr];
      j = XFASTINT (prog[ic++]);
      rrr = 7;
      op = XFASTINT (prog[ic++]);
      goto ccl_set_expr;
    case CCL_ReadJumpCondR:
      CCL_READ_CHAR (reg[rrr]);
    case CCL_JumpCondR:
      i = reg[rrr];
      j = reg[XFASTINT (prog[ic++])];
      rrr = 7;
      op = XFASTINT (prog[ic++]);
    ccl_set_expr:
      switch (op) {
      case CCL_PLUS: reg[rrr] = i + j; break;
      case CCL_MINUS: reg[rrr] = i - j; break;
      case CCL_MUL: reg[rrr] = i * j; break;
      case CCL_DIV: reg[rrr] = i / j; break;
      case CCL_MOD: reg[rrr] = i % j; break;
      case CCL_AND: reg[rrr] = i & j; break;
      case CCL_OR: reg[rrr] = i | j; break;
      case CCL_XOR: reg[rrr] = i ^ j;; break;
      case CCL_LSH: reg[rrr] = i << j; break;
      case CCL_RSH: reg[rrr] = i >> j; break;
      case CCL_LSH8: reg[rrr] = (i << 8) | j; break;
      case CCL_RSH8: reg[rrr] = i >> 8; reg[7] = i & 0xFF; break;
      case CCL_DIVMOD: reg[rrr] = i / j; reg[7] = i % j; break;
      case CCL_LS: reg[rrr] = i < j; break;
      case CCL_GT: reg[rrr] = i > j; break;
      case CCL_EQ: reg[rrr] = i == j; break;
      case CCL_LE: reg[rrr] = i <= j; break;
      case CCL_GE: reg[rrr] = i >= j; break;
      case CCL_NE: reg[rrr] = i != j; break;
      default: CCL_INVALID_CMD;
      }
      if (cc && !reg[rrr])
	ic = cc;
      continue;
    default:
      CCL_INVALID_CMD;
    }
  }

 ccl_error_handler:
  if (dst) {
    switch (ccl->status) {
    case CCL_STAT_INVALID_CMD:
      sprintf(d, "CCL: Invalid command (%x).\n", op);
      break;
    case CCL_STAT_OVERFLOW:
      sprintf(d, "CCL: Conversion buffer too short.\n");
      break;
    default:
      sprintf(d, "CCL: Unknown error type (%d).\n", ccl->status);
    }
    d += strlen(d);
  }

 ccl_finish:
  return (d - dst);
}

set_ccl_program (ccl, val)
     CCL_PROGRAM *ccl;
     Lisp_Object val;
{
  int i;

  ccl->prog = XVECTOR (val)->contents;
  ccl->size = XVECTOR (val)->size;
  ccl->buf_mag = XFASTINT (ccl->prog[CCL_HEADER_BUF_MAG]);
  if (ccl->buf_mag < 0 || ccl->buf_mag > 10)
    ccl->buf_mag = 1;
  ccl->ic = CCL_HEADER_MAIN;
  for (i = 0; i < 8; i++)
    ccl->reg[i] = 0;
  ccl->end_flag = 0;
  ccl->status = 0;
}

CCL_PROGRAM *x_ccl_programs[128];

#ifdef emacs

DEFUN ("exec-ccl", Fexec_ccl, Sexec_ccl, 2, 2, 0,
  "Execute CCL-PROGRAM with registers initialized by REG.\n\
CCL-PROGRAM is a compiled code created by `ccl-compile'.\n\
REG is a vector of [R0 R1 ... R7] where Rn is an initial value\n\
 of nth register.\n\
Returns a vector which contains the resulting register values.")
  (ccl_prog, reg)
     Lisp_Object ccl_prog, reg;
{
  CCL_PROGRAM ccl;
  int i;

  CHECK_VECTOR (ccl_prog, 0);
  CHECK_VECTOR (reg, 1);

  set_ccl_program (&ccl, ccl_prog);
  for (i = 0; i < 8; i++)
    if (XTYPE (XVECTOR (reg)->contents[i]) == Lisp_Int)
      ccl.reg[i] = XINT (XVECTOR (reg)->contents[i]);
  ccl_driver (&ccl, (char *)0, (char *)0, 0, 0);
  for (i = 0; i < 8; i++)
    XSET (XVECTOR (reg)->contents[i], Lisp_Int, ccl.reg[i]);

  return reg;
}

DEFUN ("exec-ccl-string", Fexec_ccl_string, Sexec_ccl_string, 3, 3, 0,
  "Execute CCL-PROGRAM with initial STATUS on STRING.\n\
CCL-PROGRAM is a compiled code created by `ccl-compile'.\n\
STATUS is a vector of [R0 R1 ... R7 IC], where\n\
 R0..R7 are initial values of corresponding registers,\n\
 IC is the instruction counter specifying from where to start the program.\n\
If R0..R7 are nil, they are initialized to 0.\n\
If IC is nil, it is initialized to head of the program.\n\
Returns a resulting string, and by side effect, STATUS is updated.")
  (ccl_prog, status, str)
     Lisp_Object ccl_prog, status, str;
{
  Lisp_Object val;
  CCL_PROGRAM ccl;
  int i, len;
  char *outbuf;

  CHECK_VECTOR (ccl_prog, 0);
  CHECK_VECTOR (status, 1);
  if (XVECTOR (status)->size != 9)
    error ("Invalid length of vector STATUS.");
  CHECK_STRING (str, 2);

  set_ccl_program (&ccl, ccl_prog);
  for (i = 0; i < 8; i++) {
    if (NILP (XVECTOR (status)->contents[i]))
      XSET (XVECTOR (status)->contents[i], Lisp_Int, 0);
    if (XTYPE (XVECTOR (status)->contents[i]) == Lisp_Int)
      ccl.reg[i] = XINT (XVECTOR (status)->contents[i]);
  }
  if (NILP (XVECTOR (status)->contents[8]))
    XSET (XVECTOR (status)->contents[8], Lisp_Int, ccl.ic);
  if (XTYPE (XVECTOR (status)->contents[i]) == Lisp_Int)
    i = XINT (XVECTOR (status)->contents[8]);
  else
    i = 0;
  if (ccl.ic < i && i < ccl.size)
    ccl.ic = i;

  outbuf = (char *) xmalloc (XSTRING (str)->size * ccl.buf_mag
			     + CONV_BUF_EXTRA);
  if (!outbuf)
    error ("not enough memory");
  len = ccl_driver (&ccl, XSTRING (str)->data, outbuf, XSTRING (str)->size, 0);
  ccl_driver (&ccl, "", outbuf + len, 0, 1);
  for (i = 0; i < 8; i++)
    XSET (XVECTOR (status)->contents[i], Lisp_Int, ccl.reg[i]);
  XSET (XVECTOR (status)->contents[8], Lisp_Int, ccl.ic);

  val = make_string (outbuf, len);
  free (outbuf);
  return val;
}

/* Vector to hold CCL programs */
Lisp_Object Vx_ccl_programs;

DEFUN ("x-set-ccl", Fx_set_ccl, Sx_set_ccl, 2, 2, 0,
      "Sets to character set LC the CCL-PROGRAM.\n\
CCL-PROGMAM should be a compiled code made by `ccl-compile'.\n\
If CCL-PROGMAM is nil, unset any CCL program of LC.\n\
This CCL-PROGRAM works for mapping character code used in Mule to\n\
 that of font.")
  (leading_char, ccl_prog)
     Lisp_Object leading_char, ccl_prog;
{
  int lc = XINT (leading_char);

  if (lc < 0 || (lc > 0 && lc < 0x81) || lc >= 0x100)
    error ("Invalid LEADING-CHAR (%d).", lc);
  if (!NILP (ccl_prog) && XTYPE (ccl_prog) != Lisp_Vector)
    error ("Invalid CCL program format.");
  lc &= 0x7F;
  XVECTOR (Vx_ccl_programs)->contents[lc] = ccl_prog;
  if (NILP (ccl_prog)) {
    if (x_ccl_programs[lc]) {
      free(x_ccl_programs[lc]);
      x_ccl_programs[lc] = (CCL_PROGRAM *)0;
    }
  } else {
    if (!x_ccl_programs[lc])
      x_ccl_programs[lc] = (CCL_PROGRAM *) xmalloc (sizeof (CCL_PROGRAM));
    set_ccl_program (x_ccl_programs[lc], ccl_prog);
  }

  return Qnil;
}

DEFUN ("x-get-ccl", Fx_get_ccl, Sx_get_ccl, 1, 1, 0,
      "If a character set LC is set `CCL-PROGRAM', return it.")
  (leading_char)
     Lisp_Object leading_char;
{
  int lc = XINT (leading_char);

  if (lc < 0 || (lc > 0 && lc < 0x81) || lc >= 0x100)
    return Qnil;

  return XVECTOR (Vx_ccl_programs)->contents[lc & 0x7F];
}

static long current_time;

DEFUN ("reset-elapsed-time", Freset_etime, Sreset_etime, 0, 0, 0,
  "Reset the internal value which holds a time elapsed by ccl interpreter.")
  ()
{
  int i;

  current_time = (long) clock();
  return Qnil;
}

DEFUN ("elapsed-time", Fetime, Setime, 0, 0, 0,
  "Return a time elapsed by ccl interpreter as cons of sec and msec.")
  ()
{
  long ctime = (long) clock(), etime = ctime - current_time;

  if (etime < 0) etime += 2147000000l;
  etime /= 1000;
  current_time = ctime;

  return Fcons (make_number ((int)(etime / 1000)),
		make_number ((int)(etime % 1000)));
}

syms_of_ccl ()
{
  staticpro (&Vx_ccl_programs);
  Vx_ccl_programs = Fmake_vector (128, Qnil);
  {
    int i;

    for (i = 0; i < 128; i++)
      x_ccl_programs[i] = (CCL_PROGRAM *)0;
  }

  defsubr (&Sexec_ccl);
  defsubr (&Sexec_ccl_string);
  defsubr (&Sx_set_ccl);
  defsubr (&Sx_get_ccl);
  defsubr (&Sreset_etime);
  defsubr (&Setime);
}

#else  /* not emacs */
#ifdef standalone

#include <alloca.h>

#define INBUF_SIZE 1024
#define MAX_CCL_CODE_SIZE 4096

main (argc, argv)
     int argc;
     char **argv;
{
  FILE *progf;
  char inbuf[INBUF_SIZE], *outbuf;
  CCL_PROGRAM ccl;
  int i;
  Lisp_Object ccl_prog = make_vector (MAX_CCL_CODE_SIZE);

  if (argc < 2) {
    fprintf(stderr,
	    "Usage: %s ccl_program_file_name <infile >outfile\n",
	    argv[0]);
    exit (1);
  }

  if ((progf = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "%s: Can't read file %s", argv[0], argv[1]);
    exit (1);
  }

  XVECTOR (ccl_prog)->size = 0;
  while (fscanf(progf, "%x", &i) == 1)
    XFASTINT (XVECTOR (ccl_prog)->contents[XVECTOR (ccl_prog)->size++]) = i;
  set_ccl_program (&ccl, ccl_prog);

  outbuf = (char *) alloca (INBUF_SIZE * ccl.buf_mag + CONV_BUF_EXTRA);

  while ((i = fread (inbuf, 1, INBUF_SIZE, stdin)) == INBUF_SIZE) {
    i = ccl_driver (&ccl, inbuf, outbuf, INBUF_SIZE, 0);
    fwrite (outbuf, 1, i, stdout);
  }
  if (i) {
    i = ccl_driver (&ccl, inbuf, outbuf, i, 1);
    fwrite (outbuf, 1, i, stdout);
  }

  fclose (progf);
  exit (0);
}
#endif  /* standalone */
#endif  /* not emacs */
