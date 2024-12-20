#if defined(FORWIN)
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#elif defined(FORmac) ||defined(FORaix) || defined(FORlinux)
#include <unistd.h>
#else
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rdkafka.h"
#include "kafka.h"

#if defined(FORmac) || defined(FORlinux) || defined(FORaix)
int strncpy_s(char* dst, size_t ds, const char* src, size_t ss)
{
	strncpy(dst, src, ss);
	return 0;
}
#define min(a,b) ((a)<(b)?(a):(b))
#endif



unsigned long long global_counter=0; // added =0

typedef struct {
	unsigned long long msg_id;
	rd_kafka_resp_err_t err;
} delivery_report;

typedef struct {
	int counter;
	int counter_limit;
	delivery_report** drs;
} delivery_reports;

typedef struct {
	rd_kafka_conf_t* conf;
	rd_kafka_t* rk;
	rd_kafka_message_t* msg;
} kafka_struct;


static void on_delivery(rd_kafka_t* rk, const rd_kafka_message_t* rkmessage, void* opaque)
{
	delivery_reports* drs = (delivery_reports*)rd_kafka_opaque(rk);

	if (drs->counter >= drs->counter_limit)
	{
		//extend the drs array;
		delivery_report** drs_tmp = (delivery_report**)calloc(2 * drs->counter_limit, sizeof(delivery_report*));
		memcpy(drs_tmp, drs->drs, drs->counter_limit * sizeof(delivery_report*));
		free(drs->drs);
		drs->drs = drs_tmp;
		drs->counter_limit = 2 * drs->counter_limit;
	}

	delivery_report* dr = (delivery_report*)calloc(1, sizeof(delivery_report));

	dr->msg_id = (unsigned long long)rkmessage->_private;
	dr->err = rkmessage->err;

	drs->drs[drs->counter] = dr;
	drs->counter++;
}


LIBRARY_API int Version(char* version, int len)
{
	const char* kafkaver = rd_kafka_version_str();
	strncpy_s(version, len, kafkaver, 1+strlen(kafkaver));

	return rd_kafka_version();
}


LIBRARY_API int InitKafka(void** kafka)
{
	kafka_struct* kf = (kafka_struct*)calloc(1, sizeof(kafka_struct));
	*kafka = (void*)kf;

	return 0;
}


LIBRARY_API int UninitProducer(void* prod)
{
	kafka_struct* pr = (kafka_struct*)prod;

	if (pr->rk != NULL) {
		rd_kafka_flush((rd_kafka_t*)pr->rk, 500);
		rd_kafka_destroy((rd_kafka_t*)pr->rk);
	}
	free(pr);

	return 0;
}

LIBRARY_API int UninitConsumer(void* cons)
{
	kafka_struct* co = (kafka_struct*)cons;
	// Close consumer
	rd_kafka_consumer_close(co->rk);
	if (co->rk != NULL) {
		// Destroy the consumer.
		rd_kafka_destroy(co->rk);
	}
	free(co);

	return 0;
}

LIBRARY_API int SetKafkaConf(void* kafka, char* key, char* val, char* errtxt, int *plen)
{
	rd_kafka_conf_res_t res;
	size_t len = *plen;
	*errtxt = 0;
	kafka_struct* kf = (kafka_struct*)kafka;
	if (kf->conf == NULL) {
		rd_kafka_conf_t* config = rd_kafka_conf_new();
		kf->conf = config;
	}

	res=rd_kafka_conf_set((rd_kafka_conf_t*)kf->conf, key, val, errtxt, len);
	if (res == RD_KAFKA_CONF_OK)
		*plen = 0;
	else
		*plen = (int)strlen(errtxt);

	return (int) res;
}



LIBRARY_API int Produce(void* prod, char* topic,  char* payload, uint32_t paylen, char* key,uint32_t keylen,  int32_t partition, uint64_t* msgid, char* errtxt, int *plen)
{
	int kerr = 0;
	size_t len = *plen;
	*errtxt = 0;

	kafka_struct* pr = (kafka_struct*)prod;

	if (pr->rk == NULL) {

		// Delivery CB
		rd_kafka_conf_set_dr_msg_cb(pr->conf, on_delivery);
		delivery_reports* drs = (delivery_reports*)calloc(1, sizeof(delivery_reports));
		drs->counter = 0;
		drs->counter_limit = 100;
		drs->drs = (delivery_report**)calloc(100, sizeof(delivery_report*));
		rd_kafka_conf_set_opaque(pr->conf, (void*)drs);

		rd_kafka_t* rk = rd_kafka_new(RD_KAFKA_PRODUCER, pr->conf, errtxt, len);
		pr->rk = rk;
		if (NULL==rk)
			*plen = 0;
		else
			*plen =(int) strlen(errtxt);

		//ok?
		pr->conf = NULL;
	}

	
	*msgid = global_counter++;

	kerr = rd_kafka_producev(pr->rk,
		RD_KAFKA_V_PARTITION(partition),
		RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
		RD_KAFKA_V_VALUE(payload, paylen),
		RD_KAFKA_V_KEY(key, keylen),
		RD_KAFKA_V_TOPIC(topic),
		RD_KAFKA_V_OPAQUE((void*)*msgid),
		RD_KAFKA_V_END
	);
	return kerr;
}



//LIBRARY_API int CreateTopic(void** topic, void* kafka, char* topic_name, void* topic_conf)
//{
//	kafka_struct* kf = (kafka_struct*)kafka;
//	rd_kafka_topic_new(kf->rk, topic_name, (rd_kafka_topic_conf_t*)topic_conf);
//	rd_kafka_topic_t* t = rd_kafka_topic_new(kf->rk, topic_name, (rd_kafka_topic_conf_t*)topic_conf);
//	*topic = (void*)t;
//	return 0;
//}
//

LIBRARY_API int NewTopicPartitionList(void** subscr)
{
	rd_kafka_topic_partition_list_t* subscription = rd_kafka_topic_partition_list_new(1);
	*subscr = (void*)subscription;

	return 0;
}


LIBRARY_API int SetTopicPartitionList(void* subscr, char* topic)
{
	
	rd_kafka_topic_partition_list_t* subscription = (rd_kafka_topic_partition_list_t*)subscr;
	rd_kafka_topic_partition_list_add(subscription, (const char*) topic, 0);

	return 0;
}

LIBRARY_API int SubscribeConsumerTPList(void* kafka, void* subscr, char* errtxt, int *plen)
{
	rd_kafka_resp_err_t res;
	size_t len = *plen;
	*errtxt = 0;

	kafka_struct* kf = (kafka_struct*)kafka;
	rd_kafka_topic_partition_list_t* subscription = (rd_kafka_topic_partition_list_t*)subscr;

	rd_kafka_t* rk = rd_kafka_new(RD_KAFKA_CONSUMER, kf->conf, errtxt, len);
	kf->rk = rk;
	if (NULL!=rk)
		*plen = 0;
	else
		*plen = (int)strlen(errtxt);

	res = rd_kafka_subscribe(kf->rk, subscription);
	//rd_kafka_resp_err_t err_p = rd_kafka_assign(kf->rk, subscription);

	rd_kafka_topic_partition_list_destroy(subscription);
	
	rd_kafka_poll_set_consumer(kf->rk);
	kf->conf = NULL;
	return (int) res;
}


LIBRARY_API int Consume(void* cons, char* topic,uint32_t *topiclen, char* payload,uint32_t *paylen,char* key, uint32_t *keylen,int32_t *partition,  char* errtxt, int *plen)
{
	kafka_struct* co = (kafka_struct*)cons;
	rd_kafka_message_t* rkmessage;
	size_t len = *plen;
	*errtxt = 0;

	if (NULL != co->msg)
	{
		rkmessage = co->msg;
		co->msg = NULL;
	}
	else
		rkmessage = rd_kafka_consumer_poll(co->rk, 500);


	if (rkmessage) {

		if ( 0 != rkmessage->err)
		{
			strncpy_s(errtxt, len, (char*)rkmessage->payload, rkmessage->len);
			*plen=(int)rkmessage->len;
			return rkmessage->err;
		}

		size_t tlen = strlen(rd_kafka_topic_name(rkmessage->rkt)); 
		if (*keylen < rkmessage->key_len || *paylen < rkmessage->len || *topiclen < tlen)
		{
			// not enough space to return save message for next call
			co->msg = rkmessage; 
			*topiclen = (uint32_t)tlen; 
			*paylen = (uint32_t)rkmessage->len;
			*keylen = (uint32_t)rkmessage->key_len;
			strncpy_s(errtxt, len, "message too long", 17);
			return 2;
		}

		memcpy(topic, rd_kafka_topic_name(rkmessage->rkt), tlen);
		memcpy(payload, (char*)rkmessage->payload, rkmessage->len);
		memcpy(key, (char*)rkmessage->key, rkmessage->key_len);
//		strncpy_s(topic, *topiclen, rd_kafka_topic_name(rkmessage->rkt), tlen);
//		strncpy_s(payload, *paylen, (char*)rkmessage->payload, rkmessage->len);
//		strncpy_s(key, *keylen,(char*) rkmessage->key, rkmessage->key_len);
		*partition = rkmessage->partition;

		*topiclen = (uint32_t)tlen;
		*paylen = (uint32_t)rkmessage->len;
		*keylen = (uint32_t)rkmessage->key_len;

		strncpy_s(errtxt, len, "", 1+strlen(""));
		rd_kafka_message_destroy(rkmessage);
	}
	else 
	{
		strncpy_s(errtxt, len, "no msg", 1+strlen("no msg"));
		return 1;
	}
	return 0;
}



LIBRARY_API int DeliveryReport(void* prod, unsigned long long* msgid, int* err, int* plength)
{
	kafka_struct* pr = (kafka_struct*)prod;
	
	// Copy the Producer object from APL
	rd_kafka_t* rk;
	rk = (rd_kafka_t*)pr->rk;

	// Retrive delivery callbacks for the producer
	delivery_reports* drs = (delivery_reports*)rd_kafka_opaque(rk);

	// Trigger the on_delivery function to produce the DR 
	rd_kafka_poll(rk, 500);
	
	// Number of delivery callbacks requested
	// be careful, it is possible that we have sent n msgs, but
	// poll has triggered only m<n dr, so we ask for 10 dr, we have sent 10
	// msg, we may recive 5 dr. We can asks for other 5 later, by calling again
	int req_drs = min(drs->counter, *plength);

	for (int i = 0; i < req_drs; i++)
	{
		msgid[i] = (unsigned long long) drs->drs[i]->msg_id;
		err[i] = (int)drs->drs[i]->err;

		// Free memory
		free(drs->drs[i]);
		drs->drs[i] = NULL;
	}

	// Check if the CB queue is empty
	if (*plength >= drs->counter) {
		*plength = drs->counter;
		drs->counter = 0;
	}
	// If not, move the queued messages at the beginning of the drs array and reset counters
	else {
		drs->counter = drs->counter - *plength;
		for (int i = 0; i < drs->counter; i++) {
			drs->drs[i] = drs->drs[i + *plength];
			drs->drs[i + *plength] = NULL;
		}
	}

	return 0;
}

LIBRARY_API int DRMessageError(int* err, char* errtxt, int *plen)
{
	size_t len = *plen;
	*errtxt = 0;
	rd_kafka_resp_err_t errorid = (rd_kafka_resp_err_t)*err;
	const char* DR_msgerror = rd_kafka_err2str(errorid);
	strncpy_s(errtxt, len, DR_msgerror, strlen(DR_msgerror));
	*plen =(int) strlen(DR_msgerror);
	return 0;
}


LIBRARY_API int NewTopicConf(void** topicconf)
{
	*topicconf = (void*)rd_kafka_topic_conf_new();
	return 0;
}


LIBRARY_API int DelTopicConf(void* topicconf)
{
	rd_kafka_topic_conf_destroy((rd_kafka_topic_conf_t*)topicconf);

	return 0;
}

LIBRARY_API int SetTopicConf(void* topicconf, char* key, char* val, char* errtxt, int *plen)
{
	rd_kafka_conf_res_t res;
	size_t len = *plen;
	*errtxt = 0;
	res= rd_kafka_topic_conf_set((rd_kafka_topic_conf_t*)topicconf, key, val, errtxt, len);
	if (res == RD_KAFKA_CONF_OK)
		*plen = 0;
	else
		*plen = (int)strlen(errtxt);

	return (int)res;
}

void Add(char* buffer,const char* str, int* poff, int  max)
{
	int len =(int) strlen(str);

	if (buffer != NULL && max >= len + *poff)
		memcpy(buffer + *poff, str, len);
	*poff += len;
}

LIBRARY_API int32_t Describe(char* buffer, int32_t* psize)
{
	int off = 0;
	int ret = 0;
	Add(buffer, "{", &off, *psize);
	Add(buffer, "\"Version\":\"0.1\",\"Patterns\":[", &off, *psize);
	Add(buffer, "\"I4 %P|Version >0T1 U4\",", &off, *psize);
	Add(buffer, "\"I4 %P|InitKafka >P\",", &off, *psize);
	Add(buffer, "\"I4 %P|UninitProducer P\",", &off, *psize);
	Add(buffer, "\"I4 %P|UninitConsumer P\",", &off, *psize);
	Add(buffer, "\"I4 %P|SetKafkaConf P <0T1 <0T1 >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|NewTopicPartitionList >P\",", &off, *psize);
	Add(buffer, "\"I4 %P|SetTopicPartitionList P <0T1\",", &off, *psize);
	Add(buffer, "\"I4 %P|SubscribeConsumerTPList P P >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|Consume P >0T1 =U4 >0T1 =U4 >0T1 =U4 >U4 >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|Produce P <0T1 <0T1 U4 <0T1 U4 I4 >U8 >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|DeliveryReport P >I8[] >I4[] =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|DRMessageError <I4 >0T1 =I4\"", &off, *psize);
	Add(buffer, "]", &off, *psize);
	Add(buffer, "}", &off, *psize);

	if (buffer != NULL && off < *psize)
		*(buffer + off) = 0;
	else
		ret = off + 1;
	*psize = off + 1;
	return ret;
}
