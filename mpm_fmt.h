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
static void publish_mpm(unsigned int dst, struct timespec *now)
{
	char zbuf[1600];

	// Set the dst address
	snprintf(zbuf, sizeof(zbuf), "%u", dst);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	// Set the Timestamp
	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", now->tv_sec, now->tv_nsec);
	as_zmq_pub_send(g_pktpub, zbuf, strlen(zbuf)+1, 1);

	// Mock Binary Data
	unsigned int b = fill_payload((unsigned char *)&zbuf[0], sizeof(zbuf), 1024, 512);
	as_zmq_pub_send(g_pktpub, zbuf, b, 0);
}
#endif
