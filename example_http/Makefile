OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
VER_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo 'X86-64'; else echo 'X86'; fi;)LIB_PT=$(shell bit=`getconf LONG_BIT`;if [ $$bit -eq 64 ];  then echo '_X86-64'; else echo ''; fi;)

OS=$(shell uname)$(shell uname -a | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
GCC=$(shell gcc --version | sed -n '1p' | perl -nle 'print $$1 if /\s+([0-9]\.\d+)/')
CC=g++
VER=1.0.0
XLOG=$(HOME)/local/xlog
THRIFT = $(HOME)/local/thrift
APE=$(HOME)/local/ape

DIR_LIST=./business ./student_gen_cpp ./coh  ./main
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


INC_DIR+=-I/usr/include -I$(XLOG)/include -I$(THRIFT)/include -I$(APE)/include

#LIB_DIR

LIB_DIR=-L/usr/local/lib $(XLOG)/lib/libxlog.a  $(THRIFT)/lib/libthrift.a $(APE)/lib/libape.a
#-ltcmalloc
LIBS=-lz -lrt -lboost_thread -lboost_system -lpthread -Wall -ldl -Wl,--export-dynamic --whole-archive

LDFLAGS=$(LIB_DIR) $(LIBS)
CPPFLAGS=$(INC_DIR) $(DFLAGS)  -DTIXML_USE_STL

EXE1=./test_ape_http

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
	rm -Rf $(OutPut)
	rm -rf $(EXE1)
codelen:
	find $(NEW_CODE_PATH) \( -name "*.cc"  -name "*.cpp" -o -name "*.h" -o -name "*.c" \) -exec cat {} \;|sed -e 's/\"/\n\"\n/g;s/\([^\/]\)\(\/\*\)/\1\n\2\n/g;'|sed  -e '/^\"/{:a;N;/\".*\"/!ba;s/\".*\".*//;N;/\"/!d;b}' -e '/^\/\*/{s/\/\*.*\*\///;/\/\*/{:b;N;/\/\*.*\*\//!bb;s/\/\*.*\*\///}}' -e 's/\/\/.*//g' |sed -e '/^[[:space:]]*$$/d'|wc -l
srczip:
	zip -r ./$(EXE1)_src_$(VER).zip * -x *.o *.d *.svn *.zip *.a *.so $(EXE1) *.svn-work *.svn-base *.so.* *.d.* *.svn/*
binzip:
	if [ ! -d "release" ];  then mkdir release; fi;
	cd release;if [ ! -d "lib" ];  then mkdir lib; fi;
	cd release;if [ ! -d "plugin/httptransfer" ];  then mkdir -p plugin/httptransfer; fi;
	cd release;if [ ! -d "plugin/rapidtransfer" ];  then mkdir -p plugin/rapidtransfer; fi;
	cd release;if [ ! -d "plugin/weaktransfer" ];  then mkdir -p plugin/weaktransfer; fi;
	cp ./beacon ./release/
	cp -r ./conf ./release/conf
	cp  ./run.sh ./release/run.sh
	cp  $(HOME)/local/json/lib/libjson.so.0.1.0 ./release/lib/libjson.so.0

	ldd ./beacon>sys_so_111_232_876_23;cp `awk  '{if(substr($$3,1,4)!="/lib"&&substr($$3,1,8)!="/usr/lib")print $$3}' sys_so_111_232_876_23` ./release/lib/;rm -rf sys_so_111_232_876_23
	cd release; zip -r ../Beacon$(VER)_$(OS)_Gcc$(GCC)_X86.zip *
	rm -rf release

EXEDIR=apelus-beacon-server
packet:
	if [ ! -d "release/usr/local/$(EXEDIR)" ];  then mkdir -p release/usr/local/$(EXEDIR); fi;
	if [ ! -d "release/usr/local/$(EXEDIR)/lib" ];  then mkdir -p release/usr/local/$(EXEDIR)/lib; fi;
	if [ ! -d "release/usr/local/$(EXEDIR)/conf" ];  then mkdir -p release/usr/local/$(EXEDIR)/conf; fi;
	if [ ! -d "release/usr/local/$(EXEDIR)/plugin" ];  then mkdir -p release/usr/local/$(EXEDIR)/plugin; fi;

	cp $(EXE1) ./release/usr/local/$(EXEDIR)/
	cp ./conf/* ./release/usr/local/$(EXEDIR)/conf/
	cp ./run.sh ./release/usr/local/$(EXEDIR)/
	cp  $(HOME)/local/json/lib/libjson.so.0.1.0 ./release/usr/local/$(EXEDIR)/lib/libjson.so.0
	cp -r ./plugin/* ./release/usr/local/$(EXEDIR)/plugin/
	cp -r ./dist/DEBIAN ./release/
	cp -r ./dist/etc ./release/

	chmod -R +x ./release/*
	chmod +x ./release/usr/local/$(EXEDIR)/*
	chmod 755 ./release/DEBIAN/*
	ldd ./$(EXE1)>sys_so_111_232_876_23;cp `awk  '{if(substr($$3,1,4)!="/lib"&&substr($$3,1,8)!="/usr/lib")print $$3}' sys_so_111_232_876_23` ./release/usr/local/$(EXEDIR)/lib/;rm -rf sys_so_111_232_876_23
#	cd release;tar -czhf ../$(EXE1)_$(VER)_$(OS)_Gcc$(GCC).tar.gz * --exclude=".svn" --exclude=".git" --exclude="*.pyc" --exclude="*.log"
	mv release $(EXEDIR)_$(VER)_amd64;tar -czhf ./$(EXEDIR)_$(VER)_amd64.tar.gz $(EXEDIR)_$(VER)_amd64 --exclude=".svn" --exclude=".git" --exclude="*.pyc" --exclude="*.log"
	#dpkg --build release/ $(EXE1)-1.0.0_i386.deb
	rm -rf $(EXEDIR)_$(VER)_amd64
