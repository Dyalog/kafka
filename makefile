KAFKA_MODS := \
	kafka \


BIN=$(PLATFORM)bin


ifeq ($(PLATFORM),WIN)

else  ifeq ($(PLATFORM),LINUX)
 CC=gcc
 CPP=g++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/linux-x64/native/librdkafka.so
 EXT=so
else  ifeq ($(PLATFORM),AIX) 
 CC=xlc
 CPP=xlc++
else  ifeq ($(PLATFORM),MAC) 
 CC=cc
 CPP=c++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/osx-arm64/native/librdkafka.dylib
 EXT=dylib
else
CC=cc
CPP=c++
endif


KAFKA_OBJS:= $(KAFKA_MODS:%=$(BIN)/%.o)

all: $(BIN)/kafka.$(EXT) $(BIN)/librdkafka.$(EXT)


$(BIN)/kafka.$(EXT): $(KAFKA_OBJS) $(KAFKALIBS) 
	$(CC) -shared -o $@  $(KAFKA_OBJS)  $(KAFKALIBS)

$(BIN)/%.o: kafka/%.cpp $(BIN) $(KAFKAINC)
	$(CC) -c -o $@  -DFOR$(PLATFORM) -I $(KAFKAINC)  -fpic $<

$(BIN):
	mkdir -p $@

$(KAFKAINC): $(KAFKALIBS)

$(KAFKALIBS): 
	cd $(BIN) && dotnet new classlib  --name kafka -o . --force
	cd $(BIN) && dotnet add package librdkafka.redist --version 2.5.0
	cd $(BIN) && dotnet publish

$(BIN)/librdkafka.$(EXT) : $(KAFKALIBS)
	cp $< $@ 