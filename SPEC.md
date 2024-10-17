# Dyalog-Kafka 

The aim of the Dyalog-Kafka project is to provide a binding to part of the [Confluent librdkafka](https://github.com/confluentinc/librdkafka) library such that we can access Kafka from Dyalog APL. 

## Note

The interface presented below is a work in progress, and its semantics should not be relied upon. 

## Scope

For the first milestone of this project, we aim to support the `Producer` and `Consumer` aspects only. This means that there will be no Dyalog APL version of the `AdminClient` API which interacts with the cluster (and topic) configuration. All topic creation must therefore be done outside Dyalog APL. 

Our initial aim is to provide as thin as possible a layer on top of librdkafka, upon which richer Dyalog interfaces can be based. This falls into two abstraction layers: 
1. The API layer itself (`api.apln`), mapping the librdkafka functions into APL.
2. A convenience APL layer built on top of that.

The semantics of the lower layer are largely dictated by the wrapped library, with a few work-arounds required by the Dyalog FFI. We don't expect Dyalog APL application developers will want to use this layer directly.

The convenience APL layer will likely change rapidly and without notice as the design progresses. It is provided solely as an illustration of what will be covered, and we're still investigating the exact makeup of this layer. 

The rest of this document deals with the second layer, and has three aspects:
1. Configuration
2. Producer
3. Consumer

## Configuration

The configuration format is a table of key and value columns. The key names are the standard Kafka configuration keys. Both the `Producer` and `Consumer` take the same configuration format. Here is what a consumer configuration could look like:

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
    There is also a `Record` interface that packages up the message in a bundle:
    ```
    producer.produce_record ⎕NEW #.Record('animals' 'Blackbird' 'key01')
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

3. Delivery receipts
    
    Use `producer.delivery_report n` to see the `n` most recent production receipts.

    We're looking at options for what a more ergonomic  asynchronous API would look like.

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

    Consume messages in a loop. Kafka parallelism is achieved by consumer groups and partitioned topics. The `Record` interface allow for access by the names `Topic`, `Payload`, `Key`, `Partition`.
    ```apl
    :While 0=⊃rec←consumer.consume_record
        (2⊃rec).(Topic Payload Key Partition)
    :EndWhile
    ```
5. Destroy consumer
    ```apl
    ⎕EX'consumer'
    ```

## Examples

Note: the semantics are subject to change.

Here is a complete example showing both a `Producer` and a `Consumer`.

```apl
Example n;i  
⍝ Produce and consume `n` messages
⍝
⍝ Topics created with
⍝
⍝     kafka-topics.sh \
⍝       --bootstrap-server localhost:9092 \
⍝       --create --topic "animals" \
⍝       --partitions 3
⍝  
⍝     kafka-topics.sh \
⍝       --bootstrap-server localhost:9092 \
⍝       --create --topic "cars" \
⍝       --partitions 3
⍝
⍝     kafka-topics.sh \
⍝       --bootstrap-server localhost:9092 \
⍝       --create --topic "plants" \
⍝       --partitions 3

⍝ Set up the consumer first
config←0 2⍴⍬
config⍪←'bootstrap.servers' 'localhost:9092'
config⍪←'client.id' 'bhcgrs3550'
config⍪←'group.id' 'dyalog'

topic_list←'animals' 'cars' 'plants'
consumer←⎕NEW Consumer(config topic_list)

⍝ Now set up the producer
config←0 2⍴⍬
config⍪←'bootstrap.servers' 'localhost:9092'
config⍪←'client.id' 'bhc'

producer←⎕NEW Producer config

⍝ Produce onto the animals topic
:For i :In ⍳n
    producer.produce_record ⎕NEW #.Record('animals'(100↑'Payload',⍕i)('key',⍕4|i))
    :If 0=10|i
        producer.update_outstanding
    :EndIf
:EndFor

⍝ Produce a few messages to the other topics, too
producer.produce_record ⎕NEW #.Record('cars' 'ferrari' 'sportcars')
producer.produce_record ⎕NEW #.Record('plants' 'iris' 'flowers')
producer.update_outstanding

⍝ Let's drain the topics
:While 0=⊃cr←consumer.consume_record
    (2⊃cr).(Topic Payload Key Partition)
:EndWhile

⍝ Tidy up
⎕EX'producer'
⎕EX'consumer'
 ```

