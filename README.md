# ETHIOPIC POSTSCRIPT FROM LATIN SERA FILES

## NOTE
*This is an archived work last updated in 1996, it is not expected to
 build out-of-the-box.  Abundant and better quality solutions are
 available today.*


SERA2PS 2.3.8
=============

This package is an extension of the `any2ps' (v2.0 beta), utility developed by
Ken'ichi Handa (ntakahas@etl.go.jp).  "coco" and "any2ps" are a part of
Mule 2.2+ , sera2ps is the minimal subset of the Mule package that will enable
users without Mule to obtain the same postscript printing capability.
No less true than previously: "this is a hack".


INSTALLATION
------------

Read the "INSTALL" document for instructions.  If problems occur
you may contact me at yacob@padis.gn.apc.org for assistance in this
area.


HOW TO USE
----------

After successful installation you may use your favorite ASCII editor to write
a SERA style Latin file and print it in Ethiopic postscript by typing at the
command line:

	pssera myfile | lp

"lpr" may be substituted for "lp" or the command you usually use with your
postscript printer. If you have 'ghostview', or another postscript previewer,
and would like to view a file before printing do :

	pssera myfile > outfile 
	ghostview outfile
 
Eview v0.49 may also be used to preview the file SERA file.  pssera is a
script file that calls the SERA to Junet encoder 'sera2any'.  pssera
features these three options:


-l)  If your file begins with Latin script, you will either need to add a
     \ as the first character in the file.  Or perform the following:

                pssera -l lat myfile | lp

     Where "lat" is required to indicate the starting script if not Fidel.
     "gre" is available for Greek script.

-s)  If you would like to switch all of the blank spaces, " ", into Ethiopic
     word spaces (:), in the Ethiopic portions of your text, do as follows:


                pssera -s myfile | lp


-stats) Provides a Fidel table with statistical output of character
        occurrences into the files "fidel.stats" & fidel2.stats.

You may use the -s , -l , and -stats options together.
The files "docs/recipe", "docs/fidel" and "docs/fidel2" are provided samples.
Print these files from the sera2ps directory (where pssera is located) by:

                pssera docs/fidel | lp

You may move the pssera script to another directory, or make an alias for
it.  To use it from another directory you will need to edit the SERADIR
and M2PSDIR path of lines 11 & 12  in the pssera script.  This is required so
that pssera knows where to find the tools it uses.

An example:

                SERADIR=/u1/yacob/sera2ps/


ALSO!!  If you move pssera outside of the sera2ps directory you will need
to recompile (reperform the 'make' command) after editing:

m2ps/src/config.h  Line 160
---------------------------
    before:  #define BDF_PATH "fonts:../../fonts"
    after :  #define BDF_PATH "/my/new/path/fonts:fonts:../../fonts"

m2ps/src/paths.h  Line 23
-------------------------
    before:  #define PATH_DATA "m2ps/etc"
    after :  #define PATH_DATA "/my/new/path/etc:m2ps/etc" 

Where /my/new/path/xx  is the path that you would have to enter at the
copmmand line to `cd` TO find these files FROM where you will house the
pssera script.



BUGS
----

If bugs are found in the sera2any converter, please email them to:

    yacob@padis.gn.apc.org

If problems are encountered with Ethiopic printing, it is most probably
a problem with the font and may be adjusted readily.  Please send a
description of these problems to the same address (such as what characters
appear too close together or too far apart).  Updated version of the
font will be available by ftp at: 

    ftp://ftp.geez.org/pub/fidel/fonts/

and by World Wide Web at:

    http://abyssiniagateway.net/fidel/fonts.html



SERA
----

Extended documents for The System for Ethiopic Representation in ASCII (SERA)
by World Wide Web at : 

    http://abyssiniagateway.net/fidel/fidel.html
    http://abyssiniagateway.net/fidel/sera-faq.html

and

    ftp://ftp.geez.org/pub/fidel/sera-docs/


The file "docs/fidel" gives an outline of the ASCII system.
Also sera man page are now provided.
Briefly, characters are representation goes like :

     Consonants:
     me   mu   mi   ma   mE   m   mo   mWa

     Independent Vowels:
     e/a*  u/U  i    a/A  E   I   o/O   e3

     Independent Vowels Following a 6th Form Consonant:
              l'e   l'u   l'i   l'a   l'E   l'I   l'o
     also -->       lU          lA          lI    lO

     Consonants With 12 forms:
     hWe  hWu/hW'  hWi   hWa   hWE

     *NOTE:  "a" may be used in place of "e" for the first lone
             vowel ONLY in Amharic text zones.  See INSTALL for
             setting Amharic as a default language.



YOUR OBLIGATION
---------------

The stipulation for successfully using sera2ps is that you must use it to
write at least one letter home to your mother :-) or, if not available, to at
least one beloved relative who wishes you would write more often.  It would
also be very appreciated if you would drop a line of thanks to Abass Alamnehe
(abassa@neosoft.com) for providing freely the high quality font used for the
postscript printing (so that YOU WILL write that LETTER home to YOUR MOM).  



NOTE 
----

If you have Mule, it is totally unnecesary to convert SERA files into Junet
before viewing, unless the file name ends with ".sera" which is the
sera-save-as indicator for Mule 2.2+.

You may delete the files lib-src/*.o without consequence after compilation
is successful (use "make clean").
