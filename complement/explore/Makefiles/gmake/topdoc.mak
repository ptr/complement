# Time-stamp: <08/01/17 09:39:54 ptr>
#
# Copyright (c) 2007,2008
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

.SUFFIXES:
.SCCS_GET:
.RCS_GET:

PHONY ?=

RULESBASE ?= $(SRCROOT)/Makefiles

all:	$(addsuffix .pdf,$(PDFNAMES))

distclean:
	if [ -f .gitignore ]; then rm -f `cat .gitignore`; fi
	rm -rf .out

PHONY += distclean

OUTPUT_DIR = .out

BASE_OUTPUT_DIR        := .out
PRE_OUTPUT_DIR         := $(BASE_OUTPUT_DIR)
OUTPUT_DIR             := $(PRE_OUTPUT_DIR)$(EXTRA_DIRS)
# catalog for auxilary files, if any
AUX_DIR                := $(PRE_OUTPUT_DIR)/.auxdir

OUTPUT_DIRS := $(OUTPUT_DIR)
# sort will remove duplicates:
OUTPUT_DIRS := $(sort $(OUTPUT_DIRS))

define createdirs
@for d in $@ ; do \
  if [ -e $$d -a -f $$d ] ; then \
    echo "ERROR: Regular file $$d present, directory instead expected" ; \
    exit 1; \
  elif [ ! -d $$d ] ; then \
    mkdir -p $$d ; \
  fi ; \
done
endef

$(OUTPUT_DIRS):
	$(createdirs)

$(AUX_DIR):
	$(createdirs)

PHONY += $(OUTPUT_DIRS) $(AUX_DIR)

ALLFIG =

define dep_mp
$(1)_MPS := $(patsubst %.fig,%.mps,$(notdir ${$(1)_SRC_FIG}))
ALLFIG += ${$(1)_SRC_FIG}
endef

$(foreach fig,$(PDFNAMES),$(eval $(call dep_mp,$(fig))))

define pdf_
$(1).pdf:	$($(1)_SRC_TEX) $($(1)_MPS) $($(1)_MP_MPS) $($(1)_EXTRA)
	pdflatex $$<
endef

define rule_mp
TMP1 := $$(OUTPUT_DIR)/$$(basename $$(notdir $(1))).mp
$$(TMP1):	$(OUTPUT_DIRS) $(1)
	fig2dev -L mp $(1) $$@

TMP2 := $$(OUTPUT_DIR)/$$(basename $$(notdir $(1))).0
$$(TMP2):	$$(TMP1)
	(cd $${OUTPUT_DIR}; mpost `basename $$<`)

TMP1 := $$(basename $$(notdir $(1))).mps
$$(TMP1):	$$(TMP2)
	ln -sf $$< $$@
endef

define dep_mps
$(1)_MP_MPS := $(patsubst %.mp,%.mps,$(notdir ${$(1)_SRC_MP}))
ALLMP += ${$(1)_SRC_MP}
endef

$(foreach mp,$(PDFNAMES),$(eval $(call dep_mps,$(mp))))

define rule_mps
TMP2 := $$(OUTPUT_DIR)/$$(basename $$(notdir $(1))).0
$$(TMP2):	$(1)
	sed -i -e 's/beginfig(.)/beginfig(0)/' $$<
	(cd $${OUTPUT_DIR}; mpost --tex=latex ../`basename $$<`)
	rm -f $(1)x

TMP1 := $$(basename $$(notdir $(1))).mps
$$(TMP1):	$$(TMP2)
	ln -sf $$< $$@
endef

$(foreach pdf,$(PDFNAMES),$(eval $(call pdf_,$(pdf))))
ALLFIG := $(sort $(ALLFIG))
ALLMP := $(sort $(ALLMP))
$(foreach fig,$(ALLFIG),$(eval $(call rule_mp,$(fig))))
$(foreach mp,$(ALLMP),$(eval $(call rule_mps,$(mp))))

.PHONY:	${PHONY}
