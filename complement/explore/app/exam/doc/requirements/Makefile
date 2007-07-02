PDFNAMES = UseCaseSpec

UseCaseSpec_SRC_TEX = UseCaseSpec.tex
UseCaseSpec_SRC_FIG = p1.fig p2.fig p3.fig p4.fig p5.fig debug-focus.fig

all:	UseCaseSpec.pdf

OUTDIR = .out

PDF_DIR_SRC =
FIG_BASE = 
define pdf_
PDF_DIR_SRC += $$(dir $${$(1)_SRC_TEX} $${$(1)_SRC_FIG} )
FIG_BASE += $$(basename $${$(1)_SRC_FIG}) 
$(1)_ALLMP   := $$(addsuffix .mp,$$(basename $${$(1)_SRC_FIG} ) )
$(1)_ALLMPS  := $$(addsuffix .mps,$$(basename $${$(1)_SRC_FIG} ) )
$(1)_ALL0    := $$(addsuffix .0,$$(basename $${$(1)_SRC_FIG} ) )

$(1)_MP      := $$(addprefix $$(OUTDIR)/,$${$(1)_ALLMP})
$(1)_MPS     := $$($(1)_ALLMPS)
$(1)_0       := $$(addprefix $$(OUTDIR)/,$${$(1)_ALL0})

$(1).pdf:       $$($(1)_SRC_TEX) $$($(1)_MPS)
	pdflatex $$<
endef

$(foreach pdf,$(PDFNAMES),$(eval $(call pdf_,$(pdf))))

# if sources disposed in several dirs, calculate appropriate rules

DIRS_UNIQUE_SRC := $(dir $(SRC_FIG) $(SRC_TEX) )
DIRS_UNIQUE_SRC := $(sort $(DIRS_UNIQUE_SRC) $(PDF_DIR_SRC))


define rule_mp
$$(OUTDIR$(1))/%.mp:	$(2)%.fig
	$$(chdirs)
	fig2dev -L mp $$< $$@
endef

define rule_mps
$(1).mps:	$$(OUTDIR)/$(1).0
	ln -sf $$< $$@

$$(OUTDIR)/$(1).0:	$$(OUTDIR)/$(1).mp
	(cd ${OUTDIR}; mpost `basename $$<`)
endef

define rules_m_
$(call rule_mp,,$(1))
endef

$(foreach mps,${FIG_BASE},$(eval $(call rule_mps,$$(notdir $(mps)))))

$(foreach dir,$(DIRS_UNIQUE_SRC),$(eval $(call rules_m_,$(dir))))

define chdirs
if [ ! -d ${OUTDIR} ]; then \
  mkdir -p ${OUTDIR}; \
fi
endef

.PHONY:	dirs

