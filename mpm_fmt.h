

#ifdef SUBCODE
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

	dst_msg = mpa[0];
	ts_msg = mpa[1];
	binary_msg = mpa[2];

	if(!dst_msg) { return; }
	if(!ts_msg) { return; }
	if(!binary_msg) { return; }

	if(binary_msg->size != 1500) { fprintf(stderr, "INVALID SIZE!\n"); g_shutdown = 1; exit(1); }

	// Do something with the data
	check_for_jackpot(binary_msg->buf, binary_msg->size);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PUBCODE
/*
	snprintf(zbuf, sizeof(zbuf), "%u", dst);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", ts->tv_sec, ts->tv_usec);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	as_zmq_pub_send(g_pktpub, buf, len, 0);
*/
static void publish_mpm(struct timespec *now)
{
	size_t z;
	char zbuf[1500];
	unsigned int dst;
	unsigned int temp;

	temp = rand();
	dst = (temp % g_maxdst) + 1;

	// Set the dst address
	snprintf(zbuf, sizeof(zbuf), "%u", dst);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	// Set the Timestamp
	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", now->tv_sec, now->tv_nsec);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	// Mock Binary Data
	for(z=0; z<1024; z+=4) {
		temp = rand();
		memcpy(&zbuf[z], &temp, 4);
	}
	as_zmq_pub_send(g_pktpub, zbuf, sizeof(zbuf), 0);
	g_zmqmsgs++;
}
#endif
