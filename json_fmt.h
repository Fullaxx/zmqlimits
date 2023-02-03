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
static void check_payload(char *hex, int plen)
{
	bin_pkg_t blob = hex2bin(hex);
	if(!blob.data || (blob.size < 1)) { fprintf(stderr, "hex2bin() failed decode!\n"); g_shutdown = 1; exit(1); }

#ifdef DEBUG
	int n = 0;
	char reincarnation[4096];
	memset(reincarnation, 0, sizeof(reincarnation));
	for(int i=0; i<blob.size; i++) {
		n += sprintf(&reincarnation[n], "%02x", blob.data[i]);
	}
	if(strcmp(hex, reincarnation) != 0) { fprintf(stderr, "Reincarnation Validation Failed!\n"); g_shutdown = 1; exit(1); }
#endif

	// Do something with the data
	check_for_jackpot(blob.data, blob.size);

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
	if(cJSON_IsNumber(dst_obj)) {
		// Check if the message is for me
		if(dst_obj->valueint == myaddr) { decode_this_pkg = 1; }
	}
	if(decode_this_pkg) {
		// Only decode the payload if the message is for me
		cJSON *plen_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadLen");
		cJSON *phex_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadHex");
		if(cJSON_IsString(phex_obj) && cJSON_IsNumber(plen_obj)) {
			check_payload(phex_obj->valuestring, plen_obj->valueint);
		}
	}
	cJSON_Delete(root);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PUBCODE
static void publish_json(unsigned int dst, struct timespec *now)
{
	int i, n;
	cJSON *root;
	char *hex;
	char *minjson;
	unsigned int r;
	char zbuf[1500];

	root = cJSON_CreateObject();
	(void)cJSON_AddNumberToObject(root, "Destination", dst);

	// Set the Timestamp
	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", now->tv_sec, now->tv_nsec);
	(void)cJSON_AddStringToObject(root, "Timestamp", zbuf);

	// Mock Binary Data
	for(i=0; i<1024; i+=4) {
		r = rand();
		memcpy(&zbuf[i], &r, 4);
	}

	n = 0;
	hex = malloc((1500*2)+1);
	for(i=0; i<1500; i++) {
		n+=sprintf(hex+n, "%02x", (unsigned char)zbuf[i]);
	}
	(void)cJSON_AddNumberToObject(root, "PayloadLen", 1500);
	(void)cJSON_AddStringToObject(root, "PayloadHex", hex);
	free(hex);

	minjson = cJSON_Print(root);
	cJSON_Minify(minjson);
	as_zmq_pub_send(g_pktpub, minjson, strlen(minjson)+1, 0);
	g_zmqmsgs++;
	cJSON_Delete(root);
	free(minjson);
}
#endif
