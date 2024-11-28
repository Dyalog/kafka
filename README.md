# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access [Kafka](https://kafka.apache.org/) from Dyalog APL. 

> **Note** KAFKA is a [registered trademark](https://kafka.apache.org/trademark) of [The Apache Software Foundation](https://www.apache.org/) and has been licensed for use by [Dyalog Ltd](https://www.dyalog.com/). Dyalog Ltd has no affiliation with and is not endorsed by The Apache Software Foundation.

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

For further instructions, see the file [SPEC.md](SPEC.md), and sample code examples:

1. [Sample 1](aplsource/Sample.aplf)
2. [Sample 2](aplsource/Sample2.aplf)
3. [Sample 3](aplsource/Sample3.aplf)
4. [Sample 4](aplsource/Sample4.aplf)
