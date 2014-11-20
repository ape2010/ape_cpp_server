OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
VER_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo 'X86-64'; else echo 'X86'; fi;)LIB_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo '_X86-64'; else echo ''; fi;)

OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
CC=g++
VER=1.0.0.0
GTEST=$(HOME)/local/gtest
XLOG=$(HOME)/local/xlog
MONGOC=$(HOME)/local/mongo-c-driver
JSON=$(HOME)/local/json

DIR_LIST=../interface ../common ../net ../protocol ../tinyxml2 ../session ../event ./src
OutPut=build/test/
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


INC_DIR+=-I$(GTEST)/include -I$(XLOG)/include #-I$(MONGOC)/include/libbson-1.0 #-I$(MONGOC)/include/libmongoc-1.0
INC_DIR+=-I$(JSON)/include/json
#LIB_DIR

LIB_DIR=-L/usr/local/lib -L $(GTEST)/lib $(XLOG)/lib/libxlog.a -L$(JSON)/lib
#LIB_DIR+=-L$(MONGOC)/lib #$(MONGOC)/lib/libbson-1.0.a

LIBS=-lz -lrt -lgtest -lboost_thread -lboost_system -lpthread -Wall -ldl -Wl,--export-dynamic

LDFLAGS=$(LIB_DIR) $(LIBS)
CPPFLAGS=$(INC_DIR) $(DFLAGS)  -DTIXML_USE_STL

EXE1=./test

all:$(EXE1)
$(shell mkdir -p $(sort $(OBJ_DIRS)))
include $(DEPS)

$(EXE1):$(OBJS) $(OBJS2) $(OBJS3)
	$(CC) -g  -O2 -o $@ $^ $(LDFLAGS)


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
	rm -Rf ./build
	rm -rf $(EXE1)
codelen:
	find $(NEW_CODE_PATH) \( -name "*.cc"  -name "*.cpp" -o -name "*.h" -o -name "*.c" \) -exec cat {} \;|sed -e 's/\"/\n\"\n/g;s/\([^\/]\)\(\/\*\)/\1\n\2\n/g;'|sed  -e '/^\"/{:a;N;/\".*\"/!ba;s/\".*\".*//;N;/\"/!d;b}' -e '/^\/\*/{s/\/\*.*\*\///;/\/\*/{:b;N;/\/\*.*\*\//!bb;s/\/\*.*\*\///}}' -e 's/\/\/.*//g' |sed -e '/^[[:space:]]*$$/d'|wc -l
srczip:
	zip -r ./$(EXE1)_src_$(VER).zip * -x *.o *.d *.svn *.zip *.a *.so $(EXE1) *.svn-work *.svn-base *.so.* *.d.* *.svn/*
binzip:
	if [ ! -d "release" ];  then mkdir release; fi;
	cd release;if [ ! -d "lib" ];  then mkdir lib; fi;
	cd release;if [ ! -d "plugin" ];  then mkdir plugin; fi;
	cp ./AutoTestServer ./release
	cp -r ./service ./release
	cp ./log.properties ./release/log.properties
	cp ./*.xml ./release/

	ldd ./AutoTestServer>sys_so_111_232_876_23;cp `awk  '{if(substr($$3,1,4)!="/lib"&&substr($$3,1,8)!="/usr/lib")print $$3}' sys_so_111_232_876_23` ./release/lib/;rm -rf sys_so_111_232_876_23
	cd release; zip -r ../AutoTestServer_$(VER)_$(OS)_Gcc$(GCC)_X86.zip *
	rm -rf release


