/*
	Copyright (C) 2023 Brett Kuskie <fullaxx@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "cJSON.h"
#include "getopts.h"
#include "async_zmq_sub.h"
#include "hex2bin.h"

// Prototypes
static void parse_args(int argc, char *argv[]);

// Globals
unsigned int g_shutdown = 0;
char *g_zmqsockaddr = NULL;
int g_maxdst = 1;
int g_mpm = 0;
int g_json = 0;

// Statistics
unsigned long g_zmqmsgs = 0;

static void alarm_handler(int signum)
{
	printf("%lu\n", g_zmqmsgs);
	g_zmqmsgs = 0;
	(void) alarm(1);
}

static void sig_handler(int signum)
{
	switch(signum) {
		/*case SIGPIPE:
			fprintf(stderr, "SIGPIPE recvd!\n");
			g_shutdown = 1;*/
		case SIGHUP:
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			g_shutdown = 1;
			break;
	}
}

/*
	snprintf(zbuf, sizeof(zbuf), "%u", dst);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", ts->tv_sec, ts->tv_usec);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	as_zmq_pub_send(g_pktpub, buf, len, 0);
*/
static void handle_mpm(zmq_mf_t **mpa)
{
	zmq_mf_t *dst_msg;
	zmq_mf_t *ts_msg;
	zmq_mf_t *binary_msg;
	int i;

	dst_msg = mpa[0];
	ts_msg = mpa[1];
	binary_msg = mpa[2];

	if(!dst_msg) { return; }
	if(!ts_msg) { return; }
	if(!binary_msg) { return; }

	if(binary_msg->size != 1500) { fprintf(stderr, "INVALID SIZE!\n"); exit(1); }

	// Do something with the data
	int sum = 0;
	unsigned char *data = (unsigned char *)binary_msg->buf;
	for(i=0; i<binary_msg->size; i++) {
		sum += data[i];
	}
}

static void check_payload(char *hex, int plen)
{
	int i;
	bin_pkg_t blob;

	blob = hex2bin(hex);
	if(!blob.data || (blob.size < 1)) { fprintf(stderr, "hex2bin() failed decode!\n"); exit(1); }

#ifdef DEBUG
	int n = 0;
	char reincarnation[3000];
	memset(reincarnation, 0, sizeof(reincarnation));
	for(i=0; i<blob.size; i++) {
		n += sprintf(&reincarnation[n], "%02x", blob.data[i]);
	}
	if(strcmp(hex, reincarnation) != 0) { fprintf(stderr, "DIFFERENT!\n"); exit(1); }
#endif

	// Do something with the data
	int sum = 0;
	for(i=0; i<blob.size; i++) {
		sum += blob.data[i];
	}

	free(blob.data);
}

static void handle_json(zmq_mf_t **mpa, void *user_data)
{
	zmq_mf_t *json_msg;

	json_msg = mpa[0];
	if(!json_msg) { return; }

	//printf("%s\n", (char *)json_msg->buf);
	int decode_this_pkg = 0;
	int *ident = (int *)user_data;
	int myaddr = *ident;
	cJSON *root = cJSON_Parse(json_msg->buf);
	cJSON *dst_obj = cJSON_GetObjectItemCaseSensitive(root, "Destination");
	cJSON *plen_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadLen");
	cJSON *phex_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadHex");
	if(cJSON_IsNumber(dst_obj)) {
		// Check if the message is for me
		if(plen_obj->valueint == myaddr) { decode_this_pkg = 1; }
	}
	if(cJSON_IsString(phex_obj) && cJSON_IsNumber(plen_obj)) {
		if(decode_this_pkg) {
			// Only decode the payload if the message is for me
			check_payload(phex_obj->valuestring, plen_obj->valueint);
		}
	}
	cJSON_Delete(root);
}

static void sub_cb(zmq_sub_t *s, zmq_mf_t **mpa, int msgcnt, void *user_data)
{
	if(!mpa) { return; }
	if(msgcnt == 1) { handle_json(mpa, user_data); }
	else if(msgcnt == 3) { handle_mpm(mpa); }
	else { return; }

	g_zmqmsgs++;
	if(g_shutdown) { as_zmq_sub_destroy(s); }
}

int main(int argc, char *argv[])
{
	int i;
	char filter[16];
	int int_array[64];
	zmq_sub_t *sub;

	parse_args(argc, argv);

	for(i=1; i<=g_maxdst; i++) {
		filter[0] = 0;
		int_array[i-1] = i;
		if(g_mpm) { snprintf(filter, sizeof(filter), "%d", i); }
		printf("Starting %2d ...\n", i);
		sub = as_zmq_sub_create(g_zmqsockaddr, filter, sub_cb, 0, &int_array[i-1]);
		if(!sub) {
			fprintf(stderr, "as_zmq_sub_create(%s) failed!\n", g_zmqsockaddr);
			exit(EXIT_FAILURE);
		}
	}

	signal(SIGHUP,  sig_handler);
	signal(SIGINT,  sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGALRM, alarm_handler);
	(void) alarm(1);

	// wait for the slow release of death
	while(!g_shutdown) {
		usleep(100);
	}

	// Shutdown the ZMQ SUB bus
	//as_zmq_sub_destroy(sub);

	usleep(200000);
	if(g_zmqsockaddr) { free(g_zmqsockaddr); }
	return 0;
}

struct options opts[] = {
	{ 1, "ZMQ",		"Set the ZMQ SUB",			"Z",  1 },
	{ 2, "DST",		"Set Max Destinations",		"d",  1 },
	{ 3, "MPM",		"Use multipart messages",	"m",  0 },
	{ 4, "JSON",	"Use JSON messages",		"j",  0 },
	{ 0, NULL,		NULL,						NULL, 0 }
};

static void parse_args(int argc, char *argv[])
{
	int c;
	char *args;

	while ((c = getopts(argc, argv, opts, &args)) != 0) {
		switch(c) {
			case -2:
				// Special Case: Recognize options that we didn't set above.
				fprintf(stderr, "Unknown Getopts Option: %s\n", args);
				exit(EXIT_FAILURE);
				break;
			case -1:
				// Special Case: getopts() can't allocate memory.
				fprintf(stderr, "Unable to allocate memory for getopts().\n");
				exit(EXIT_FAILURE);
				break;
			case 1:
				g_zmqsockaddr = strdup(args);
				break;
			case 2:
				g_maxdst = atoi(args);
				break;
			case 3:
				g_mpm = 1;
				break;
			case 4:
				g_json = 1;
				break;
			default:
				fprintf(stderr, "Unknown command line argument %i\n", c);
		}
		free(args);
	}

	if(!g_zmqsockaddr) {
		fprintf(stderr, "I need a ZMQ bus to listen to! (Fix with -Z)\n");
		exit(EXIT_FAILURE);
	}

	if(g_mpm + g_json != 1) {
		fprintf(stderr, "I need a mode! (Fix with -m/-j)\n");
		exit(EXIT_FAILURE);
	}

	if((g_maxdst < 1) || (g_maxdst > 64)) {
		fprintf(stderr, "0 < MAXDST <= 64! (Fix with -d)\n");
		exit(EXIT_FAILURE);
	}
}
