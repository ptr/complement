# -*- Makefile -*- Time-stamp: <05/12/27 00:00:15 ptr>

DIRS := stemecho stemload
timeprog := ../../app/utils/time/obj/gcc/so/time

all:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} release-shared); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} depend); \
	done

clean:	
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clean); \
	done
	rm -rf stemload/log

clobber:	
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clobber); \
	done
	rm -rf stemload/log

test:
	(cd stemecho; ../${timeprog} -o ../echo.dat obj/gcc/so/stemecho 2>/dev/null &)
	(cd stemload; rm -rf log; mkdir log; ../${timeprog} -o ../load.dat obj/gcc/so/stemload)
	pkill -TERM stemecho
	cat stemload/log/*_log.* | sort -n -k 2 | awk 'BEGIN { m = 0.0 } { if ( NR == 1 ) { m = $$2; }; print $$1, $$2 - m, $$3; }' > load.log
	awk 'BEGIN {i1 = 0.0; n = 0; sq1 = 0; dt = 60.0; t0 = 0.0; } { if ( ($$2 - t0) > dt ) { print t0 + 0.5 * dt, (i1 / n), sqrt((sq1 - (i1^2)/n ) / n), n / dt; t0 += dt; i1 = $$3; n = 1; sq1 = $$3^2; } else { i1 += $$3; ++n; sq1 += $$3^2; }; } END {print t0 + 0.5 * dt, (i1 / n), sqrt((sq1 - (i1^2)/n ) / n), n / dt; }' load.log > stat.dat

.PHONY:	all depend clean clobber test

