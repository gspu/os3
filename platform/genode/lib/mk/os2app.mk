include $(REP_DIR)/mk/osfree.mk

CC_CXX_WARN_STRICT = 
SHARED_LIB = yes
SRC_CC = app.cc
LIBS = base libc compat

vpath %.cc $(REP_DIR)/src/lib/os2app
