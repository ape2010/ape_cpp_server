OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
VER_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo 'X86-64'; else echo 'X86'; fi;)LIB_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo '_X86-64'; else echo ''; fi;)

OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
CC=g++
VER=1.0.0
XLOG=$(HOME)/local/xlog

INSTALLDIR=$(HOME)/local/ape

DIR_LIST=./common ./interface ./net ./protocol ./session ./tinyxml2 ./event
OutPut=build/
NEW_CODE_PATH=./
#SOURCE CODE
CC_SRC=$(shell find  $(DIR_LIST)   -name "*.cc" )
CC_SRC2=$(shell find  $(DIR_LIST)   -name "*.c" )
CC_SRC3=$(shell find  $(DIR_LIST)   -name "*.cpp" )

#OBJECTS
CC_OBJS=$(patsubst %.cc,./$(OutPut)/%.o,$(CC_SRC))
CC_OBJS2=$(patsubst %.c,./$(OutPut)/%.o,$(CC_SRC2))
CC_OBJS3=$(patsubst %.cpp,./$(OutPut)/%.o,$(CC_SRC3))
OBJS=$(CC_OBJS)
OBJS2=$(CC_OBJS2)
OBJS3=$(CC_OBJS3)
#DEPS
DEPS=$(patsubst %.o,%.d,$(OBJS))

define OBJ_MKDIR
  OBJ_DIRS+=./$(OutPut)/$(1)
endef
CC_DIRS=$(shell find $(DIR_LIST) -type d|sed -e '/.svn/d')
	#@echo $(CC_DIRS)
$(foreach dir,$(CC_DIRS),$(eval $(call OBJ_MKDIR,$(dir))))

#DEPS
DEPS=$(patsubst %.o,%.d,$(OBJS))
INC_DIR=
#INCLUDE DIR
define SAFE_MKDIR
  INC_DIR+=-I $(1)
endef
$(foreach dir,$(CC_DIRS),$(eval $(call SAFE_MKDIR,$(dir))))


INC_DIR+=-I/usr/include -I$(XLOG)/include

#LIB_DIR

LIB_DIR=-L/usr/local/lib
#-ltcmalloc
LIBS=-Wall, --export-dynamic

LDFLAGS=$(LIB_DIR) $(LIBS)
CPPFLAGS=$(INC_DIR) $(DFLAGS)  -DTIXML_USE_STL

EXE1=$(OutPut)/libape.a

all:$(EXE1)
$(shell mkdir -p $(sort $(OBJ_DIRS)))
include $(DEPS)

$(EXE1):$(OBJS) $(OBJS2) $(OBJS3)
	ar -r $@ ./build/*/*.o


./$(OutPut)/%.o:%.cc
	$(CC) -g -O2 -o $@ -c -fPIC $< $(CPPFLAGS)
./$(OutPut)/%.d:%.cc
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

./$(OutPut)/%.o:%.cpp
	$(CC) -g -O2 -o $@ -c -fPIC $< $(CPPFLAGS)
./$(OutPut)/%.d:%.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

./$(OutPut)/%.o:%.c
	$(CC) -g  -O2 -o $@ -c -fPIC $< $(CPPFLAGS)
./$(OutPut)/%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(patsubst %.d,%.o,$@) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm -Rf $(OutPut)
	rm -rf $(EXE1)
codelen:
	find $(NEW_CODE_PATH) \( -name "*.cc"  -name "*.cpp" -o -name "*.h" -o -name "*.c" \) -exec cat {} \;|sed -e 's/\"/\n\"\n/g;s/\([^\/]\)\(\/\*\)/\1\n\2\n/g;'|sed  -e '/^\"/{:a;N;/\".*\"/!ba;s/\".*\".*//;N;/\"/!d;b}' -e '/^\/\*/{s/\/\*.*\*\///;/\/\*/{:b;N;/\/\*.*\*\//!bb;s/\/\*.*\*\///}}' -e 's/\/\/.*//g' |sed -e '/^[[:space:]]*$$/d'|wc -l
srczip:
	zip -r ./$(EXE1)_src_$(VER).zip * -x *.o *.d *.svn *.zip *.a *.so $(EXE1) *.svn-work *.svn-base *.so.* *.d.* *.svn/*
install:
	if [ ! -d $(INSTALLDIR)/include/ape ];  then mkdir -p $(INSTALLDIR)/include/ape; fi;
	if [ ! -d $(INSTALLDIR)/lib ];  then mkdir -p $(INSTALLDIR)/lib; fi;
	cp ./common/*.h $(INSTALLDIR)/include/ape/
	cp ./interface/*.h $(INSTALLDIR)/include/ape/
	cp ./tinyxml2/*.h $(INSTALLDIR)/include/ape/
	cp ./net/netthreadholder.h $(INSTALLDIR)/include/ape/
	cp ./net/netservice.h $(INSTALLDIR)/include/ape/
	cp ./net/sessioncallback.h $(INSTALLDIR)/include/ape/
	cp ./event/*.h $(INSTALLDIR)/include/ape/
	cp ./protocol/*.h $(INSTALLDIR)/include/ape/
	cp $(EXE1) $(INSTALLDIR)/lib
