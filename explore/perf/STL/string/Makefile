# -*- Makefile -*- Time-stamp: <04/07/14 23:08:07 ptr>
# $Id$

DIRS = add add-ropes find find-ropes ops ops-ropes params params-ropes params-ref params-ref-ropes params-short

all:
	for d in $(DIRS); do \
	  (cd $$d; make); \
	done


clean:
	for d in $(DIRS); do \
	  (cd $$d; make clean); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; make depend); \
	done
