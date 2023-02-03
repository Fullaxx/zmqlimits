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
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "cJSON.h"
#include "getopts.h"
#include "async_zmq_pub.h"

// Prototypes
static void parse_args(int argc, char **argv);

// Globals
int g_shutdown = 0;

char *g_zmqsockaddr = NULL;
zmq_pub_t *g_pktpub = NULL;
long g_pps = 10000;
int g_mpm = 0;
int g_json = 0;
int g_maxdst = 1;

unsigned long g_zmqmsgs = 0;
static void alarm_handler(int signum)
{
	unsigned long l_zmqmsgs = g_zmqmsgs;
	g_zmqmsgs = 0;
	printf("%lu\n", l_zmqmsgs);
	(void) alarm(1);
}

static void sig_handler(int signum)
{
	switch(signum) {
		case SIGHUP:
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			g_shutdown = 1;
			break;
	}
}

#include "mpm_fmt.h"
#include "json_fmt.h"

static void publisher_loop(void)
{
	long this_sec;
	unsigned int r;
	unsigned int dst;
	struct timespec now;
	unsigned int msgsthissec;

	clock_gettime(CLOCK_MONOTONIC, &now);
	this_sec = now.tv_sec;
	msgsthissec = 0;
	while(!g_shutdown) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		if(msgsthissec < g_pps) {
			r = rand();
			dst = (r % g_maxdst) + 1;
			if(g_mpm) { publish_mpm(dst, &now); }
			if(g_json) { publish_json(dst, &now); }
			g_zmqmsgs++;
			msgsthissec++;
		} else {
			// Wait for the second hand to rollover
			if(now.tv_sec == this_sec) {
				usleep(5);
			} else {
				msgsthissec = 0;
				this_sec = now.tv_sec;
			}
		}
	}
}

int main(int argc, char *argv[])
{

	srand(time(NULL));
	parse_args(argc, argv);

	g_pktpub = as_zmq_pub_create(g_zmqsockaddr, 0, 0);
	if(!g_pktpub) {
		fprintf(stderr, "as_zmq_pub_create(%s) failed!\n", g_zmqsockaddr);
		exit(EXIT_FAILURE);
	}

	signal(SIGINT,  sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGHUP,  sig_handler);
	signal(SIGALRM, alarm_handler);
	(void) alarm(1);

	publisher_loop();

	// Shutdown the ZMQ PUB bus
	if(g_pktpub) {
		as_zmq_pub_destroy(g_pktpub);
		g_pktpub = NULL;
	}

	//fclose(f);
	if(g_zmqsockaddr) { free(g_zmqsockaddr); }

	return 0;
}

struct options opts[] = 
{
	{ 1, "ZMQ",		"Set the ZMQ PUB",			"Z",  1 },
	{ 2, "DST",		"Set Max Destinations",		"d",  1 },
	{ 3, "MPM",		"Use multipart messages",	"m",  0 },
	{ 4, "JSON",	"Use JSON messages",		"j",  0 },
	{ 9, "PPS",		"Set the target PPS",		"r",  1 },
	{ 0, NULL,		NULL,						NULL, 0 }
};

static void parse_args(int argc, char **argv)
{
	char *args;
	int c;

	while ((c = getopts(argc, argv, opts, &args)) != 0) {
		switch(c) {
			case -2:
				// Special Case: Recognize options that we didn't set above.
				fprintf(stderr, "Unknown Getopts Option: %s\n", args);
				break;
			case -1:
				// Special Case: getopts() can't allocate memory.
				fprintf(stderr, "Unable to allocate memory for getopts()\n");
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
			case 9:
				g_pps = atol(args);
				break;
			default:
				fprintf(stderr, "Unexpected getopts Error! (%d)\n", c);
				break;
		}

		//This free() is required since getopts() automagically allocates space for "args" everytime it's called.
		free(args);
	}

	if(!g_zmqsockaddr) {
		fprintf(stderr, "I need a ZMQ bus to drop packets onto! (Fix with -Z)\n");
		exit(EXIT_FAILURE);
	}

	if(g_pps < 1) {
		fprintf(stderr, "PPS must be > 0! (Fix with -r)\n");
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
