#ifndef __HEX2BIN_H__
#define __HEX2BIN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bin_pkg {
	unsigned char *data;
	ssize_t size;
} bin_pkg_t;

bin_pkg_t hex2bin(char *hex);

#ifdef __cplusplus
}
#endif

#endif /* __HEX2BIN_H__ */
