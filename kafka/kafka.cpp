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
	rd_kafka_topic_partition_list_t* tplist;
	rd_kafka_resp_err_t err;
} commit_report;

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

static void on_commit(rd_kafka_t* rk, rd_kafka_resp_err_t err,	rd_kafka_topic_partition_list_t* topic_partition_list, void* opaque) {

	//commit_report* cr = (commit_report*)rd_kafka_opaque(rk);

	commit_report* cr = (commit_report*)opaque;
	cr->tplist = rd_kafka_topic_partition_list_copy(topic_partition_list);
	cr->err = err;
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


LIBRARY_API int SetTopicPartitionList(void* subscr, char* topic, int32_t partition)
{

	rd_kafka_topic_partition_list_t* subscription = (rd_kafka_topic_partition_list_t*)subscr;
	rd_kafka_topic_partition_list_add(subscription, (const char*)topic, partition);

	return 0;
}

LIBRARY_API int SetOffset(void* subscr, char* topic, int32_t partition, int64_t offset)
{
	rd_kafka_resp_err_t res;
	rd_kafka_topic_partition_list_t* subscription = (rd_kafka_topic_partition_list_t*)subscr;
	res = rd_kafka_topic_partition_list_set_offset(subscription, (const char*) topic, partition, offset);

	return (int)res;
}

LIBRARY_API int SubscribeConsumerTPList(void* kafka, void* subscr, char* errtxt, int *plen)
{
	rd_kafka_resp_err_t res;
	size_t len = *plen;
	*errtxt = 0;
	kafka_struct* kf = (kafka_struct*)kafka;
	rd_kafka_topic_partition_list_t* subscription = (rd_kafka_topic_partition_list_t*)subscr;
	
	// Set on_commit fn as callback for commit
	// rd_kafka_conf_set_offset_commit_cb(kf->conf, on_commit);
	// Allocate a commit_report for cb
	commit_report* cr = (commit_report*)calloc(1, sizeof(commit_report));
	// Create enough space by allocating subscription->cnt number of topic/partition
	cr->tplist = rd_kafka_topic_partition_list_new(subscription->cnt);
	// Set cr as opaque for commit callback
	rd_kafka_conf_set_opaque(kf->conf, (void*)cr);

	// Init consumer on its config conf
	rd_kafka_t* rk = rd_kafka_new(RD_KAFKA_CONSUMER, kf->conf, errtxt, len);
	kf->rk = rk;
	if (NULL!=rk)
		*plen = 0;
	else
		*plen = (int)strlen(errtxt);
	
	// Subscribe consumer to topic partition list
	res = rd_kafka_subscribe(kf->rk, subscription); // Only the "topic" field is used here

	// Set the poll call
	rd_kafka_poll_set_consumer(kf->rk);

	// Cleaning
	rd_kafka_topic_partition_list_destroy(subscription);
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

LIBRARY_API int Commit(void* cons, void* subscr, int32_t async)
{
	kafka_struct* co = (kafka_struct*)cons;
	rd_kafka_t* rk = (rd_kafka_t*)co->rk;
	rd_kafka_topic_partition_list_t* offsets = (rd_kafka_topic_partition_list_t*)subscr;
	
	rd_kafka_resp_err_t res;


	if (async) {
		// Get the commit_report as opaque and pass it to the on_commit callback
		commit_report* cr = (commit_report*)rd_kafka_opaque(rk);
		// rkqu is NULL - see doc for reference
		rd_kafka_commit_queue(rk, offsets, NULL, on_commit, cr);
		// Async, return no error while committing, get back the error only when asking for report
		res = = RD_KAFKA_RESP_ERR_NO_ERROR;
	}
	else 		
		res = rd_kafka_commit(rk, offsets, async);

	return (int) res;
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


LIBRARY_API int CommitReport(void* cons, char* topics, uint32_t* topiclen, int32_t* partitions, int64_t* offsets, int* err, int* listlen)
{
	// Copy the Consumer object from APL
	kafka_struct* co = (kafka_struct*)cons;
	rd_kafka_t* rk;
	rk = (rd_kafka_t*)co->rk;

	// Retrive commit callback for the consumer
	commit_report* cr = (commit_report*)rd_kafka_opaque(rk);

	// Need a way to check that the cr has something in it
	// if I call commit report and the list is not populated, it will complain
	// What should I add??
	if ( !(int)cr->err) {
		
		// Topic partition offset: this will point to each rd_kafka_topic_partition_t in cr->tplist
		rd_kafka_topic_partition_t* tpo;

		// Pointer to topics names
		char** topic = (char**)calloc(cr->tplist->cnt, sizeof(char*));

		// Need this to create enough space to return the topics list
		int totaltopiclen = 0;
		// Loop over all the topics/partitions
		for (int i = 0; i < cr->tplist->cnt; i++)
		{
			// Retrive each rd_kafka_topic_partition_t 
			tpo = &cr->tplist->elems[i];

			// Save each topic, partition and offset, and the error in the cb
			topic[i] = tpo->topic;
			partitions[i] = tpo->partition;
			offsets[i] = tpo->offset;
			err[i] = tpo->err;

			// Create also space for the topic names
			totaltopiclen = strlen(topic[i]) + totaltopiclen;
		}

		// Topic length considering spaces and null
		*topiclen = (uint32_t)(totaltopiclen + (int)cr->tplist->cnt);
		
		// Copy topic names to APL
		char* dest = topics;
		for (int i = 0; i < cr->tplist->cnt; i++) {
			size_t len = strlen(topic[i]);
			memcpy(dest, topic[i], len);
			dest += len;
			// Add blanks and null
			if (i < cr->tplist->cnt - 1) {
				*dest = ' ';
				dest++;
			}
			else
				*dest = '\0';
		}

		// Copy to APL the number of rd_kafka_topic_partition_t in cr->tplist
		*listlen = (int)cr->tplist->cnt;

		// Clean up
		 free(topic);
		// Should I free tplist?
		rd_kafka_topic_partition_list_destroy(cr->tplist);
	}
	else
	{
		//errormsg
		return (int) cr->err;
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
	Add(buffer, "\"I4 %P|SetTopicPartitionList P <0T1 I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|SetOffset P <0T1 I4 I8\",", &off, *psize);
	Add(buffer, "\"I4 %P|SubscribeConsumerTPList P P >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|Consume P >0T1 =U4 >0T1 =U4 >0T1 =U4 >U4 >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|Commit P P I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|Produce P <0T1 <0T1 U4 <0T1 U4 I4 >U8 >0T1 =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|DeliveryReport P >I8[] >I4[] =I4\",", &off, *psize);
	Add(buffer, "\"I4 %P|CommitReport P >0T1 =U4 >I4[] >I8[] >I4[] =I4\",", &off, *psize);
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
