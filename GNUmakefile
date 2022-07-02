include common.mk

PREFIX ?= ~/.local
INST_BIN ?= ${PREFIX}/bin
INST_LIB ?= ${PREFIX}/lib

ROOT ?= $(abspath $(shell pwd))
SPATH = .
HPATH = .
OPATH = ./obj
DPATH = ./obj
IPATH = . ${ROOT}/inc ${ROOT}/src
LPATH = . ${ROOT}/lib
BPATH = . ${ROOT}/bin
RPATH = .
LIBS = libboost_filesystem libboost_program_options libboost_regex libpthread
OBJS =
DEFS = _REENTRANT

# DEBUG-related settings. Use `DEBUG_LEVEL=3 make' to build debugging version
OPTIMIZE_LEVEL ?= 2
DEBUG_LEVEL ?= 0
ifneq ($(DEBUG_LEVEL), 0)
 OPTIMIZE_LEVEL = 0
 CFLAGS += -DDEBUG
 CXXFLAGS += -DDEBUG
endif

ARCH = arch=native tune=native
TUNE = -pipe -fPIC
CTUNE = ${TUNE} -std=gnu99
CXXTUNE = ${TUNE} -std=gnu++11

INCLUDE = $(addprefix -I,$(subst ${PATH_DELIMITER}, ,${IPATH}))
LIB = $(addprefix -L,$(subst ${PATH_DELIMITER}, ,${LPATH}))
RTL = $(addprefix ${RT},$(subst ${PATH_DELIMITER}, ,${RPATH}))

CFLAGS += ${CTUNE} $(addprefix -D,${DEFS}) -g${DEBUG_LEVEL} -O${OPTIMIZE_LEVEL} $(addprefix -m,${ARCH}) -Wall ${INCLUDE}
CXXFLAGS += ${CXXTUNE} $(addprefix -D,${DEFS}) -g${DEBUG_LEVEL} -O${OPTIMIZE_LEVEL} $(addprefix -m,${ARCH}) -Wall ${INCLUDE}
LDFLAGS = ${TUNE} ${OBJS} ${LIB} ${RTL} $(foreach ITEM,${LIBS},$(patsubst ${LIB_PREFIX}%,-l%,${ITEM})) -Wl,--no-undefined
STFLAGS = -static -Wl,-static
SHFLAGS = -export-dynamic -shared -Wl,-shared
ARFLAGS = ruv

CSRC = $(call csrc,${SPATH})
CHDR = $(call chdr,${HPATH})
CXXSRC = $(call cxxsrc,${SPATH})
CXXHDR = $(call cxxhdr,${HPATH})
COBJ = $(call normalize,${OPATH},${SPATH},$(call cobj,${SPATH}))
CXXOBJ = $(call normalize,${OPATH},${SPATH},$(call cxxobj,${SPATH}))
CDEP = $(call normalize,${DPATH},${SPATH},$(call cdep,${SPATH}))
CXXDEP = $(call normalize,${DPATH},${SPATH},$(call cxxdep,${SPATH}))
SRC = ${CSRC} ${CXXSRC}
HDR = ${CHDR} ${CXXHDR}
DEP = ${CDEP} ${CXXDEP}
OBJ = ${COBJ} ${CXXOBJ}

OUT_BIN = $(call prefer,${BPATH})
OUT_LIB = $(call prefer,${LPATH})
OUT_OBJ = $(call prefer,${OPATH})
OUT_DEP = $(call prefer,${DPATH})
PATHS = $(sort ${OUT_OBJ} ${OUT_DEP} ${OUT_LIB} ${OUT_BIN})


BASENAME ?= reverseproxyhttp
EXEC = ${OUT_BIN}${NAME_DELIMITER}${BASENAME}
DLIB = ${OUT_LIB}${NAME_DELIMITER}${LIB_PREFIX}${BASENAME}${DLIB_SUFFIX}
SLIB = ${OUT_LIB}${NAME_DELIMITER}${LIB_PREFIX}${BASENAME}${SLIB_SUFFIX}
TARGETS ?= ${EXEC}

$(call vpath,${HDR_SUFFIX},${HPATH})
$(call vpath,${SRC_SUFFIX},${SPATH})
$(call vpath,${OBJ_SUFFIX},${OPATH})
$(call vpath,${DEP_SUFFIX},${DPATH})

all: ${PATHS} ${TARGETS}

dlib: ${OUT_LIB} ${DLIB}

slib: ${OUT_LIB} ${SLIB}

${OUT_OBJ}/%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

${OUT_OBJ}/%.o: %.cpp
	$(CXX) ${CXXFLAGS} -c $< -o $@

${OUT_OBJ}/%.o: %.cc
	$(CXX) ${CXXFLAGS} -c $< -o $@

${OUT_DEP}/%.d: %.c ${OUT_DEP}
	$(CC) -M ${CFLAGS} -c $< -o $@

${OUT_DEP}/%.d: %.cpp ${OUT_DEP}
	$(CXX) -M ${CXXFLAGS} -c $< -o $@

${OUT_DEP}/%.d: %.cc ${OUT_DEP}
	$(CXX) -M ${CXXFLAGS} -c $< -o $@

${EXEC}: ${OBJ}
	$(CXX) -o $@ $^ ${LDFLAGS}

${DLIB}: ${OBJ}
	$(CXX) ${SHFLAGS} -Wl,-soname,$@ -o $@ $^ ${LDFLAGS}

${SLIB}: ${OBJ}
	$(AR) ${ARFLAGS} $@ $^

${PATHS}:
	$(call verify,$@)

${COBJ}: ${CHDR}

${CXXOBJ}: ${CXXHDR}

depend: ${DEP}
	@echo INC: ${INCLUDE}
	@echo LIB: ${LIB}
	@echo RTL: ${RTL}
	@echo SRC: ${SRC}
	@echo HDR: ${HDR}
	@echo DEP: ${DEP}
	@echo OBJ: ${OBJ}
	@echo \<DEPENDENCE MAKEFILE_LIST=\"${MAKEFILE_LIST}\"\>
	@cat -n ${DEP}
	@echo \</DEPENDENCE\>

clean:
	$(RM) ${OBJ}
	$(RM) ${TARGETS}

distclean: clean
	$(RM) ${TARGETS} ${DEP}

install: ${DLIB}
	install -d ${INST_LIB}
ifeq ($(DEBUG), TRUE)
	install $< ${INST_LIB}/
else
	install -s $< ${INST_LIB}/
endif

uninstall:
	rm -f ${INST_LIB}/${DLIB}

.PHONY: all depend clean distclean

#.SECONDARY: ${DEP}

-include ${DEP}
