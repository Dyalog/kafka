KAFKA_MODS := \
	kafka \

BIN=$(PLATFORM)bin


ifeq ($(PLATFORM),WIN)

else  ifeq ($(PLATFORM),LINUX)
 CC=gcc
 CPP=g++
 KAFKALIBS=$(BIN)/Release/net8.0/publish/runtimes/linux-x64/native/librdkafka.so
 EXT=so
else  ifeq ($(PLATFORM),AIX) 
 CC=xlc
 CPP=xlc++
else  ifeq ($(PLATFORM),MAC) 
 CC=cc
 CPP=c++
 KAFKALIBS=$(BIN)/Release/net8.0/publish/runtimes/osx-arm64/native/librdkafka.dylib
 EXT=dylib
else
CC=cc
CPP=c++
endif



BIN=$(PLATFORM)bin

KAFKA_OBJS:= $(KAFKA_MODS:%=$(BIN)/%.o)

all: $(BIN)/kafka.$(EXT)

KAFKALIBS: 
	cd $(BIN) && dotnet new classlib  --name kafka -o .
	cd $(BIN) && dotnet add package librdkafka.redist --version 2.5.0
	cd $(BIN) && dotnet publish


$(BIN)/kafka.$(EXT): $(KAFKA_OBJS)
	$(CC) -shared -o $@  $(KAFKA_OBJS)

$(BIN)/%.o:%.cpp $(BIN)
	$(CC) -c -fpic -o $@  -DFOR$(PLATFORM)  $<

$(BIN):
	mkdir -p $@