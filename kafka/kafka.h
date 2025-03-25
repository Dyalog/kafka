#pragma once
#if defined(FORmac) || defined(FORlinux) || defined(FORaix)
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
	/* Version 
	*	get the textual version and the native version number of Kafka
	*	args
	*     version buffer for the textual representation of the version of kafka
	*     len size of the buffer provided
	* 
	*  return kafka version in ative format (HEX)
	*/
	LIBRARY_API int Version(char* version, int len);

	/* Describe 
	*	Describes the functions exposed from the sharedlibrary
	* 
	*	args
	*		buffer space the the json description of the functions exported
	*		psize  size of the buffer provide in entry and size of space needed on exit 
	*/
	LIBRARY_API int32_t Describe(char* buffer, int32_t* psize);


	LIBRARY_API int InitKafka(void** kafka);
	//LIBRARY_API int UninitKafka(void* kafka);
	LIBRARY_API int UninitProducer(void* prod);
	LIBRARY_API int UninitConsumer(void* cons);

	LIBRARY_API int SetKafkaConf(void* kafka, char* key, char* val, char* errtxt, int *plen);

	LIBRARY_API int NewTopicConf(void** topicconf);
	LIBRARY_API int DelTopicConf(void* topicconf);
	LIBRARY_API int SetTopicConf(void* topicconf, char* key, char* val, char* errtxt, int *plen);

	//LIBRARY_API int CreateTopic(void** topic, void* kafka, char* topic_name, void* topic_conf);
	LIBRARY_API int NewTopicPartitionList(void** subscr);
	LIBRARY_API int SetTopicPartitionList(void* subscr, char* topic);
	
	LIBRARY_API int SubscribeConsumerTPList(void* kafka, void* subscr, char* errtxt, int *plen);
	LIBRARY_API int Produce(void* prod, char* topic, char* payload, uint32_t paylen, char* key, uint32_t keylen, int32_t partition, uint64_t* msgid, char* errtxt, int *plen);

	LIBRARY_API int Consume(void* cons, char* topic, uint32_t* topiclen, char* payload, uint32_t* paylen, char* key, uint32_t* keylen, int32_t* partition, char* errtxt, int *plen);
	LIBRARY_API int Commit(void* cons, void* subscr, int* async);

	LIBRARY_API int DeliveryReport(void* prod, unsigned long long* msgid, int* err, int* plength);
	LIBRARY_API int DRMessageError(int* err, char* errtxt, int *plen);
	// Do we need these?
}
