# -*- makefile -*- Time-stamp: <03/07/10 00:20:54 ptr>
# $Id$

DBG_SUFFIX := -g

# Shared libraries:

SO_NAME        := lib${LIBNAME}_${COMPILER_NAME}.$(SO)
SO_NAMEx       := ${SO_NAME}${MAJOR}
SO_NAMExx      := ${SO_NAMEx}${MINOR}
SO_NAMExxx     := ${SO_NAMExx}${PATCH}

SO_NAME_OUT    := $(OUTPUT_DIR)/${SO_NAME}
SO_NAME_OUTx   := $(OUTPUT_DIR)/${SO_NAMEx}
SO_NAME_OUTxx  := $(OUTPUT_DIR)/${SO_NAMExx}
SO_NAME_OUTxxx := $(OUTPUT_DIR)/${SO_NAMExxx}

SO_NAME_DBG    := lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)
SO_NAME_DBGx   := ${SO_NAME_DBG}${MAJOR}
SO_NAME_DBGxx  := ${SO_NAME_DBGx}${MINOR}
SO_NAME_DBGxxx := ${SO_NAME_DBGxx}${PATCH}

SO_NAME_OUT_DBG    := $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}
SO_NAME_OUT_DBGx   := $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}
SO_NAME_OUT_DBGxx  := $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}
SO_NAME_OUT_DBGxxx := $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxxx}

SO_NAME_STLDBG    := lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)
SO_NAME_STLDBGx   := ${SO_NAME_STLDBG}${MAJOR}
SO_NAME_STLDBGxx  := ${SO_NAME_STLDBGx}${MINOR}
SO_NAME_STLDBGxxx := ${SO_NAME_STLDBGxx}${PATCH}

SO_NAME_OUT_STLDBG    := $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}
SO_NAME_OUT_STLDBGx   := $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}
SO_NAME_OUT_STLDBGxx  := $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}
SO_NAME_OUT_STLDBGxxx := $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxxx}

SO_INSTALL = $(INSTALL_LIB_DIR)/${SO_NAME_LONG}

# Static libraries:
A_NAME_SHORT = $(OUTPUT_DIR_A)/lib${LIBNAME}_${COMPILER_NAME}.$(ARCH)
A_NAME_SHORT_DBG = $(OUTPUT_DIR_A_DBG)/lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(ARCH)
A_NAME_SHORT_STLDBG = $(OUTPUT_DIR_A_STLDBG)/lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(ARCH)

A_NAME_LONG = lib${LIBNAME}_${COMPILER_NAME}.$(ARCH)
A_NAME_LONG_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(ARCH)
A_NAME_LONG_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(ARCH)

A_INSTALL = $(INSTALL_LIB_DIR)/${A_NAME_LONG}
A_INSTALL_DBG = $(INSTALL_LIB_DIR_DBG)/${A_NAME_LONG_DBG}
A_INSTALL_STLDBG = $(INSTALL_LIB_DIR_STLDBG)/${A_NAME_LONG_STLDBG}
