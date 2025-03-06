
progdirs=sera2 m2ps/lib-src

all: 
	for dir in $(progdirs);			\
	do								\
	  (cd $${dir}; $(MAKE) all);	\
	done

clean:
	for dir in $(progdirs);			\
	do								\
	  (cd $${dir}; $(MAKE) clean);	\
	done


