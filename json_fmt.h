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
//#include "pthread.h"
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void check_b64_payload(char *b64, int plen)
{
	size_t msglen = strlen(b64);
	unsigned char *msg = malloc(msglen);
//pthread_mutex_lock(&mutex);
	int z = base64_decode(b64, strlen(b64), (char *)msg, &msglen, 0);
//pthread_mutex_unlock(&mutex);
	if(msglen != plen) { fprintf(stderr, "msglen != plen!\n"); g_shutdown = 1; exit(1); }
	if(z == -1) { fprintf(stderr, "base64_decode() codec error!\n"); g_shutdown = 1; exit(1); }
	if(z == 0) { fprintf(stderr, "base64_decode() input error!\nstring: %s\n", b64); g_shutdown = 1; exit(1); }

	// Do something with the data
	check_for_jackpot(msg, msglen);

	free(msg);
}

static inline void dbg_validate_hex_decode(char *hex, bin_pkg_t *blob)
{
#ifdef DEBUG
	unsigned int i, n = 0;
	char *reincarnation = calloc(1, (blob->size*2)+1);
	for(i=0; i<blob->size; i++) {
		n += sprintf(reincarnation+n, "%02x", blob->data[i]);
	}
	if(strcmp(hex, reincarnation) != 0) { fprintf(stderr, "Reincarnation Validation Failed!\n"); g_shutdown = 1; exit(1); }
	free(reincarnation);
#endif
}

static void check_hex_payload(char *hex, int plen)
{
	// Decode our hex payload
	bin_pkg_t blob = hex2bin(hex);
	if(!blob.data || (blob.size < 1)) { fprintf(stderr, "hex2bin() failed decode!\n"); g_shutdown = 1; exit(1); }
	if(blob.size != plen) { fprintf(stderr, "blob.size != plen!\n"); g_shutdown = 1; exit(1); }

	// Validate the decode process
	dbg_validate_hex_decode(hex, &blob);

	// Do something with the data
	check_for_jackpot(blob.data, blob.size);

	// Clean up
	free(blob.data);
}

static inline void dbg_print_msg_dst(int dst)
{
/*#ifdef DEBUG
	printf("%d", dst);
#endif*/
}

static void handle_json(zmq_mf_t **mpa, void *user_data)
{
	zmq_mf_t *json_msg;

	json_msg = mpa[0];
	if(!json_msg) { return; }

	// Unwrap the JSON
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
		dbg_print_msg_dst(dst_obj->valueint);
		cJSON *plen_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadLen");
		cJSON *phex_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadHex");
		cJSON *pb64_obj = cJSON_GetObjectItemCaseSensitive(root, "PayloadB64");
		if(cJSON_IsNumber(plen_obj)) {
			if(cJSON_IsString(phex_obj)) {
				check_hex_payload(phex_obj->valuestring, plen_obj->valueint);
			}
			if(cJSON_IsString(pb64_obj)) {
				check_b64_payload(pb64_obj->valuestring, plen_obj->valueint);
			}
		}
	}
	cJSON_Delete(root);
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PUBCODE
static inline void add_b64_message(cJSON *root, unsigned char *data, unsigned int dlen)
{
	size_t outlen = (dlen*4)+1;
	char *b64 = calloc(1, outlen);
	base64_encode((char *)data, dlen, b64, &outlen, 0);
	(void)cJSON_AddStringToObject(root, "PayloadB64", b64);
	free(b64);
}

static inline void add_hex_message(cJSON *root, unsigned char *data, unsigned int dlen)
{
	// Convert Binary to Hex String
	unsigned int i, n = 0;
	char *hex = malloc((dlen*2)+1);
	for(i=0; i<dlen; i++) {
		n += sprintf(hex+n, "%02x", data[i]);
	}

	(void)cJSON_AddStringToObject(root, "PayloadHex", hex);
	free(hex);
}

static void publish_json(unsigned int dst, struct timespec *now, unsigned char *data, unsigned int dlen)
{
	char zbuf[128];

	// Create JSON object and insert DST address
	cJSON *root = cJSON_CreateObject();
	(void)cJSON_AddNumberToObject(root, "Destination", dst);

	// Set the Timestamp
	snprintf(zbuf, sizeof(zbuf), "%ld.%09ld", now->tv_sec, now->tv_nsec);
	(void)cJSON_AddStringToObject(root, "Timestamp", zbuf);

	(void)cJSON_AddNumberToObject(root, "PayloadLen", dlen);
	if(g_base64) {
		add_b64_message(root, data, dlen);
	} else {
		add_hex_message(root, data, dlen);
	}

	// Publish the JSON message
	char *minjson = cJSON_Print(root);
	cJSON_Minify(minjson);
	as_zmq_pub_send(g_pktpub, minjson, strlen(minjson)+1, 0);

	// Clean up
	free(minjson);
	cJSON_Delete(root);
}
#endif
