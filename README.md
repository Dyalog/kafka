# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access [Kafka](https://kafka.apache.org/) from Dyalog APL. 

> **Note** KAFKA is a [registered trademark](https://kafka.apache.org/trademark) of [The Apache Software Foundation](https://www.apache.org/) and has been licensed for use by [Dyalog Ltd](https://www.dyalog.com/). Dyalog Ltd has no affiliation with and is not endorsed by The Apache Software Foundation.

## Build and installation

### Precompiled Library
Compiled libraries for supported platforms — AIX, Linux, macOS (arm64 and x64), and Windows — are available on the [Releases](https://github.com/Dyalog/kafka/tags) page.

### Building from Source
You can also build the library from source. To do so, clone this repository and ensure you have a C++ development toolchain installed.

#### Windows

Open `kafka.sln` in Visual Studio 2022, and build the project.

Copy all the dlls from the OutDir (`kafka\x64\Debug` for 64 bit debug version) to a directory and `Start Apl` in that directory.

#### Linux

To build on linux for 64 bit 
```
PLATFORM=linux ./mk_kafka.sh 64
```
The output files are in distribution/linux/x64

#### AIX

TODO

#### MacOS

Install the `librdkafka` via Homebrew:
```
brew install librdkafka
```
Build the wrapper:
```
cd <path>/kafka
c++ -shared -fpic -oMACbin/kafka.dylib -DFORmac  kafka/kafka.cpp -lrdkafka -L/opt/homebrew/opt/librdkafka/lib -I/opt/homebrew/opt/librdkafka/include/librdkafka
```

or like linux but this requires dotnet to be installed to get the librdkafka package

```
PLATFORM=mac ./mk_kafka.sh 64
```


## Initialising

Now start Dyalog. You need a running Kafka instance. In the session, type

```apl
]cd path/to/kafka/repo
]link.create # aplsource
```

For users who can not use `]link`, it is possible to create the `kafka.dws` workspace.
The workspace can be built using the ]DBuild tool and saving it
```apl
]DBuild path/to/kafka.dyalogbuild
)WSID 
)SAVE
```
and then `)LOAD kafka.dws`.

Finally, initialise the library, passing as the argument the path where the `kafka.[so|a|dylib]` shared library was installed
```apl
Init 'path/to/dir/housing/shared/lib' ⍝ to load the dll
```

For further instructions, see the file [SPEC.md](SPEC.md), and sample code examples in [Samples](aplsource/Samples/).
