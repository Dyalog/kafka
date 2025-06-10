The  [Samples](../../aplsource/Samples/) directory contains code samples to show how to produce and consume messages:

- [Example](../../aplsource/Samples/Example.aplf): a single function showing how to produce messages on multiple topics and consume them.
- [SimpleProducer](../../aplsource/Samples/SimpleConsumer.aplf): produce n+1 messages on a topic. The partitions are assigned depending on the key value (input p is number of keys), and messages with same key will be written into the same partition.
- [SimpleConsumer](../../aplsource/Samples/SimpleConsumer.aplf): consume messages from a topic.
- [TopicProducer](../../aplsource/Samples/TopicConsumer.aplf): produce 2×n messages on topic1 and topic2. The partitions are assigned depending on the key value (input p is number of keys), and messages with same key will be written into the same partition.
- [TopicConsumer](../../aplsource/Samples/TopicConsumer.aplf): consume messages from multiple topic1 and topic2.
- [ConsumerAGroupAB](../../aplsource/Samples/ConsumerAGroupAB.aplf) and [ConsumerBGroupAB](../../aplsource/Samples/ConsumerBGroupAB.aplf): two consumers belonging to the same consumer group (GroupAB) share the messages when consuming from the same topics.
- [ConsumerCGroupC](../../aplsource/Samples/ConsumerCGroupC.aplf): consumer belonging to a different consumer group (GroupC), consumes the queue again, when reading topics already consumed, for example, by ConsumerA and ConsumerB.
- [ConsumerLoop](../../aplsource/Samples/ConsumerLoop.aplf): consume messages from two topics synchronously in a loop, while they are being produced.
- [ConsumerLoopCommit](../../aplsource/Samples/ConsumerLoop.aplf): consume messages from two topics synchronously in a loop and commits offsets to the server every 20 messages.
- [ProducerLoop](../../aplsource/Samples/ProducerLoop.aplf): produce messages for 120 seconds every 5 seconds on topic1 and topic2.
- [DirectSubscribe](../../aplsource/Samples/DirectSubscribe.aplf): simple example to show how to configure and subscribe to a topic list directly when the consumer is created.
- [ManualTPList](../../aplsource/Samples/ManualTPList.aplf): simple example to show how to manually create a new topic partition list, produce and consume messages from it. In this example we also show how the consumer can be configured by using consumer.configure.