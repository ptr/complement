# -*- Makefile -*- Time-stamp: <03/07/11 18:22:23 ptr>
# $Id$

DP_OUTPUT_DIR = | grep "^\#line 1 " | (echo -e 's|\([a-zA-Z]\):|/cygdrive/\1|g\nt next\n: next\n1s|^\#line 1 \(.*\)|$(OUTPUT_DIR)/$*.o $@ : $< \\\\|\nt\n$$s|^\#line 1 "\(.*\)"|\1|g\nt space\ns|^\#line 1 "\(.*\)"|\1\\\\|g\nt space\nd\n: space\ns| |\\\\ |g\ns|^|  |\ns|\\\\\\\\|/|g\n' > $(OUTPUT_DIR)/tmp.sed; sed -f $(OUTPUT_DIR)/tmp.sed; rm -f $(OUTPUT_DIR)/tmp.sed ) > $@; \
                  [ -s $@ ] || rm -f $@

DP_OUTPUT_DIR_DBG = | grep "^\#line 1 " | (echo -e 's|\([a-zA-Z]\):|/cygdrive/\1|g\nt next\n: next\n1s|^\#line 1 \(.*\)|$(OUTPUT_DIR_DBG)/$*.o $@ : $< \\\\|\nt\n$$s|^\#line 1 "\(.*\)"|\1|g\nt space\ns|^\#line 1 "\(.*\)"|\1\\\\|g\nt space\nd\n: space\ns| |\\\\ |g\ns|^|  |\ns|\\\\\\\\|/|g\n' > $(OUTPUT_DIR_DBG)/tmp.sed; sed -f $(OUTPUT_DIR_DBG)/tmp.sed; rm -f $(OUTPUT_DIR_DBG)/tmp.sed ) > $@; \
                  [ -s $@ ] || rm -f $@

DP_OUTPUT_DIR_STLDBG = | grep "^\#line 1 " | (echo -e 's|\([a-zA-Z]\):|/cygdrive/\1|g\nt next\n: next\n1s|^\#line 1 \(.*\)|$(OUTPUT_DIR_STLDBG)/$*.o $@ : $< \\\\|\nt\n$$s|^\#line 1 "\(.*\)"|\1|g\nt space\ns|^\#line 1 "\(.*\)"|\1\\\\|g\nt space\nd\n: space\ns| |\\\\ |g\ns|^|  |\ns|\\\\\\\\|/|g\n' > $(OUTPUT_DIR_STLDBG)/tmp.sed; sed -f $(OUTPUT_DIR_STLDBG)/tmp.sed; rm -f $(OUTPUT_DIR_STLDBG)/tmp.sed ) > $@; \
                  [ -s $@ ] || rm -f $@

