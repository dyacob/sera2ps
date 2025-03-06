/* Functions to handle multilingual characters.
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

/* 91.10.09 written by K.Handa <handa@etl.go.jp> */
/* 92.3.6   modified for Mule Ver.0.9.0 by K.Handa <handa@etl.go.jp> */
/* 92.3.21  modified for Mule Ver.0.9.1
   by T.Nakagawa <nakagawa@is.titech.ac.jp>
	In Fcharacter_set(), argument of last Fcons fixed.
	In Fnew_character_set(), cheking of leading char fixed.
   by K.Handa <handa@etl.go.jp>
	Declarations of char_... are changed. */
/* 92.3.25  modified for Mule Ver.0.9.2
   by T.Nakagawa <nakagawa@is.titech.ac.jp>
	More severe error check in Fnew_character_set() */
/* 92.7.2   modified for Mule Ver.0.9.5 by K.Handa <handa@etl.go.jp>
	char_width() handles control characters. */
/* 92.7.8   modified for Mule Ver.0.9.5 by K.Handa <handa@etl.go.jp>
	Support for WNN3 and WNN4V3 stopped. */
/* 92.7.8   modified for Mule Ver.0.9.5 by Y.Kawabe <kawabe@sramhc.sra.co.jp>
	Support for SJ3 started. */
/* 92.9.2   modified for Mule Ver.0.9.6 by K.Handa <handa@etl.go.jp>
	Big change to support private character-set. */
/* 92.9.11  modified for Mule Ver.0.9.6 by K.Handa <handa@etl.go.jp>
	Fredisplay_region() is created. */
/* 92.9.16  modified for Mule Ver.0.9.6 by K.Handa <handa@etl.go.jp>
	undefined_private_charset() is created. */
/* 92.11.10 modified for Mule Ver.0.9.6 by K.Handa <handa@etl.go.jp>
	Composite character is supported. */
/* 92.12.15 modified for Mule Ver.0.9.7 by K.Handa <handa@etl.go.jp>
	Set char_bytes and char_width also for LCINV. */
/* 92.12.17 modified for Mule Ver.0.9.7 by Y.Niibe <gniibe@mri.co.jp>
	Preliminary support (92.8.2) for right-to-left languages. */
/* 93.1.13  modified for Mule Ver.0.9.7.1 by K.Handa <handa@etl.go.jp>
	In Fcharacter_set(), char_description[] should be index by lc&0x7F */
/* 93.4.29  modified for Mule Ver.0.9.7.1 by K.Handa <handa@etl.go.jp>
	CNS11643 support. */
/* 93.5.22  modified for Mule Ver.0.9.7.1 by Y.Niibe <gniibe@mri.co.jp>
	change so that private char-set could have direction */
/* 93.5.27  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	Fmake_character() handles partially defined char. */
/* 93.6.2   modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	Arrays lc94[],... are changed to two dimentional array lc_table,
	indexed by type and final character. */
/* 93.6.17  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	'string-width' is defined in this file. */
/* 93.7.13  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	Format of wordbuf changed.  */
/* 93.7.15  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	In Fdefine_word_pattern(), memory allocation bug fixed. */
/* 94.5.10  modified for Mule Ver.1.1 by K.Handa <handa@etl.go.jp>
	Bug in charwidth() fixed. */

#include <stdio.h>

#ifdef emacs
#include <sys/types.h>
#include <config.h>
#else  /* not emacs */
#define xmalloc (void *)malloc
#define xrealloc (void *)realloc
#endif /* emacs */

#ifdef emacs
#include "lisp.h"
#include "buffer.h"
#include "charset.h"		/* 92.1.21 by K.Handa */
#else  /* not emacs */
#include "mulelib.h"
#endif /* not emacs */

#ifdef emacs

Lisp_Object Vlc_ascii, Vlc_ltn1, Vlc_ltn2, Vlc_ltn3, Vlc_ltn4, Vlc_grk;
Lisp_Object Vlc_thai;
Lisp_Object Vlc_arb, Vlc_hbw, Vlc_kana, Vlc_roman, Vlc_crl, Vlc_ltn5;
Lisp_Object Vlc_jpold, Vlc_cn, Vlc_jp, Vlc_jp2, Vlc_kr, Vlc_big5_1, Vlc_big5_2;
Lisp_Object Vlc_cns1, Vlc_cns2, Vlc_cns14; /* 93.4.29, 93.7.25 by K.Handa */
Lisp_Object Vlc_prv11, Vlc_prv12, Vlc_prv21, Vlc_prv22, Vlc_prv3;
Lisp_Object Vlc_invalid, Vlc_composite; /* 92.11.14 by K.Handa */
Lisp_Object Vlc_prv11_ext, Vlc_prv12_ext, Vlc_prv21_ext, Vlc_prv22_ext;
Lisp_Object Vlc_prv3_ext;

#endif /* emacs */

/* Tables for character set definition. */
/* 92.3.21, 92.9.2 by K.Handa - variable declaration changed */
char *char_description[128];
char *char_registry[128];
unsigned char char_bytes[256];
unsigned char char_width[256];
unsigned char char_direction[256]; /* 92.8.2 Y.Niibe */
unsigned char char_data[512];
/* Indices to the following arrays should be 0 or 0x80..0xFF. */
unsigned char *char_graphic;
unsigned char *char_type;
unsigned char *char_final;
/* Table of leading-char indexed by TYPE/FINAL_CHAR. */
unsigned char lc_table[4][128];
/* Table of leading-char indexed by leading-char of the same TYPE/FINAL_CHAR
   but reverse DIRECTION. */
unsigned char rev_lc_table[256];

/* 93.5.27 by K.Handa
   Use macro CHARtoSTR (c,str) instead of calling mchar_to_string() directly.
*/
/* 92.1.16 by K.Handa */
mchar_to_string (c, str)
     register unsigned int c;
     register unsigned char *str;
{				/* 92.9.5, 93.2.12, 93.5.27 by K.Handa */
  register unsigned char *p, c1;

  if (!MC_CHAR_P(c))
    {
      *str = c;
      return 1;
    }

  p = str;
  c1 = CHAR_LEADING_CHAR (c);
  if (c1 >= LCPRV11EXT)
    *p++ = LC_OF_EXT_LC (c1);
  *p++ = c1;
  if (char_type[c1] == TYPE94N || char_type[c1] == TYPE96N)
    if ((c1 = CHAR_HIGH (c)) > 0x80) *p++ = c1;
  if ((c1 = CHAR_LOW (c)) > 0x80) *p++ = c1;
  return (p - str);
}

/* 93.5.27 by K.Handa
   Use macro STRtoCHAR(str,size) instead of calling string_to_mchar() directly.
*/
string_to_mchar (str, size)
     register unsigned char *str;
     register int size;
{				/* 92.11.12, 93.5.27 by K.Handa */
  register int i0 = *str, i1, i2, i3;

  if (i0 == LCCMP) {
    for (i0 = 1; i0 < size && NONASCII_P (str[i0]); i0++)
      ;
    return (search_cmpchar(str, i0) | 0x7C000);
  }

  if (size == 2) i1 = *++str, i2 = i3 = 0;
  else if (size == 3) i1 = *++str, i2 = *++str, i3 = 0;
  else i1 = *++str, i2 = *++str, i3 = *++str;

  switch (char_bytes[i0]) {
  case ONEBYTE:			/* Type 1-1 */
    break;
  case TWOBYTE:			/* Type 1-2 */
    i0 = ((i0 - 0x70) << 7) | (i1 & 0x7F);
    break;
  case THREEBYTE:		/* Type 1-3 or 2-3 */
    if (i0 < LCPRV11)		/* Type 2-3 */
      i0 = ((i0 - 0x8F) << 14) | ((i1 & 0x7F) << 7) | (i2 & 0x7F);
    else {			/* Type 1-3 */
      if (i1 >= LCPRV11EXT && i1 < LCPRV21EXT)
	i0 = ((i1 - 0x70) << 7) | (i2 & 0x7F);
      else			/* invalid extented leading char */
	i0 = ' ';
    }
    break;
  default:			/* Type 2-4 or 3-4 */
    if (i0 < LCPRV3) {		/* Type 2-4 */
      if (i1 >= LCPRV21EXT && i1 < LCPRV3EXT)
	i0 = ((i1 - 0xE0) << 14) | ((i2 & 0x7F) << 7) | (i3 & 0x7F);
      else			/* invalid extented leading char*/
	i0 = ' ';
    } else			/* Type 3-4 (not supported) */
      i0 = ' ';
  }

  return i0;
}
/* end of patch */

/* Add direction, 92.8.2 Y.Niibe */
update_mc_table(lc, bytes, clm, type, graphic, final, direction, doc, reg)
     unsigned char lc, bytes, clm, type, graphic, final, direction;
     char *doc, *reg;
{
  char *p;

  if (lc < LCINV) {
    char_bytes[lc] = bytes;
    char_width[lc] = clm;
  }
  char_direction[lc] = direction; /* 92.8.2 Y.Niibe */
  char_type[lc] = type;
  char_graphic[lc] = graphic;
  char_final[lc] = final;
  char_description[lc & 0x7F] = doc; /* 92.9.2 by K.Handa */
  char_registry[lc & 0x7F] = reg;
  if (lc_table[type][final] == LCINV) {
    lc_table[type][final] = lc;
    rev_lc_table[lc] = lc;
  } else if (direction == RIGHT_TO_LEFT) {
    rev_lc_table[lc] = lc_table[type][final];
    rev_lc_table[lc_table[type][final]] = lc;
  }
}

#ifdef emacs

/* 92.9.16 by K.Handa */
undefined_private_charset(bytes, width)
     int bytes, width;
{
  int lc, from, to;

  if (bytes == 1) {
    if (width == 1) from = LCPRV11EXT, to = LCPRV12EXT;
    else from = LCPRV12EXT, to = LCPRV21EXT;
  } else {
    if (width == 1) from = LCPRV21EXT, to = LCPRV22EXT;
    else from = LCPRV22EXT, to = 0xFF;
  }
  for (lc = from; lc < to; lc++)
    if (char_type[lc] != TYPEINV) break;
  return (lc < to ? lc : 0);
}
/* end of patch */

/* Add direction, 92.8.2 Y.Niibe */
DEFUN ("new-character-set", Fnew_character_set, Snew_character_set, 9, MANY, 0,
  "Define new character set of LEADING-CHAR (1st arg).\n\
Rest of args are:\n\
 BYTE: 1, 2, or 3\n\
 COLUMNS: 1 or 2\n\
 TYPE: 0 (94 chars), 1 (96 chars), 2 (94x94 chars), or 3 (96x96 chars)\n\
 GRAPHIC: 0 (use g0 on output) or 1 (use g1 on output)\n\
 FINAL: final character of ISO escape sequence\n\
 DIRECTION: 0 (left-to-right) or 1 (right-to-left)\n\
 DOC: short description string.\n\
 REGISTRY: registry name of the character set.\n\
If LEADING-CHAR >= 0xA0, it is regarded as extended leading-char\n\
and BYTE and COLUMNS args are ignored.")
  (nargs, args)
     int nargs;
     Lisp_Object *args;
{
  unsigned char lc, b, c, t, g, f, dir; /* 92.8.2 Y.Niibe */
  char *r, *d;

  CHECK_NUMBER (args[0], 0);
  lc = XFASTINT (args[0]);
  if (lc <= 0x80 || lc == 0x9F)	/* 92.3.21 by T. NAKAGAWA, 92.9.2 by K.Handa */
    error ("Invalid leading character: %d", lc);
  if (char_type[lc] != TYPEINV)
    error ("The character set (%d) already used.", lc);

  CHECK_NUMBER (args[1], 1);
  if ((b = XFASTINT (args[1]) + 1) > 3 || b < 1) /* 92.3.25 by T.Nakagawa */
    error ("Invalid BYTES: %d", --b);
  CHECK_NUMBER (args[2], 2);
  if ((c = XFASTINT (args[2])) > 2) error ("Invalid WIDTH: %d", c);
  CHECK_NUMBER (args[3], 3);
  if ((t = XFASTINT (args[3])) > 4) error ("Invalid TYPE: %d", t);
  CHECK_NUMBER (args[4], 4);
  if ((g = XFASTINT (args[4])) > 1) error ("Invalid GRAPHIC: %d", g);
  CHECK_NUMBER (args[5], 5);
  if ((f = XFASTINT (args[5])) < '0' || f > '_')
    error ("Invalid final character: %c", f);
/* 92.8.2 Y.Niibe */
  CHECK_NUMBER (args[6], 6);
  if ((dir = XFASTINT (args[6])) > 1)
    error ("Invalid direction: %d", dir);
  CHECK_STRING (args[7], 7);
  d = (char *)xmalloc (XSTRING (args[7])->size + 1);
  bcopy (XSTRING (args[7])->data, d, XSTRING (args[7])->size + 1);
  CHECK_STRING (args[8], 8);
  r = (char *)xmalloc (XSTRING (args[8])->size + 1);
  bcopy (XSTRING (args[8])->data, r, XSTRING (args[8])->size + 1);

  update_mc_table(lc, b, c, t, g, f, dir, d, r);
/* end of patch */
  return Qnil;
}

DEFUN ("leading-char", Fleading_char, Sleading_char, 2, 3, 0,
  "Return (extended) leading-char of character-set with TYPE and FINAL-CHAR.\n\
Optional arg DIRECTION specifies writing direction (0:normal, 1:r2l).")
  (type, final, direction)
     Lisp_Object type, final, direction;
{				/* 93.6.2 by K.Handa */
  Lisp_Object val;
  int dir = (NILP (direction) || XFASTINT (direction) == 0) ? 0 : 1;

  CHECK_NUMBER (type, 0);
  CHECK_NUMBER (final, 0);
  XFASTINT (val) = lc_table[XFASTINT (type) & 3][XFASTINT (final) & 0xFF];
  if (dir) XFASTINT (val) = rev_lc_table[XFASTINT (val)];
  return val;
}

DEFUN ("set-leading-char", Fset_leading_char, Sset_leading_char, 3, 3, 0,
  "Make a character set LEADING-CHAR to be designated upon receiving\n\
a designation sequence for a character set of TYPE and FINAL-CHAR.")
  (leading_char, type, final)
     Lisp_Object leading_char, type, final;
{
  CHECK_NUMBER (leading_char, 0);
  CHECK_NUMBER (type, 0);
  CHECK_NUMBER (final, 0);
  if (VALID_LC (XFASTINT (leading_char))
      && XFASTINT (type) >= TYPE94 && XFASTINT (type) <= TYPE96N
      && XFASTINT (final) >= '0' && XFASTINT (final) <= '~')
    lc_table[XFASTINT (type)][XFASTINT (final)] = XFASTINT (leading_char);
  return Qnil;
}

/* Add direction, 92.8.2 Y.Niibe */
DEFUN ("character-set", Fcharacter_set, Scharacter_set, 1, 1, 0,
  "Return list of attributes of CHARSET indicated by a leading-char.\n\
Elements of the returned list are:\n\
 BYTES: 1..4,\n\
 COLUMNS: 1 or 2,\n\
 TYPE: 0 (94 chars), 1 (96 chars), 2 (94x94 chars), or 3 (96x96 chars),\n\
 GRAPHIC: 0 (use g0 on output) or 1 (use g1 on output),\n\
 FINAL-CHAR: final character of ISO escape sequence,\n\
 DIRECTION: 0 (left-to-right) or 1 (right-to-left),\n\
 DOC: short description string,\n\
 REGISTRY: charset registry name.\n\
Return nil if CHARSET is invalid or not yet defined.")
  (leading_char)
     Lisp_Object leading_char;
{				/* 92.9.2 by K.Handa */
  unsigned int lc;

  CHECK_NUMBER (leading_char, 0);
  lc = XFASTINT (leading_char);
  if (!VALID_LC (lc)) return Qnil;
/* 92.3.21 by T.Nakagawa -- argument of last Fcons fixed */
/* 92.8.2 Y.Niibe, 93.1.13 by K.Handa */
  return (Fcons (make_number (char_bytes[lc]),
		 Fcons (make_number (char_width[lc]),
			Fcons (make_number (char_type[lc]),
			       Fcons (make_number (char_graphic[lc]),
				      Fcons (make_number (char_final[lc]),
					     Fcons (make_number (char_direction[lc]),
						    Fcons (build_string (char_description[lc & 0x7F]),
							   Fcons (char_registry[lc & 0x7F] ? build_string (char_registry[lc & 0x7F]) : Qnil, Qnil)))))))));
}


DEFUN ("make-character", Fmake_character, Smake_character, 1, 4, 0,
  "Make multi-byte character from LEADING-CHAR\n\
 and optional args ARG1 and ARG2.")
  (leading_char, arg1, arg2, arg3)
     Lisp_Object leading_char, arg1, arg2, arg3;
{				/* 92.9.2, 93.5.27 by K.Handa */
  unsigned int lc, i = 0;
  unsigned char buf[4];

  CHECK_NUMBER(leading_char, 0);
  lc = XFASTINT (leading_char);
  if (!lc)
    /* lc == LCASCII */
    return make_number (NILP (arg1) ? 0 : XFASTINT (arg1));

  if (!VALID_LC (lc))
    error ("Invalid leading-char: %d", lc);
  if (LC_P (lc))
    buf[i++] = lc;
  else {
    buf[i++] = LC_OF_EXT_LC (lc);
    buf[i++] = lc;
  }
  if (!NILP (arg1))
    buf[i++] = XFASTINT (arg1) | 0x80;
  if (!NILP (arg2))
    buf[i++] = XFASTINT (arg2) | 0x80;

  return make_number (STRtoCHAR (buf, i));
}

DEFUN ("char-component", Fchar_component, Schar_component, 2, 2, 0,
  "Return a components of multi-byte character CHAR.\n\
Second arg IDX indicate which component should be returned as follows.\n\
 0: leading character or extended leading character,\n\
 1: first byte of the character code,\n\
 2: second byte of the character code.\n\
If the character does not have the componets, 0 is returned.")
  (ch, idx)
     Lisp_Object ch, idx;
{				/* 92.9.2 by K.Handa */
  register unsigned int c, i;
  Lisp_Object val;

  CHECK_CHARACTER (ch, 0);
  c = XFASTINT (ch);

  CHECK_NUMBER (idx, 1);
  i = XFASTINT (idx);

  if (i > 2)
    error ("Invalid index: %d", i);

  if (! MC_CHAR_P (c))		/* Type 1-1 */
    XFASTINT (val) = (i == 0)? c : 0;
  else
    {
      unsigned char buffer[3], *s = buffer;

      s[1] = s[2] = 0;

      *s++ = CHAR_LEADING_CHAR (c);
      if (c & CHAR_F1_MASK)	/* Type 2-[34] or Type N */
	*s++ = CHAR_HIGH (c);
      *s++ = CHAR_LOW (c);

      XFASTINT (val) = (buffer[i] > 0x80)? buffer[i] : 0;
    }

  return val;
}

DEFUN ("char-leading-char", Fchar_leading_char, Schar_leading_char, 1, 1, 0,
  "Return leading character of CHAR.")
  (ch)
     Lisp_Object ch;
{			/* 92.7.2 by K.Handa -- calls char_leading_char() */
  unsigned int c;

  CHECK_CHARACTER (ch, 0);
  c = XFASTINT (ch);
  XSET (ch, Lisp_Int, CHAR_LEADING_CHAR (c));
  return ch;
}

charbytes (c)
     unsigned int c;
{
  if (NILP (current_buffer->mc_flag))
    return 1;
  else {
    if (MC_CHAR_P (c))
      c = CHAR_LEADING_CHAR (c);
    if (VALID_LC (c))
      return char_bytes[c];
    else
      return 1;
  }
}

DEFUN ("char-bytes", Fchar_bytes, Schar_bytes, 1, 1, 0,
  "Return number of bytes a character in CHARSET occupies in a buffer.\n\
CHARSET should be specified by a leading character.\n\
You can also provide a character as an argument.")
  (charset)
     Lisp_Object charset;
{
  CHECK_NUMBER (charset, 0);
  return make_number (charbytes (XFASTINT (charset)));
}

/* 93.6.17 by K.Handa */
charwidth (c)
     unsigned int c;
{
  if (!MC_CHAR_P (c)) {		/* 94.5.10 by K.Handa */
    if (c == '\t')
      c = current_buffer->tab_width;
    else if (c == '\n')
      c = 0;
    else if (C0_P(c) || c == DEL)
      c = !NILP (current_buffer->ctl_arrow) ? 2 : 4;
    else if (ASCII_P(c))
      c = 1;
    else if (VALID_LC (c) && !NILP (current_buffer->mc_flag))
      c = char_width[c];
    else
      c = 4;
  } else {
    if (NILP (current_buffer->mc_flag))
      c = 4;
    else
      c = char_width[CHAR_LEADING_CHAR (c)];
  }

  return (int)c;
}

strwidth (s)
     unsigned char *s;
{
  int i = 0;
  unsigned char *endp = s + strlen(s);

  while (s < endp) {
    i += charwidth (*s);
    s += charbytes (*s);
  }
  return i;
}

DEFUN ("string-width", Fstring_width, Sstring_width, 1, 1, 0,
  "Return number of columns STRING will occupy when displayed with mc-flag t.")
  (str)
     Lisp_Object str;
{
  CHECK_STRING (str, 0);
  return make_number (strwidth (XSTRING (str)->data));
}
/* end of patch */

DEFUN ("char-width", Fchar_width, Schar_width, 1, 1, 0,
  "Return number of columns a character of CHARSET occupies when displayed.\n\
CHARSET should be specified by a leading character.\n\
You can also provide a character as an argument.")
  (charset)
       Lisp_Object charset;
{
  CHECK_NUMBER (charset, 0);
  return make_number (charwidth (XFASTINT (charset)));
}

/* 92.8.2 by Y.Niibe */
DEFUN ("char-direction", Fchar_direction, Schar_direction, 1, 1, 0,
  "Return the direction of a character of CHARSET by\n\
  0 (left-to-right) or 1 (right-to-left).\n\
CHARSET should be specified by a leading character.\n\
You can also provide a character as an argument.")
  (charset)
       Lisp_Object charset;
{
  Lisp_Object val;
  unsigned int c;
  int direction;

  CHECK_NUMBER (charset, 0);
  c = XFASTINT (charset);
  if (MC_CHAR_P (c))
    c = CHAR_LEADING_CHAR (c);
  if (VALID_LC (c))
    direction = char_direction[c];
  else
    direction = LEFT_TO_RIGHT;

  XSET (val, Lisp_Int, direction);
  return val;
}
/* end of patch */

/* 94.6.3 by K.Handa */
DEFUN ("char-registry", Fchar_registry, Schar_registry, 1, 1, 0,
  "Return registry name of CHARSET.\n\
CHARSET should be specified by a leading character.\n\
You can also provide a character as an argument.")
  (charset)
     Lisp_Object charset;
{
  unsigned int c;

  CHECK_NUMBER (charset, 0);
  c = XFASTINT (charset);
  if (MC_CHAR_P (c))
    c = CHAR_LEADING_CHAR (c);
  if (VALID_LC (c) && char_registry[c & 0x7F])
    return (build_string (char_registry[c & 0x7F]));
  else
    return Qnil;
}
/* end of patch */

DEFUN ("char-description", Fchar_description, Schar_description, 1, 1, 0,
  "Return description of CHARSET.\n\
CHARSET should be specified by a leading character.\n\
You can also provide a character as an argument.")
  (charset)
     Lisp_Object charset;
{
  unsigned int c;

  CHECK_NUMBER (charset, 0);
  c = XFASTINT (charset);
  if (MC_CHAR_P (c))
    c = CHAR_LEADING_CHAR (c);
  if (VALID_LC (c) && char_description[c & 0x7F])
    return (build_string (char_description[c & 0x7F]));
  else
    return Qnil;
}

DEFUN ("chars-in-string", Fchars_in_string, Schars_in_string, 1, 1, 0,
  "Return number of characters in STRING.\n\
Each multilingual/composite character is also counted as one.")
  (string)
     Lisp_Object string;
{
  register Lisp_Object val;
  register unsigned char *p, *endp;
  register unsigned int i;

  CHECK_STRING (string, 0);

  p = XSTRING (string)->data; endp = p + XSTRING (string)->size;
  i = 0;
  while (p < endp) {		/* 92.11.16 by K.Handa */
    if (!NONASCII_P (*p)) i++;
    p++;
  }

  XFASTINT (val) = i;
  return val;
}

DEFUN ("char-boundary-p", Fchar_boundary_p, Schar_boundary_p, 1, 1, 0,
  "Return non nil value if POS is at character boundary.\n\
The value is:\n\
 0: if POS is at an ASCII character or end of range,\n\
 1: if POS is at a leading char of 2-byte sequence.\n\
 2: if POS is at a leading char of 3-byte sequence.\n\
 3: if POS is at a leading char of 4-byte sequence.\n\
 4: if POS is at a leading char of a composite character.\n\
If POS is out of range or not at character boundary, NIL is returned.")
  (pos)
     Lisp_Object pos;
{
  register Lisp_Object val;
  register unsigned char c;
  register int n;

  if (NILP (current_buffer->mc_flag))
    XSET (val, Lisp_Int, 0);
  else {
    CHECK_NUMBER_COERCE_MARKER (pos, 0);
    n = XINT (pos);
    if (n < BEGV || n >= ZV) return Qnil;
    c = FETCH_CHAR (n);
    if (NONASCII_P(c)) return Qnil;
    n = c == LCCMP ? 4 : char_bytes[c] - 1;
    XSET (val, Lisp_Int, n);
  }
  return val;
}
#endif /* emacs */

/* Composite char staffs */
struct cmpchar_info *cmpchar_table;
static int cmpchar_table_size;
int n_cmpchars;

struct cmpchar_hash_info {
  int size, used, *table;
};

#define CMPCHAR_HASH_SIZE 0xFFF

static struct cmpchar_hash_info cmpchar_hash[CMPCHAR_HASH_SIZE];

hash_cmpchar(str)
     unsigned char *str;
{
  unsigned int h;

  for (h = 0; *str; str++)
    h = ((h << 7) + (*str & 0x7F)) % CMPCHAR_HASH_SIZE;
  return (int)h;
}

search_cmpchar(str, len)
     unsigned char *str;
     int len;
{
  unsigned char *buf = (unsigned char *) xmalloc (len + 1);
  struct cmpchar_hash_info *hashp;

  bcopy (str, buf, len);
  buf[len] = 0;

  hashp = cmpchar_hash + hash_cmpchar (buf);
  if (hashp->used) {
    int i, idx;

    for (i = 0; i < hashp->used; i++) {
      idx = hashp->table[i];
      if (len == cmpchar_table[idx].len
	  && !bcmp(buf, cmpchar_table[idx].data, len)) {
	free(buf);
	return idx;
      }
    }
  }

  if (hashp->size == 0) {
    hashp->size = 8;
    hashp->used = 0;
    hashp->table = xmalloc (sizeof (hashp->table[0]) * hashp->size);
  } else if (hashp->size <= hashp->used) {
    hashp->size *= 2;
    hashp->table = xrealloc (hashp->table,
			     sizeof (hashp->table[0]) * hashp->size);
  }
  hashp->table[hashp->used++] = n_cmpchars;
  if (cmpchar_table_size == 0) {
    cmpchar_table_size = 8;
    cmpchar_table = (struct cmpchar_info *) xmalloc (sizeof (cmpchar_table[0])
						     * cmpchar_table_size);
  } else if (cmpchar_table_size <= n_cmpchars) {
    cmpchar_table_size *= 2;
    cmpchar_table = (struct cmpchar_info *) xrealloc (cmpchar_table,
						      sizeof (cmpchar_table[0])
						      * cmpchar_table_size);
  }
  cmpchar_table[n_cmpchars].len = len;
  cmpchar_table[n_cmpchars].data = buf;

  {
    GLYPH *g = (GLYPH *) alloca (sizeof (*g) * 16);
    unsigned char *p, *q;
    int i, width = 1;

    for (i = 0, p = buf + 1; i < 16 && *p; i++) {
      if (*p == 0xA0) {
	g[i] = *++p & 0x7F;
	p++;
      } else {
	*p -= 0x20;
	g[i] = STRtoCHAR (p, 4);
	if (char_width[*p] > width) width = char_width[*p];
	q = p + char_bytes[*p];
	*p += 0x20;
	p = q;
      }
    }
    cmpchar_table[n_cmpchars].glyph_len = i;
    cmpchar_table[n_cmpchars].glyph = (GLYPH *) xmalloc (sizeof (*g) * i);
    bcopy (g, cmpchar_table[n_cmpchars].glyph, sizeof (*g) * i);
    cmpchar_table[n_cmpchars].width = width;
  }

  return n_cmpchars++;
}

init_charset_once()
{
  int i,j;

/* 92.3.21, 92.9.2 by K.Handa - big change */
  bzero(char_description, sizeof (char *) * 128);
  bzero(char_data, 512);
  char_type = char_data;
  char_graphic = char_type + 128;
  char_final = char_graphic + 128;
  for (i = 0; i < 4; i++) for (j = 0; j < 128; j++)
    lc_table[i][j] = LCINV;
  for (i = 0; i < 256; i++) {	/* 92.8.2 by Y.Niibe, 93.6.2 by K.Handa */
    char_direction[i] = LEFT_TO_RIGHT;
    rev_lc_table[i] = LCINV;
  }
  for (i = 0; i < 128; i++) char_bytes[i] = char_width[i] = 1;
  for (; i < 0x90; i++) char_bytes[i] = 2, char_width[i] = 1;
  for (; i < 0x9A; i++) char_bytes[i] = 3, char_width[i] = 2;
  i = LCPRV11EXT;
  for (; i < LCPRV12EXT; i++) char_bytes[i] = 3, char_width[i] = 1;
  for (; i < LCPRV21EXT; i++) char_bytes[i] = 3, char_width[i] = 2;
  for (; i < LCPRV22EXT; i++) char_bytes[i] = 4, char_width[i] = 1;
  for (; i <= 0xFF; i++) char_bytes[i] = 4, char_width[i] = 2;
  for (i = 0; i <= 0xFF; i++) char_type[i] = TYPEINV;
/* 92.8.2, 93.5.22 by Y.Niibe */
  update_mc_table(LCPRV11,THREEBYTE,ONECOLUMN,TYPE94,0,0,DIRECTION_UNDEFINED,
		  "1-byte/1-column private character sets", 0);
  update_mc_table(LCPRV12,THREEBYTE,TWOCOLUMN,TYPE94,0,0,DIRECTION_UNDEFINED,
		  "1-byte/2-column private character sets", 0);
  update_mc_table(LCPRV21,FOURBYTE,ONECOLUMN,TYPE94N,0,0,DIRECTION_UNDEFINED,
		  "2-byte/1-column private character sets", 0);
  update_mc_table(LCPRV22,FOURBYTE,TWOCOLUMN,TYPE94N,0,0,DIRECTION_UNDEFINED,
		  "2-byte/2-column private character sets", 0);
  char_bytes[LCPRV3] = 3; char_width[LCPRV3] = 2;
  char_bytes[LCINV] = char_width[LCINV] = 1; /* 92.12.15 by K.Handa */
  update_mc_table(LCASCII, ONEBYTE, ONECOLUMN, TYPE94, GRAPHIC0, 'B',
		  LEFT_TO_RIGHT, "ASCII", "ISO8859-1");
/* end of patch */

#ifdef emacs
  for (i = 0; i < CMPCHAR_HASH_SIZE; i++)
    cmpchar_hash[i].size = 0;
#endif
}

#ifdef emacs

syms_of_charset ()
{
  defsubr (&Snew_character_set);
  defsubr (&Sleading_char);
  defsubr (&Sset_leading_char);
  defsubr (&Scharacter_set);
  defsubr (&Smake_character);
  defsubr (&Schar_component);
  defsubr (&Schar_leading_char);
  defsubr (&Schar_boundary_p);
  defsubr (&Sstring_width);	/* 93.6.17 by K.Handa */
  defsubr (&Schar_width);
  defsubr (&Schar_bytes);
  defsubr (&Schar_direction);  /* 92.8.2 by Y.Niibe */
  defsubr (&Schar_registry);	/* 94.6.3 by K.Handa */
  defsubr (&Schar_description);
  defsubr (&Schars_in_string);

  DEFVAR_INT ("lc-ascii", &Vlc_ascii,
    "Leading character for ASCII character, but never used.");
  XSET (Vlc_ascii, Lisp_Int, LCASCII);

  DEFVAR_INT ("lc-ltn1", &Vlc_ltn1,
    "Leading character for ISO8859-1, Latin alphabet No.1.");
  XSET (Vlc_ltn1, Lisp_Int, LCLTN1);

  DEFVAR_INT ("lc-ltn2", &Vlc_ltn2,
    "Leading character for ISO8859-2, Latin alphabet No.2.");
  XSET (Vlc_ltn2, Lisp_Int, LCLTN2);

  DEFVAR_INT ("lc-ltn3", &Vlc_ltn3,
    "Leading character for ISO8859-3, Latin alphabet No.3.");
  XSET (Vlc_ltn3, Lisp_Int, LCLTN3);

  DEFVAR_INT ("lc-ltn4", &Vlc_ltn4,
    "Leading character for ISO8859-4, Latin alphabet No.4.");
  XSET (Vlc_ltn4, Lisp_Int, LCLTN4);

  DEFVAR_INT ("lc-thai", &Vlc_thai,
    "Leading character for TIS620-2533, Thai.");
  XSET (Vlc_thai, Lisp_Int, LCTHAI);

  DEFVAR_INT ("lc-grk", &Vlc_grk,
    "Leading character for ISO8859-7, Greek alphabet.");
  XSET (Vlc_grk, Lisp_Int, LCGRK);

  DEFVAR_INT ("lc-arb", &Vlc_arb,
    "Leading character for ISO8859-6, Arabic alphabet.");
  XSET (Vlc_arb, Lisp_Int, LCARB);

  DEFVAR_INT ("lc-hbw", &Vlc_hbw,
    "Leading character for ISO8859-8, Hebrew alphabet.");
  XSET (Vlc_hbw, Lisp_Int, LCHBW);

  DEFVAR_INT ("lc-kana", &Vlc_kana,
    "Leading character for JIS X0201-1976, Japanese Katakana.");
  XSET (Vlc_kana, Lisp_Int, LCKANA);

  DEFVAR_INT ("lc-roman", &Vlc_roman,
    "Leading character for JIS X0201-1976, Japanese Roman.");
  XSET (Vlc_roman, Lisp_Int, LCROMAN);

  DEFVAR_INT ("lc-crl", &Vlc_crl,
    "Leading character for ISO8859-5, Cyrillic alphabet.");
  XSET (Vlc_crl, Lisp_Int, LCCRL);

  DEFVAR_INT ("lc-ltn5", &Vlc_ltn5,
    "Leading character for ISO8859-9, Latin alphabet No.5.");
  XSET (Vlc_ltn5, Lisp_Int, LCLTN5);

  DEFVAR_INT ("lc-jpold", &Vlc_jpold,
    "Leading character for JIS X0208-1978, Japanese characters.");
  XSET (Vlc_jpold, Lisp_Int, LCJPOLD);

  DEFVAR_INT ("lc-cn", &Vlc_cn,
    "Leading character for GB2312-1980, China (RPC) Hanzi.");
  XSET (Vlc_cn, Lisp_Int, LCCN);

  DEFVAR_INT ("lc-jp", &Vlc_jp,
    "Leading character for JIS X0208-1983, Japanese characters.");
  XSET (Vlc_jp, Lisp_Int, LCJP);

  DEFVAR_INT ("lc-jp2", &Vlc_jp2,
    "Leading character for JIS X0212-1990, Japanese characters.");
  XSET (Vlc_jp2, Lisp_Int, LCJP2);

  DEFVAR_INT ("lc-kr", &Vlc_kr,
    "Leading character for KS C5601-1987, Hangul character.");
  XSET (Vlc_kr, Lisp_Int, LCKR);

/* 93.4.29 by K.Handa */
  DEFVAR_INT ("lc-cns1", &Vlc_cns1,
    "Leading character for CNS 11643 Set 1.");
  XSET (Vlc_cns1, Lisp_Int, LCCNS1);

  DEFVAR_INT ("lc-cns2", &Vlc_cns2,
    "Leading character for CNS 11643 Set 2.");
  XSET (Vlc_cns2, Lisp_Int, LCCNS2);

/* 93.7.25 by K.Handa */
  DEFVAR_INT ("lc-cns14", &Vlc_cns14,
    "Leading character for CNS 11643 Set 14.");
  XSET (Vlc_cns14, Lisp_Int, LCCNS14);
/* end of patch */

  DEFVAR_INT ("lc-big5-1", &Vlc_big5_1,
    "Leading character for Big5 Level 1.");
  XSET (Vlc_big5_1, Lisp_Int, LCBIG5_1);

  DEFVAR_INT ("lc-big5-2", &Vlc_big5_2,
    "Leading character for Big5 Level 2.");
  XSET (Vlc_big5_2, Lisp_Int, LCBIG5_2);

  DEFVAR_INT ("lc-prv11", &Vlc_prv11,
    "Leading character for 1byte/1column private character sets.");
  XSET (Vlc_prv11, Lisp_Int, LCPRV11);

  DEFVAR_INT ("lc-prv12", &Vlc_prv12,
    "Leading character for 1byte/2column private character sets.");
  XSET (Vlc_prv12, Lisp_Int, LCPRV12);

  DEFVAR_INT ("lc-prv21", &Vlc_prv21,
    "Leading character for 2byte/1column private character sets.");
  XSET (Vlc_prv21, Lisp_Int, LCPRV21);

  DEFVAR_INT ("lc-prv22", &Vlc_prv22,
    "Leading character for 2byte/2column private character sets.");
  XSET (Vlc_prv22, Lisp_Int, LCPRV22);

  DEFVAR_INT ("lc-prv3", &Vlc_prv3,
    "Leading character for 3byte private character set.");
  XSET (Vlc_prv3, Lisp_Int, LCPRV3);

  DEFVAR_INT ("lc-prv11-ext", &Vlc_prv11_ext,
    "Extended leading character for 1byte/1column private character sets.");
  XSET (Vlc_prv11_ext, Lisp_Int, LCPRV11EXT);

  DEFVAR_INT ("lc-prv12-ext", &Vlc_prv12_ext,
    "Extended leading character for 1byte/2column private character sets.");
  XSET (Vlc_prv12_ext, Lisp_Int, LCPRV12EXT);

  DEFVAR_INT ("lc-prv21-ext", &Vlc_prv21_ext,
    "Extended leading character for 2byte/1column private character sets.");
  XSET (Vlc_prv21_ext, Lisp_Int, LCPRV21EXT);

  DEFVAR_INT ("lc-prv22-ext", &Vlc_prv22_ext,
    "Extended leading character for 2byte/2column private character sets.");
  XSET (Vlc_prv22_ext, Lisp_Int, LCPRV22EXT);

  DEFVAR_INT ("lc-prv3-ext", &Vlc_prv3_ext,
    "Extended leading character for 3byte private character sets.");
  XSET (Vlc_prv3_ext, Lisp_Int, LCPRV3EXT);

  DEFVAR_INT ("lc-invalid", &Vlc_invalid,
    "Invalid leading character.");
  XSET (Vlc_invalid, Lisp_Int, LCINV);

  DEFVAR_INT ("lc-composite", &Vlc_composite,
    "Leading character for a composite characer.");
  XSET (Vlc_composite, Lisp_Int, LCCMP);
}
#endif /* emacs */
