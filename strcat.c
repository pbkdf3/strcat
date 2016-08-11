/**
  Copyright (C) 2016 ADP, LLC

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

/* strcat - poorly named utility to treat mapr streams as files
 * by daniel reznick
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <streams/streams.h>
#include <sys/time.h>

/* dumb but seems to work */
#define CHECK(r, msg)                           \
    if (r != EXIT_SUCCESS) {                    \
        fprintf(stderr, "%s - %d\n", msg,r);    \
        return(r);                              \
    }

/* does not return, for callbacks that are typed void */
#define CHECK_VOID(r, msg)                      \
    if (r != EXIT_SUCCESS) {                    \
        fprintf(stderr, "%s - %d\n", msg,r);    \
        return;                                 \
    }                                           


#if 0
#define DEBUG(...) fprintf(stderr, __VA_ARGS__);
#else
#define DEBUG(...) (void)0;
#endif

long opt_poll_timeout = 1000;
long opt_sleep = 0;
int opt_exit_on_timeout = 1;
char *opt_group_id=NULL;


/* maybe will come in handy */
void 
noop_rebalance_cb(streams_topic_partition_t *topic_partitions,
                  uint32_t topic_partitions_size,
                  void *cb_ctx) { return; }


int
consumer_init(const char *streamtopic, const char *gid, 
              streams_config_t *conf, streams_consumer_t *consumer)
{
    CHECK (streams_config_create(conf),
           "streams_config_create() failed");

    streams_config_set(*conf, "auto.offset.reset", "earliest");
    streams_config_set(*conf, "metadata.max.age.ms", "60000"); //1 minute
    if (gid != NULL)
        streams_config_set(*conf, "group.id", gid);

    CHECK (streams_consumer_create(*conf, consumer),
           "streams_consumer_create() failed");

    CHECK (streams_consumer_subscribe_regex(*consumer,  streamtopic, NULL, NULL, NULL),
           "streams_consumer_subscribe_regex() faild");
    
    return(EXIT_SUCCESS);
}

int
consumer(const char *streamtopic)
{
    int ret_val;
    streams_config_t config;
    streams_consumer_t consumer;

    CHECK (consumer_init(streamtopic, opt_group_id, &config, &consumer),
           "consumer_init() failed");

    sleep(opt_sleep);

    while (1) {
        streams_consumer_record_t *records;
        uint32_t nrecords;

        CHECK (streams_consumer_poll(consumer, opt_poll_timeout, &records, &nrecords),
               "streams_consumer_poll() failed");
        DEBUG("%d records retrieved\n", nrecords);

        if (nrecords==0 && opt_exit_on_timeout==1) {
            break;
        }

        for (int rec = 0; rec < nrecords; rec++) {
            uint32_t nmsg;
            CHECK (streams_consumer_record_get_message_count(records[rec], &nmsg),
                   "streams_consumer_record_get_message_count() failed");
            DEBUG("%d msgs in record\n", nmsg);

            uint32_t nval;
            void *val;

            /* no support for keys currently, currently in development */
            for (uint32_t i = 0; i < nmsg; ++i) {
                CHECK (streams_msg_get_value(records[rec], i, &val, &nval),
                       "streams_msg_get_value() failed");
                DEBUG("%d retrieved\n", i);
                /* output half of "cat" */
                write(1, val, nval);
                write(1, "\n", 1); // could become optional
            }
            streams_consumer_record_destroy(records[rec]);
        }
        CHECK (streams_consumer_commit_all_sync(consumer),
               "streams_consumer_commit_all_sync() failed\n");
    }
    return (EXIT_SUCCESS);
}

void
producer_cb(int32_t err,
            streams_producer_record_t record,
            int partitionid, int64_t offset,
            void *ctx)
{
    char *val_ptr;
    uint32_t vs;
    int ret_val;
  
        
    CHECK_VOID (streams_producer_record_get_value(record,
                                                  (const void **) &val_ptr,
                                                  &vs),
                "streams_producer_record_get_value() failed");

    free(val_ptr);

    CHECK_VOID (streams_producer_record_destroy(record),
                "streams_producer_record_destroy() failed");
}

int
producer_init(const char *streamtopic,
              streams_topic_partition_t *t,
              streams_config_t *conf,
              streams_producer_t *prod)
{
    CHECK (streams_topic_partition_create(streamtopic, INVALID_PARTITION_ID, t),
           "streams_topic_partition_create() failed");

    CHECK (streams_config_create(conf),
           "streams_config_create() failed");

    streams_config_set(*conf, "buffer.memory", "33554432");
    streams_config_set(*conf, "streams.buffer.max.time.ms", "1000");
    
    CHECK (streams_producer_create(*conf, prod),
           "streams_producer_create() failed");

    return(EXIT_SUCCESS);
}

int
producer(const char *streamtopic)
{
    char *line = NULL;
    char *buf = NULL;
    size_t len = 0;
    ssize_t read;
    streams_topic_partition_t topic;
    streams_config_t config;
    streams_producer_t producer;
    
    CHECK (producer_init(streamtopic, &topic, &config, &producer),
           "producer_init() failed");

    /* input half of cat */
    while ((read = getline(&line, &len, stdin)) != -1) {
        DEBUG("getline [%zu] %s\n", read, line);
        buf = (char *)malloc(read);
        memcpy(buf, line, read);

        /* could be smarter, attempt to keep EOL out of stream */
        if (buf[read-1] == '\n') {
            read--;
        }

        /* Create a record that contains the message. */
        streams_producer_record_t record;
        CHECK (streams_producer_record_create(topic, NULL, 0, buf, read, &record),
               "streams_producer_record_create() failed"); 
        CHECK (streams_producer_send(producer, record, producer_cb, NULL),
               "streams_producer_send() failed");
    }

    /* final flush */
    CHECK (streams_producer_flush(producer),
           "streams_producer_flush() failed\n");
    if (line)
        free(line);

    return(EXIT_SUCCESS);
}

void display_usage(char *name)
{
    fprintf(stderr,
            "usage: %s [-xpc] [-g gid] [-w seconds] /stream:regex\n"
            " -x: do not exit on timeout, stream forever\n"
            " -p: produce, stdin -> /stream:topic\n"
            " -c: consume, /stream:regex -> stdout\n"
            " -w seconds: seconds to wait after subscription (for rebalancing)\n"
            " -g gid: consumer group id\n", name);
    exit(-1);
}


int
main(int argc, char *argv[])
{
    char *streamtopic;
    int consume = 0;
    int produce = 0;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "pcxg:w:")) != -1)
        switch (c) {
        case 'x':
            opt_exit_on_timeout = 0;
            break;
        case 'p':
            produce = 1;
            break;
        case 'c':
            consume = 1;
            break;
        case 'g':
            opt_group_id = optarg;
            break;
        case 'w':
            opt_sleep = atoi(optarg);
            break;
        default:
            abort();
        }
    
    if (argc-optind < 1) { //need one real arg
        display_usage(argv[0]);
    }

    /* i-o mode autodetection based on tty presence */
    if (produce == 0 && consume == 0) {
        int ttyin = isatty(fileno(stdin));
        int ttyout = isatty(fileno(stdout));
        if (ttyin==0 && ttyout==0) {
            fprintf(stderr, "%s: stdin or stdout can be redirected but not both\n", argv[0]);
            exit(1);
        }

        if (ttyin==1) {
            consume=1;
        } else if (ttyin==0 && ttyout==1) {
            produce=1;
        }
    }
    
    streamtopic = argv[optind];
    
    if (produce == 1) {
        CHECK (producer(streamtopic),
               "producer failed!");
    }
  
    if (consume == 1) {
        CHECK (consumer(streamtopic),
               "consumer failed!");
    }
    exit(0);
}
