#pragma once
#if defined(FORMAC) || defined(FORLINUX)
#define LIBRARY_API __attribute__((visibility("default")))
#endif

#if defined(FORWIN)
#ifdef KAFKA_EXPORTS
#    define LIBRARY_API __declspec(dllexport)
#else
#    define LIBRARY_API __declspec(dllimport)
#endif
#endif
extern "C"
{

	LIBRARY_API int Version(char* version, int len);

	LIBRARY_API int InitKafka(void** kafka);
	//LIBRARY_API int UninitKafka(void* kafka);
	LIBRARY_API int UninitProducer(void* prod);
	LIBRARY_API int UninitConsumer(void* cons);

	LIBRARY_API int SetKafkaConf(void* kafka, char* key, char* val, char* errtxt, int len);

	LIBRARY_API int NewTopicConf(void** topicconf);
	LIBRARY_API int DelTopicConf(void* topicconf);
	LIBRARY_API int SetTopicConf(void* topicconf, char* key, char* val, char* errtxt, int len);

	//LIBRARY_API int CreateTopic(void** topic, void* kafka, char* topic_name, void* topic_conf);
	LIBRARY_API int NewTopicPartitionList(void** subscr);
	LIBRARY_API int SetTopicPartitionList(void* subscr, char* topic);
	
	LIBRARY_API int SubscribeConsumerTPList(void* kafka, void* subscr, char* errtxt, int len);
	LIBRARY_API int Produce(void* prod, char* topic_name, int* partition, char* payload, uint64_t* msgid, char* errtxt, int len);
	LIBRARY_API int Produce1(void* prod, char* topic, char* payload, uint32_t paylen, char* key, uint32_t keylen, int32_t partition, uint64_t* msgid, char* errtxt, int len);

	LIBRARY_API int Consume(void* cons, char* consumed_msg, char* errtxt, int len);
	LIBRARY_API int Consume1(void* cons, char* topic, uint32_t* topiclen, char* payload, uint32_t* paylen, char* key, uint32_t* keylen, int32_t* partition, char* errtxt, int len);

	LIBRARY_API int DeliveryReport(void* prod, unsigned long long* msgid, int* err, int* plength);
	LIBRARY_API int DRMessageError(int* err, char* errtxt, int length);
	// Do we need these?
}