KAFKA_MODS := \
	kafka \

ifeq ($(ARCH),x86_64)
ARCH=x64
endif

KAFKA=$(PWD)

DIST=$(KAFKA)/distribution/$(PLATFORM)/$(ARCH)/$(BITS)

BIN=$(KAFKA)/$(PLATFORM)$(ARCH)bin


ifeq ($(PLATFORM),WIN)

else  ifeq ($(PLATFORM),linux)
 CC=gcc
 CPP=g++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/linux-$(ARCH)/native/librdkafka.so
 EXT=so
 KAFKABINSRC=nuget
else  ifeq ($(PLATFORM),aix) 
 KAFKACFLAGS=-m$(BITS)
 KAFKALDFLAGS=-m$(BITS)  
 KAFKAEXTLIBS=-lssl -lcrypto
 CC=ibm-clang_r
 CPP=ibm-clang++_r
 KAFKAINC=$(KAFKA)/librdkafka/src
 KAFKALIBS=$(KAFKA)/librdkafka/src/librdkafka.a
 EXT=so
 KAFKABINSRC=build
 KAFKARDLICENSE=$(KAFKA)/librdkafka/LICENSES.txt
else  ifeq ($(PLATFORM),mac) 
 CC=cc
 CPP=c++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/osx-$(ARCH)/native/librdkafka.dylib
 KAFKARDLICENSE=~/.nuget/packages/librdkafka.redist/2.5.0/LICENSES.txt
 EXT=dylib
 KAFKABINSRC=nuget
else
CC=cc
CPP=c++
endif


KAFKA_OBJS:= $(KAFKA_MODS:%=$(BIN)/%.o)

all: $(DIST)/kafka.$(EXT) $(DIST)/librdkafka.$(EXT)


$(BIN)/kafka.$(EXT): $(KAFKA_OBJS) $(KAFKALIBS) 
	$(CPP) $(KAFKALDFLAGS) -shared -o $@   $(KAFKA_OBJS)  $(KAFKALIBS) $(KAFKAEXTLIBS)

$(BIN)/%.o: kafka/%.cpp $(BIN) $(KAFKAINC)
	$(CPP) $(KAFKACFLAGS) -c -o $@   -DFOR$(PLATFORM) -I $(KAFKAINC)  -fpic $<

$(BIN):
	mkdir -p $@

$(DIST):
	mkdir -p $@

$(DIST)/kafka.$(EXT): $(DIST) $(BIN)/kafka.$(EXT)  
	cp  $(BIN)/kafka.$(EXT) $@

$(DIST)/librdkafka.$(EXT): $(KAFKALIBS)
	cp $< $@
	cp $(KAFKARDLICENSE) $(DIST)/LICENSES.librdkafka


$(KAFKAINC): $(KAFKALIBS)


$(KAFKALIBS):  $(KAFKABINSRC)

$(PWD)/librdkafka:
	git clone -b dyalog-build git@github.com:Dyalog/librdkafka librdkafka

build: $(PWD)/librdkafka
	cd $(KAFKA)/librdkafka && ./configure --prefix=/home/bhc/kafkalib  --install-deps --cc=ibm-clang_r --cxx=ibm-clang++_r --CFLAGS="-D__aix" --mbits=64 --ARFLAGS=-X64 --LDFLAGS=" -lssl -lcrypto"
	cd $(KAFKA)/librdkafka && make libs

nuget: 
	cd $(BIN) && dotnet new classlib  --name kafka -o . --force
	cd $(BIN) && dotnet add package librdkafka.redist --version 2.5.0
	cd $(BIN) && dotnet publish

$(BIN)/librdkafka.$(EXT) : $(KAFKALIBS)
	cp $< $@ 

clean:
	rm -rf $(BIN)
	rm -rf $(DIST)
	rm -rf $(KAFKA)/librdkafka
