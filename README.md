# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access Kafka from Dyalog APL. 

## Build and installation

At the moment, this library must be built from source. Clone this repository. You will need a C++ development toolchain installed.

### Windows

TODO

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
Now start Dyalog. You need a running Kafka instance. In the session, type

```apl
]cd path-to-kafka  
]link.create # aplsource
```
In the `Consumer` and `Producer` classes, modify the property `sharedlib`:
```apl
:field private shared sharedlib←'MACbin/kafka.dylib'
```