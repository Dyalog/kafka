The  [Samples](../../aplsource/Samples/) directory contains code samples to show how to produce and consume messages:

- [Example](../../aplsource/Samples/Example.aplf): a single function showing how to produce messages on multiple topics and consume them.
- [SimpleProducer](../../aplsource/Samples/SimpleConsumer.aplf): produces n+1 messages on a topic. The partitions are assigned depending on the key value (input p is the number of keys), messages with same key will be written into the same partition.
- [SimpleConsumer](../../aplsource/Samples/SimpleConsumer.aplf): consumes messages from a topic.
- [TopicProducer](../../aplsource/Samples/TopicConsumer.aplf): produces 2×n messages on topics "topic1" and "topic2". The partitions are assigned depending on the key value (input p is number of keys), messages with same key will be written into the same partition.
- [TopicConsumer](../../aplsource/Samples/TopicConsumer.aplf): consumes messages from topics "topic1" and "topic2".
- [ConsumerAGroupAB](../../aplsource/Samples/ConsumerAGroupAB.aplf) and [ConsumerBGroupAB](../../aplsource/Samples/ConsumerBGroupAB.aplf): two consumers belonging to the same consumer group (GroupAB) share the messages when consuming from the same topics.
- [ConsumerCGroupC](../../aplsource/Samples/ConsumerCGroupC.aplf): consumer belonging to a different consumer group (GroupC), consumes the queue again, when reading topics already consumed, for example, by ConsumerA and ConsumerB.
- [ConsumerLoop](../../aplsource/Samples/ConsumerLoop.aplf): consumes messages from two topics synchronously, while they are being produced, in a loop. The consumer will stop listening if no message has been consumed within 60 seconds.
- [ConsumerLoopCommit](../../aplsource/Samples/ConsumerLoop.aplf): consumes messages from two topics synchronously in a loop and commit offsets to the server every 20 messages.
- [ProducerLoop](../../aplsource/Samples/ProducerLoop.aplf): produces messages for 120 seconds every 5 seconds on topic1 and topic2.
- [DirectSubscribe](../../aplsource/Samples/DirectSubscribe.aplf): simple example to show how to configure and subscribe to a topic list directly when the consumer is created.
- [ManualTPList](../../aplsource/Samples/ManualTPList.aplf): simple example to show how to manually create a new topic partition list, produce and consume messages from it. This example also shows how the consumer can be configured by using the "consumer.configure" interface.