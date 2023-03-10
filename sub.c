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
#include "libbase64.h"
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

// We need to do something with the data after the payload has been decode/recv'd
// Lets just add up the bits and see if we hit the jackpot ...
static void check_for_jackpot(unsigned char *data, unsigned long size)
{
	unsigned long sum = 0;
	for(int i=0; i<size; i++) {
		sum += data[i];
	}
	if(sum % 192000 == 0) { printf("JACKPOT!\n"); }
}

#include "mpm_fmt.h"
#include "json_fmt.h"

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
		printf("Starting SUB %2d ...\n", i);
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
