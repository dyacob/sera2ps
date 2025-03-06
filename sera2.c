/*
**  sera2any v0.24
**
**  Generalized code to convert SERA (The System for Ethiopic Representation
**  in ASCII) to an arbitrary second system.  The generalized transcription
**  is achieved here by requiring a mapping of the output system into the
**  "fidel" address table (see fidel.h).
** 
**  This is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 1, or (at your option)
**  any later version.  See the included "COPYING" file.
** 
**  --Daniel Yaqob, 1995
** 
*/

#define SERA_UTIL "sera2any"
#define SERA_UTIL_MAJOR_VERSION 0.2
#define SERA_UTIL_MINOR_VERSION 4

#include <signal.h>
#include <stdio.h>
#include "fidel.h"
#include "libeth.h"

/* #define DEFLANG  amh */
/* #define DEFLANG  gez */
#define DEFLANG  tir 

extern char *FidelName[], *TeXFidel[];
extern int UniToTraditional[];
extern LS LnS[];

typedef struct {
         boolean yes, html;
         enum IOput  out;
       } STATSFlags;

static struct mystats {
         int  stat;
         char*  name;
       } Stats[UNITOTAL];

#ifdef __STDC__
int flag_reset ( int argc, char** argv, FILE** fp, STATSFlags** statsFlags, SERAFlags** seraFlags );
int gt ( struct mystats** A, struct mystats** B );
int get_traditional ( int trad, FCHAR* uni );
void print_stats( STATSFlags* statsFlags, SERAFlags* seraFlags ); 
int fout_java ( FCHAR* fstring, FILE* fp, SERAFlags* mysflags );
static void sig_term ( int signo );
# ifdef  SUPPORT_TEX
int fout_tex  ( FCHAR* fstring, FILE* fp, SERAFlags* mysflags );
# endif  /* SUPPORT_TEX */
#else
int flag_reset(),
int gt();
int get_traditional ();
void print_stats ();
int fout_java ();
static void sig_term ( );
# ifdef  SUPPORT_TEX
int fout_tex  ( );
# endif  /* SUPPORT_TEX */
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//  Main reads input switches, does file read-in, and sends output where   //
//  we want it.                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

void main (argc,argv)
  int argc;
  char **argv;
{

FCHAR* mystring;
FILE *inptr = stdin;
FILE* outptr = stdout;
SERAFlags* seraFlags;
STATSFlags* statsFlags;
int i, test = true;


  if (signal(SIGTERM, sig_term) == SIG_ERR)    /* catch kill commands */
    fprintf(stderr, "can't catch SIGTERM");
  if (signal(SIGINT, sig_term) == SIG_ERR)     /* catch Control-C     */
    fprintf(stderr, "can't catch SIGINT");

  flag_reset (argc, argv, &inptr, &statsFlags, &seraFlags);


  while (!feof(inptr) && test)
   {
     test = fidel_fgets (&mystring, WSIZE, inptr, seraFlags);
     switch ( seraFlags->out )
	   {

       /*  TeX and Java are not character codes schemes so we
	    *  handle I/O at this level and not in the library.
		*/

         case java:  
		   fout_java (mystring, outptr, seraFlags);
		   break;
#ifdef  SUPPORT_TEX
         case tex:  
         case latex:
		   fout_tex (mystring, outptr, seraFlags);
		   break;
#endif  /* SUPPORT_TEX */

         default:
		   fidel_fputs (mystring, outptr, seraFlags);
	   }
     if (statsFlags->yes)
       {
         i = 0;
         while ( mystring[i] )
           {
             if ( isethio (mystring[i]) )
               if ( isprivate(mystring[i]) )
                 Stats[(UNITOTAL-1) + (mystring[i] - PRIVATE_USE_END)].stat ++;
               else
                 Stats[mystring[i]-UNIFIDEL].stat ++;
             i++;
           }
       }
     free (mystring);
   }

  if (statsFlags->yes) 
    print_stats ( statsFlags, seraFlags );

/*-------Lets Be A Good Citizen-----------------*/

  fclose (inptr);
  free (seraFlags);
  free (statsFlags);
  exit (0);

}



/*-------------------------------------------------------------------------//
//                                                                         //
//  Print Table of Character Occurances                                    //
//                                                                         //
//-------------------------------------------------------------------------*/



void 
print_stats (statsFlags, seraFlags)
  STATSFlags* statsFlags;
  SERAFlags* seraFlags;
{

int trad, uniMap, charBase, i, mod, space=0, stop;
FCHAR uni=0, *uniP;
FILE* fp;
struct mystats* PStats[UNITOTAL];


  uniP =& uni;
  seraFlags->out = statsFlags->out;


  fp = fopen ( "fidel.stats", "w" );
  if ( statsFlags->html ) 
    fprintf(fp,"<p><hr><p>\n<pre>\n");
  else
    fprintf(fp,"-------------------------------------------------------------------------------------\n\n");

  for (trad = 0; trad < UNITOTAL; trad += ROW) 
    {
      charBase = get_traditional ( trad, uniP );

      mod = get_fmodulo ( uni ); 

      if ( uni == SPACE || uni == MYA || uni == ONE )
        if ( statsFlags->html ) 
          fprintf(fp,"<p><hr><p>\n");
        else
          fprintf(fp,"-------------------------------------------------------------------------------------\n\n");

      if ( uni == SPACE ) 
        space = 1;

      for (i=0; i<mod; i++) 
        {
          if ( (mod == 12 && i > 6) || (mod == 15) )
            uniMap = get_traditional (trad+i, uniP);
          else
            uniMap = charBase + i;
          fidel_sputc (uni, &Stats[uniMap].name, seraFlags);
          fprintf (fp, "%*s    ", 3+space, Stats[uniMap].name);
          uni++;
        }
      fprintf (fp, "\n");
      if ( mod == 12 )                           /* start over, uniMap was trashed */
        uniMap = get_traditional ( trad, uniP );
      for (i=0; i<mod; i++) 
        {
          if ( mod == 12 && i > 6 )
            uniMap = get_traditional (trad+i, uniP);
          else
            uniMap = charBase + i;
          fprintf (fp, "%*d    ", 3+space, Stats[uniMap].stat);
        }
      fprintf (fp,"\n\n");
      if (mod > 8)
        {
          if (mod == 12)
            trad += ROW;
          else if ( (uni-mod) == ONE )
            trad += 2;
          else if ( (uni-mod) == (TEN+1) )   /* (TEN+1) = TWENTY */
            trad += 2;
          else if ( isfpunct(uni) )
            trad = UNITOTAL;
        }

    }

  if ( statsFlags->html ) 
    fprintf(fp,"</pre>\n<p><hr><p>\n");
  else
    fprintf(fp,"-------------------------------------------------------------------------------------");
  fclose(fp);
 
  for ( i = 0; i < UNITOTAL; i++ )
    PStats[i] =& Stats[i]; 

  fp = fopen("fidel2.stats", "w");
  qsort ( PStats, UNITOTAL, sizeof(struct mystruct *), gt );
  space = stop = 0;

  if ( statsFlags->html ) 
    fprintf (fp, "<p><hr><p>\n<pre>");

  for (trad = 0; trad < UNITOTAL; trad += ROW) 
    {
      fprintf (fp,"\n");
      for (i=0; i < 8; i++) 
        fprintf (fp, "%0*d    ", 3+space, trad+i);
      fprintf (fp,"\n");
      stop = 0;

      for (i=0; stop != 8 && (trad+i) < UNITOTAL; i++) 
        if ( PStats[trad+i]->name != '\0' )
          {
            fprintf(fp, "%*s    ", 3+space, PStats[trad+i]->name);
            stop++;
          }
      fprintf (fp,"\n");
      stop = 0;

      for (i=0; stop != 8 && (trad+i) < UNITOTAL; i++) 
        if ( PStats[trad+i]->name != '\0' )
          {
            fprintf(fp, "%*d    ", 3+space, PStats[trad+i]->stat);
            stop++;
          }
      fprintf (fp,"\n\n");
      if (i > ROW)
        trad += (i-ROW);

    }
  if ( statsFlags->html ) 
    fprintf (fp, "</pre>\n<p><hr><p>\n");

  fclose(fp);
  return;

}


 /*****************************************************************************/
/*****************************************************************************/
/* get_traditional
/*
/* is here becuase fidel.map does not have a TraditionalToUni
/* ...maybe it should ?
/*
/******************************************************************************/
/*****************************************************************************/

int 
get_traditional ( trad, uni )
  int trad; 
  FCHAR* uni;
{

int uniMap;


  uniMap = *uni = 0;
  while ( UniToTraditional[uniMap] != trad && uniMap < UNITOTAL ) 
    (uniMap)++;

  if ( uniMap == UNITOTAL ) /* nothing found */
    return ( NIL );

  *uni = uniMap + UNIFIDEL;

  if ( *uni >= UNILAST+NUM_EXTEND )
    *uni = (*uni - (UNILAST + NUM_EXTEND) ) + PRIVATE_USE_END - NUM_SPECIAL;

  return ( uniMap ) ;

}


int 
gt ( A, B )
  struct mystats **A, **B;
{
  return ( (*B)->stat - (*A)->stat );
}


/*-------------------------------------------------------------------------//
//                                                                         //
//  Simple routine to read input options and reset the default flags.      //
//  The flags for this version are -l if a file starts in Latin and        //
//  -s to use Ethiopic word separators in Ethiopic text zones.             //
//                                                                         //
//-------------------------------------------------------------------------*/


int 
flag_reset (argc, argv, fp, statsFlags, seraFlags)
  int argc;
  char **argv;
  FILE** fp;
  STATSFlags** statsFlags;
  SERAFlags** seraFlags;
{

LS* default_lang =& LnS[DEFLANG];
char* Lnames2[NUMLANGS] = LNAMES2;
char* Lnames3[NUMLANGS] = LNAMES3;
register int i,j;


/*---------------- Define and Initialize Flag Values -------------------------*/

  *seraFlags                =  (SERAFlags *) malloc ( sizeof(SERAFlags) );
  (*seraFlags)->top         =  default_lang; 
  (*seraFlags)->minor       =  (*seraFlags)->top;
  (*seraFlags)->major       =& LnS[lat];
  (*seraFlags)->lastchar    =  '\0';
  (*seraFlags)->dos         =  0;
  (*seraFlags)->html        =  false;
  (*seraFlags)->qmark       =  false;
  (*seraFlags)->colon       =  false;
  (*seraFlags)->gspace      =  false;
  (*seraFlags)->verbatim    =  false;
  (*seraFlags)->lastpunct   =  false;
  (*seraFlags)->in          =  jun;
  (*seraFlags)->out         =  jun;
  (*seraFlags)->other_state =  false;

  *statsFlags               =  (STATSFlags *) malloc ( sizeof(STATSFlags) );
  (*statsFlags)->yes        =  false;
  (*statsFlags)->html       =  false;
  (*statsFlags)->out        =  sera;


  i = 0;

/* Who Am I? */

  if ( strncmp( &argv[strlen(argv[0])-8], "sera2any", 8) )
    if ( strstr(argv[0], "sera2jis") ) 
      (*seraFlags)->out  =  jis;
    else if ( strstr(argv[0], "sera2jun") || strstr(argv[0], "sera2ps") ) 
      (*seraFlags)->out  =  jun;
    else if ( strstr(argv[0], "sera2java") ) 
      (*seraFlags)->out  =  java;
    else if ( strstr(argv[0], "sera2uni") ) 
      (*seraFlags)->out  =  uni;
    else if ( strstr(argv[0], "sera2tex") ) 
#ifdef SUPPORT_TEX
      (*seraFlags)->out  =  tex;
#else
      {
        fprintf(stderr,"\a TeX Output Not Available By This Package \n");
        exit(0);
      }
#endif
    


/* What Do You Want From Me? */

  while ( ++i < argc )
    {
      if ( argv[i][0] == '-' ) 
        {
          switch ( toupper(argv[i][1]) ) 
            {
              case 'C' :  (*seraFlags)->colon = true;
                          break;
              case 'D' :  (*seraFlags)->out = debug;
                          break;

              case 'F' :  
#ifdef SUPPORT_DOS
                          (*seraFlags)->dos =  1;
#else
                            {
                              fprintf(stderr,"\a -fromdos option not available\n");
                              fprintf(stderr,"\a See ``README.dosio'' to enable DOS support\n");
                              exit(0);
                            }
#endif
                          break;

              case 'H' :  if ( !strcmp ( &argv[i][1], "html") )
                            (*seraFlags)->html  =  true;
                          break;

              case 'L' :  j=-1;
                          while( ++j < NUMLANGS && (strcmp(argv[i+1],Lnames2[j]) && strcmp(argv[i+1],Lnames3[j])) );
                          if (j < NUMLANGS ) 
                            (*seraFlags)->minor =& LnS[j];  
                          (*seraFlags)->top     =& LnS[j];  
                          if (j == lat)
                            (*seraFlags)->major = default_lang;
                          i++;
                          if (j == NUMLANGS) 
                            {
                              fprintf(stderr,"Language %s Not Supported\n",argv[i]);
                              exit(1);
                            }
                          break; 

              case 'O' :  i++;
                          if ( strstr ( argv[i], "-html" ) )
                            (*seraFlags)->html = true;
                          if ( !strncmp (argv[i], "jun", 3) )
                            (*seraFlags)->out  =  jun;
                          else if ( !strncmp (argv[i], "jis", 3) )
                            (*seraFlags)->out  =  jis;
                          else if ( !strncmp (argv[i], "jav", 3) )
                            (*seraFlags)->out  =  java;
                          else if ( !strncmp (argv[i], "uni", 3) )
                            (*seraFlags)->out  =  uni;
                          else if ( !strcmp (argv[i], "debug") )
                            (*seraFlags)->out = debug;
                          else if ( !strcmp (argv[i], "tex") )
#ifdef SUPPORT_TEX
                            (*seraFlags)->out  =  tex;
#else
                            {
                              fprintf(stderr,"\a TeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif

                          else if ( !strcmp (argv[i], "latex") )
#ifdef SUPPORT_TEX
                            (*seraFlags)->out  =  latex;
#else
                            {
                              fprintf(stderr,"\a LaTeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else if ( !strncmp (argv[i], "sera") )
                            (*seraFlags)->out  =  sera;

                          break;

              case 'Q' :  (*seraFlags)->qmark = true;
                          break;
              case 'S' :  if ( toupper(argv[i][2]) == 'T' )
                            {
                              (*statsFlags)->yes = true;            /* give character occurance statistics */
                              for ( j = 0; j < UNITOTAL; j++ )
                                Stats[j].stat = 0;
                              if ( argv[i+1] == '\0' || argv[i+1][0] == '-' )
                                break;
                              i++;

                              if ( strstr ( argv[i], "-html" ) )
                                (*statsFlags)->html = true;
                              if ( !strncmp (argv[i], "jun", 3) )
                                (*statsFlags)->out  =  jun;
                              else if ( !strncmp (argv[i], "jis", 3) )
                                (*statsFlags)->out  =  jis;
                              else if ( !strncmp (argv[i], "uni", 3) )
                                (*statsFlags)->out  =  uni;
                              else if ( !strncmp (argv[i], "sera", 4) )
                                (*statsFlags)->out  = sera;
                              else
                                i--;  /* no alternative option given, stick with default */
                            }
                          else
                            (*seraFlags)->gspace = true;  /* use Eth Word Sep. for " " */
                          break;

              case 'T' :  
#ifdef SUPPORT_DOS
                          (*seraFlags)->dos =  2;
#else
                            {
                              fprintf(stderr,"\a -todos option not available\n");
                              fprintf(stderr,"\a Recompile sera2any and libeth with ``make withdos'' \n");
                              exit(0);
                            }
#endif
                          break;

              case 'V' :  fprintf (stdout, "This is %s Version %0.1f%i\n", SERA_UTIL, SERA_UTIL_MAJOR_VERSION, SERA_UTIL_MINOR_VERSION);
                          exit(1);
                          break;
              default  :
                          fprintf (stderr, "\n\tUseage:  sera2any option[s] file\n");
                          fprintf (stderr, "\tTo substitute Latin spaces with Ge'ez wordspace:\n");
                          fprintf (stderr, "\t   -s\n");
                          fprintf (stderr, "\tTo substitute Latin colon with Ge'ez colon:\n");
                          fprintf (stderr, "\t(instead of wordspace):\n");
                          fprintf (stderr, "\t   -c\n");
                          fprintf (stderr, "\tTo switch the mappings for ?  and `? :\n");
                          fprintf (stderr, "\t   -q\n");
                          fprintf (stderr, "\tTo print tables of statistics in fidel.out and fidel2.out\n");
                          fprintf (stderr, "\t   -stats [encoding]\n");
                          fprintf (stderr, "\t           Encoding is one of: jun, jis, uni (default is sera)\n");
                          fprintf (stderr, "\tTo specify output encoding:\n");
                          fprintf (stderr, "\t   -o encoding-name \n");
                          fprintf (stderr, "\t      debug = Debug Mode \n");
                          fprintf (stderr, "\t      jis  = JIS \n");
                          fprintf (stderr, "\t      jun  = JUNET \n");
                          fprintf (stderr, "\t      java = Java \n");
#ifdef SUPPORT_TEX
                          fprintf (stderr, "\t      latex = LaTeX \n");
                          fprintf (stderr, "\t      tex = TeX \n");
#endif
                          fprintf (stderr, "\t      uni = Unicode \n");
                          fprintf (stderr, "\tTo set starting language:\n");
                          fprintf (stderr, "\t   -l iso639-name \n");
                          fprintf (stderr, "\t      am = amh = Amharic \n");
                          fprintf (stderr, "\t      gz = gez = Ge'ez   \n");
                          fprintf (stderr, "\t      la = lat = Latin   \n");
                          fprintf (stderr, "\t      ti = tir = Tigrigna\n");
                          fprintf (stderr, "\tTo force HTML conversion:\n");
                          fprintf (stderr, "\t   -html\n");
                          fprintf (stderr, "\t   or append -html to encoding name as in:\n");
                          fprintf (stderr, "\t      -o jis-html\n");
#ifdef SUPPORT_DOS
                          fprintf (stderr, "\tTo strip out DOS ^M end of line characters from input:\n");
                          fprintf (stderr, "\t   -fromdos\n");
                          fprintf (stderr, "\tTo insert DOS ^M end of line characters in output:\n");
                          fprintf (stderr, "\t   -todos\n");
#endif
                          fprintf (stderr, "\tEcho version number and quit:\n");
                          fprintf (stderr, "\t   -v\n");
                          fprintf (stderr, "\n\tTo contact the maintainers:\n");
                          fprintf (stderr, "\t   fisseha@cig.mot.com / yacob@padis.gn.apc.org\n\n");

                          exit(1);
                          break;
             
               }
           }  
         else 
           {
             fclose (*fp);
             if ((*fp = fopen(argv[i],"r")) == NULL)
               {
                 fprintf(stderr,"\n*** File Not Found \"%s\" ***\n",argv[i]);
                 exit(1);
               }
           }
  }

  return(true);

    /* else ignore, and use defaults */
}


/*
 *  Simple routine to prefix Java escape sequences to post-ANSI chars
 *  and then write them to the passed output stream.
 *    This may or may not be useful...
 */
int
fout_java (fstring, fp, mysflags)
  FCHAR* fstring;
  FILE* fp;
  SERAFlags* mysflags;
{

  int i=-1, count=0;

  if ( fp == NULL || feof(fp) || fstring == NULL || mysflags == NULL )
    return ( _S_FAIL );
  
  while ( fstring[++i] )
    if ( fstring[i] < ANSI )
      fprintf (fp, "%c", fstring[i]);
    else
      {
        count++;
        fprintf (fp, "\\u%4i", fstring[i]);
      }

  return ( count );

}


#ifdef  SUPPORT_TEX

/*
 *  Simple routine to prefix Java escape sequences to post-ANSI chars
 *  and then write them to the passed output stream.
 *    This may or may not be useful...
 */
int
fout_tex (fstring, fp, mysflags)
  FCHAR* fstring;
  FILE* fp;
  SERAFlags* mysflags;
{

  int i=-1, count=0;
  FCHAR fch;

  if ( fp == NULL || feof(fp) || fstring == NULL || mysflags == NULL )
    return ( _S_FAIL );
  
  while ( fch = fstring[++i] )
    if ( fch < ANSI )
      fprintf (fp, "%c", fstring[i]);
    else if ( fch >= UNIFIDEL && fch < UNIFIDEL+UNITOTAL )
      {
        count++;
        fch -= UNIFIDEL;
		if ( fstring[i+1] == GEMINATION )
		  {
            fprintf ( fp, "\\geminateG{\\%s}", TeXFidel[fch] );
			i++;
		  }
        else
          fprintf ( fp, "{\\%s}", TeXFidel[fch] );
      }
    else if ( fch >= PRIVATE_USE_BEGIN && fch <= PRIVATE_USE_END )
      {
        count++;
        fch  = (UNITOTAL-1)  + (fch - PRIVATE_USE_END);
        fprintf ( fp, "{\\%s}", TeXFidel[fch] );
      }

  return ( count );

}

#endif  /* SUPPORT_TEX */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//  sig_term catches the more violent break signals, presumably something  //
//  has gone awry...                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
sig_term ( int signo )
{
  if ( signo == SIGTERM || signo == SIGINT )
    {
      fprintf (stderr, "\nYou seem to have found a bug with %s version %0.1f%i\n", SERA_UTIL, SERA_UTIL_MAJOR_VERSION, SERA_UTIL_MINOR_VERSION);
   	  fprintf (stderr, "Please email your input file to the maintainer\n");
	  fprintf (stderr, "Daniel Yaocb:-  Daniel_Yacob_at_UNECA@un.org\n");
	  fprintf (stderr, "                                              Thank You!\n\n");
	  exit (0);
    }

}
