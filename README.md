# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access Kafka from Dyalog APL. 

## Scope

For the first milestone of this project, we aim to support the Producer and Consumer aspects only. This specifically means that there will be no Dyalog APL version of the `AdminClient` API which interacts with the cluster (and topic) configuration. All topic creation must therefore be done outside Dyalog APL.

Our aim is to provide as thin as possible a layer on top of librdkafka.

## Build and installation

At the moment, this library must be built from source. Clone this repository. You will need a C++ development toolchain installed.

### MacOS

Install the `librdkafka` via Homebrew:
```
brew install librdkafka
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

## Configuration

The configuration format is a table of key and value columns. Both the `Producer` and `Consumer` take the same configuration format. Here is what a consumer configuration could look like:

```apl
      config ← 0 2 ⍴⍬
      config⍪← 'bootstrap.servers' 'localhost:9092'
      config⍪← 'client.id' 'bhcgrs3550'
      config⍪← 'group.id' 'dyalog'
      config

bootstrap.servers  localhost:9092 
client.id          bhcgrs3550     
group.id           dyalog 
```
which specifies a client with the id `bhcgrs3550`, belonging to the consumer group `dyalog`, talking to the Kafka cluster with entry point `localhost:9092`. A corresponding `Producer` configuration example:

```apl
      config←0 2⍴⍬
      config⍪←'bootstrap.servers' 'localhost:9092'
      config⍪←'client.id' 'bhc'
      config

 bootstrap.servers  localhost:9092 
 client.id          bhc 
```

## Producer scope
Assumption: you have created a configuration table as per above.

1. Create a producer
    ```
    producer←⎕NEW Producer config
    ```

2. Produce a message
    
    A Kafka message is a tuple specifying the topic, the payload and the message key.Use the `produce` method to produce a message of `topic`, `payload`, `key`:
    ```
    producer.produce 'animals' 'tiger' 'key01'
    ```
    Where necessary, encode the payload:
    ```
    producer.produce 'invoices' (1⎕JSON invoice) customer_id
    ```
    To send produce multiple messages, use
    ```apl
    :For i :In ⍳n
        producer.produce 'animals' (100↑'Payload',⍕i) ('key',⍕4|i)
        :If 0=10|i
            producer.update_outstanding ⍝ Await successful delivery
        :EndIf
    EndFor
    ```
    We're looking at options for what a more ergonomic  asynchronous API would look like.
3. Delivery receipts
    
    Use `producer.delivery_report n` to see the `n` most recent production receipts.

4. Destroy the producer
    ```
    ⎕EX'producer'
    ```

## Consumer scope

Assumption: you have created a configuration table as per above.

1. Create a consumer instance
    ```apl
    consumer←⎕NEW Consumer config
    ```
2. Subscribe client instance to a set of Kafka topics
    ```apl
    consumer.subscribe 'animals' 'cars' 'plants'
    ```
3. Alternatively, assign specific topic partitions to a consumer instance
    If we have a partitioned topic, we can specify a specific partition. Let's say our `animals` topic was created as
    ```other
    kafka-topics.sh --bootstrap-server localhost:9092 --create --topic animals --partitions 3
    ```
    ```apl
    consumer.assign ⊂'animals' 2
    ```
    A partition is represented by a two-element vector of topic name and topic-id.

4. Consume

    Consume messages in a loop. Kafka parallelism is achieved by consumer groups and partitioned topics.
    ```apl
    :While 0=⊃rec←consumer.consume_record
        (2⊃rec).(Topic Payload Key Partition)
    :EndWhile
    ```
5. Destroy consumer
    ```apl
    ⎕EX'consumer'
    ```