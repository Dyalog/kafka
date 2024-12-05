KAFKA_MODS := \
	kafka \

ifeq ($(ARCH),x86_64)
ARCH=x64
endif

DIST=distribution/$(PLATFORM)/$(ARCH)/$(BITS)

BIN=$(PLATFORM)$(ARCH)bin


ifeq ($(PLATFORM),WIN)

else  ifeq ($(PLATFORM),linux)
 CC=gcc
 CPP=g++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/linux-$(ARCH)/native/librdkafka.so
 EXT=so
else  ifeq ($(PLATFORM),AIX) 
 CC=xlc
 CPP=xlc++
else  ifeq ($(PLATFORM),mac) 
 CC=cc
 CPP=c++
 KAFKAINC=~/.nuget/packages/librdkafka.redist/2.5.0/build/native/include/librdkafka
 KAFKALIBS=~/.nuget/packages/librdkafka.redist/2.5.0/runtimes/osx-$(ARCH)/native/librdkafka.dylib
 EXT=dylib
else
CC=cc
CPP=c++
endif


KAFKA_OBJS:= $(KAFKA_MODS:%=$(BIN)/%.o)

all: $(DIST)/kafka.$(EXT) $(DIST)/librdkafka.$(EXT)


$(BIN)/kafka.$(EXT): $(KAFKA_OBJS) $(KAFKALIBS) 
	$(CPP) -shared -o $@   $(KAFKA_OBJS)  $(KAFKALIBS)

$(BIN)/%.o: kafka/%.cpp $(BIN) $(KAFKAINC)
	$(CPP) -c -o $@   -DFOR$(PLATFORM) -I $(KAFKAINC)  -fpic $<

$(BIN):
	mkdir -p $@

$(DIST):
	mkdir -p $@

$(DIST)/kafka.$(EXT): $(DIST) $(BIN)/kafka.$(EXT)  
	cp  $(BIN)/kafka.$(EXT) $@

$(DIST)/librdkafka.$(EXT) : $(KAFKALIBS)
	cp $< $@

$(KAFKAINC): $(KAFKALIBS)

$(KAFKALIBS): 
	cd $(BIN) && dotnet new classlib  --name kafka -o . --force
	cd $(BIN) && dotnet add package librdkafka.redist --version 2.5.0
	cd $(BIN) && dotnet publish

$(BIN)/librdkafka.$(EXT) : $(KAFKALIBS)
	cp $< $@ 