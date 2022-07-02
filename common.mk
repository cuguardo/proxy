
BOOST_DIR ?= $(firstword $(foreach LOCATION,$(subst :, ,${BOOST_PREFER_LOCATIONS}),$(wildcard ${LOCATION})))
ifneq ($(strip ${BOOST_DIR}),)
 BOOST_INCLUDE = $(BOOST_DIR)/include
 BOOST_LIBRARY = $(BOOST_DIR)/lib
 BOOST_RUNTIME = $(BOOST_DIR)/lib
endif

CC ?= gcc
CXX ?= g++
AR ?= gcc-ar
LD ?= ld
CONF ?= pkg-config
TEST ?= test
MD ?= mkdir
YACC ?= bizon
LEX ?= flex
#LIB_VER ?= 0
LIB_PREFIX = lib
SLIB_SUFFIX = .a
DLIB_SUFFIX = $(if ${LIB_VER},.so.${LIB_VER},.so)
OBJ_SUFFIX = .o
DEP_SUFFIX = .d .dep
C_SRC_SUFFIX = .c
C_HDR_SUFFIX = .h
CXX_SRC_SUFFIX = .cpp .cc
CXX_HDR_SUFFIX = .hpp .hh
PATH_DELIMITER = :
NAME_DELIMITER = /
RT = -Wl,-rpath=
SRC_SUFFIX = ${C_SRC_SUFFIX} ${CXX_SRC_SUFFIX}
HDR_SUFFIX = ${C_HDR_SUFFIX} ${CXX_HDR_SUFFIX}
PKG_CONFIG_PATH ?= ${NAME_DELIMITER}usr${NAME_DELIMITER}local${NAME_DELIMITER}lib${NAME_DELIMITER}pkgconfig

define conf
 CFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --cflags ${1})
 LDFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --libs ${1})
 STFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --static ${1})
endef

define cxxonf
 CXXFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --cflags ${1})
 LDFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --libs ${1})
 STFLAGS += $(shell export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}; ${CONF} --static ${1})
endef

define use
 ifneq ($(wildcard ${1}),)
  -include ${1}
 else
  -include ${2}
 endif
endef

override prefer = $(firstword $(subst ${PATH_DELIMITER}, ,${1}))
override vpath = $(foreach WILDCARD,$(addprefix %,${1}),$(eval vpath ${WILDCARD} ${2}))
override normalize = $(foreach CAT,$(subst ${PATH_DELIMITER}, ,${2}),$(patsubst ${CAT}/%,$(firstword ${1})/%,${3}))
override wildpath = $(foreach WILDCARD,${1},$(foreach CAT,$(subst ${PATH_DELIMITER}, ,${2}),$(wildcard ${CAT}/${WILDCARD})))
override wildfile = $(foreach WILDCARD,${1},$(foreach CAT,$(subst ${PATH_DELIMITER}, ,${2}),$(notdir $(wildcard ${CAT}/${WILDCARD}))))
override verify = $(foreach DIR,$1,$(shell ${MD} -p ${DIR}))
override cpkg = $(eval $(call cxxonf,${1}))
override cxxpkg = $(eval $(call conf,${1}))
override import = $(eval $(call use,${1}, ${2}))

override csrc = $(call wildpath,$(addprefix *,${C_SRC_SUFFIX}),${1})
override chdr = $(call wildpath,$(addprefix *,${C_HDR_SUFFIX}),${1})
override cxxsrc = $(call wildpath,$(addprefix *,${CXX_SRC_SUFFIX}),${1})
override cxxhdr = $(call wildpath,$(addprefix *,${CXX_HDR_SUFFIX}),${1})
override src = $(call csrc,${1}) $(call cxxsrc,${1})
override hdr = $(call chdr,${1}) $(call cxxhdr,${1})
override cobj = $(foreach WILDCARD,$(addprefix %,${C_SRC_SUFFIX}),$(patsubst ${WILDCARD},%$(firstword ${OBJ_SUFFIX}),$(filter ${WILDCARD},$(call csrc,${1}))))
override cxxobj = $(foreach WILDCARD,$(addprefix %,${CXX_SRC_SUFFIX}),$(patsubst ${WILDCARD},%$(firstword ${OBJ_SUFFIX}),$(filter ${WILDCARD},$(call cxxsrc,${1}))))
override obj = $(call cobj,$1) $(call cxxobj,$1)
override cdep = $(foreach WILDCARD,$(addprefix %,${C_SRC_SUFFIX}),$(patsubst ${WILDCARD},%$(firstword ${DEP_SUFFIX}),$(filter ${WILDCARD},$(call csrc,${1}))))
override cxxdep = $(foreach WILDCARD,$(addprefix %,${CXX_SRC_SUFFIX}),$(patsubst ${WILDCARD},%$(firstword ${DEP_SUFFIX}),$(filter ${WILDCARD},$(call cxxsrc,${1}))))
override dep = $(call cdep,$1) $(call cxxdep,$1)


$(call import,local.mk,local.mk.default)