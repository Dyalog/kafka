# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access Kafka from Dyalog APL. 

## Build and installation

At the moment, this library must be built from source. Clone this repository. You will need a C++ development toolchain installed.

### Windows

Open `kafka.sln` in Visual Studio 2022, and build the project.

Copy all the dlls from the OutDir (`kafka\x64\Debug` for 64 bit debug version) to a directory and `Start Apl` in that directory.

### Linux

TODO

### AIX

TODO

### MacOS

Install the `librdkafka` via Homebrew:
```
brew install librdkafka
```
Build the wrapper:
```
cd <path>/kafka
c++ -shared -fpic -oMACbin/kafka.dylib -DFORMAC  kafka/kafka.cpp -lrdkafka -L/opt/homebrew/opt/librdkafka/lib -I/opt/homebrew/opt/librdkafka/include/librdkafka
```

## Initialising

Now start Dyalog. You need a running Kafka instance. In the session, type

```apl
]cd path/to/kafka/repo
]link.create # aplsource
```
Initialise the library, passing as the argument the path where the `kafka.[so|a|dylib]` shared library was installed:
```apl
Init 'path/to/dir/housing/shared/lib' ⍝ to load the dll
```
