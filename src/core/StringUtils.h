/*
 * StringUtils.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_
#include <string.h>
#include <stdarg.h>

#define OC_ESCAPE_URI            0
#define OC_ESCAPE_ARGS           1
#define OC_ESCAPE_URI_COMPONENT  2
#define OC_ESCAPE_HTML           3
#define OC_ESCAPE_REFRESH        4
#define OC_ESCAPE_MEMCACHED      5
#define OC_ESCAPE_MAIL_AUTH      6

#define OC_UNESCAPE_URI       1
#define OC_UNESCAPE_REDIRECT  2

/*
 *
 */

#define oc_strcmp(s1, s2)  strcmp(s1,s2)

//
inline u_char oc_tolower(u_char c ){

	return (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c);
};

inline u_char oc_toupper(u_char c){

	 return (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c);
};

inline int32_t oc_strncmp(u_char* s1, u_char* s2, size_t n){

	return (int32_t)::strncmp((const char *) s1, (const char *) s2, n);
};

inline  int32_t oc_strcmp(u_char* s1, u_char* s2){

	return ::strcmp((const char *) s1, (const char *) s2);
};

inline int32_t oc_strstr(u_char* s1, u_char*  s2){

	return ::strstr((const char *) s1, (const char *) s2);
};

inline size_t oc_strlen(u_char* s) {

	return ::strlen((const char *) s);
};

inline u_char* oc_strchr(u_char*s1, u_char c){

	return (u_char*)::strchr((const char *) s1, (int) c);
};

inline u_char* oc_strrchr(u_char*s1, u_char c){

	return (u_char*)::strrchr((const char *) s1, (int) c);
};

inline u_char*  oc_strlchr(u_char *p, u_char *last, u_char c){
	while (p < last) {

		if (*p == c) {
			return p;
		}

		p++;
	}
	return NULL;
};

inline void oc_memzero(u_char* buf, size_t  n) {

	(void) ::memset(buf, 0, n);
};

inline void oc_memset(u_char* buf, u_char c, size_t n) {

	(void) ::memset(buf, c, n);
};

inline void oc_memcpy(u_char* dst,  u_char* src, size_t n) {

	(void) ::memcpy(dst, src, n);
};

inline u_char* oc_cpymem(u_char* dst,  u_char* src, size_t  n){

	return (((u_char *) memcpy(dst, src, n)) + (n));
}

//translate string into lowercase strings
void 	oc_strlow(u_char *dst, u_char *src, size_t n);

/**
 * 从src字符串中连续复制n字节到dst, 且返回dst的首地址
 *
 */
u_char*   oc_cpystrn(u_char *dst, u_char *src, size_t n);

/**
 *
 */
u_char *  sprintf(u_char *buf, const char *fmt, ...);

u_char *  oc_slprintf(u_char *buf, u_char *last, const char *fmt, ...);

u_char *  snprintf(u_char *buf, size_t max, const char *fmt, ...);

u_char *  vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);

u_char *   sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero, uint hexadecimal, uint width);

int32_t	 oc_strcasecmp(u_char *s1, u_char *s2);

int32_t  oc_strncasecmp(u_char *s1, u_char *s2, size_t size);

u_char *  strnstr(u_char *s1, char *s2, size_t len);

u_char *  strstrn(u_char *s1, char *s2, size_t n);

	u_char *  strcasestrn(u_char *s1, char *s2, size_t n);
	u_char *  strlcasestrn(u_char *s1, u_char *last, u_char *s2, size_t n);

	int32_t    rstrncmp(u_char *s1, u_char *s2, size_t n);
	int32_t    rstrncasecmp(u_char *s1, u_char *s2, size_t n);

	int32_t    memn2cmp(u_char *s1, u_char *s2, size_t n1, size_t n2);
	int32_t    atoi(u_char *line, size_t n);
	int32_t	   dns_strcmp(u_char *s1, u_char *s2);
	int32_t	   atofp(u_char *line, size_t n, size_t point);

	ssize_t	   atosz(u_char *line, size_t n);
	off_t 	   atoof(u_char *line, size_t n);
	time_t	   atotm(u_char *line, size_t n);

	int32_t	   hextoi(u_char *line, size_t n);
	u_char *  hex_dump(u_char *dst, u_char *src, size_t len);

	void 	encode_base64(oc_str_t *dst, oc_str_t *src);
	int32_t  decode_base64(oc_str_t *dst, oc_str_t *src);

	int32_t   decode_base64url(oc_str_t *dst, oc_str_t *src);

	int32_t  utf8_decode(u_char **p, size_t n);
	size_t    utf8_length(u_char *p, size_t n);
	u_char *  utf8_cpystrn(u_char *dst, u_char *src, size_t n, size_t len);
	uintptr_t  escape_uri(u_char *dst, u_char *src, size_t size, uint32_t type);
	void	   unescape_uri(u_char **dst, u_char **src, size_t size, uint32_t type);
	uintptr_t  escape_html(u_char *dst, u_char *src, size_t size);


#endif /* STRINGUTILS_H_ */
