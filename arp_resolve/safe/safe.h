#ifndef _SAFE_H_
#define _SAFE_H_

#include <stdarg.h> /* For va_list */
#include <sys/types.h> /* For fork */
#include <unistd.h> /* For fork */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <dirent.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Safe version of malloc
 */
void *safe_malloc (int size);

/* @brief Safe version of strdup
 */
char *safe_strdup(const char *s);

/* @brief Safe version of asprintf
 */
int safe_asprintf(char **strp, const char *fmt, ...);

/* @brief Safe version of vasprintf
 */
int safe_vasprintf(char **strp, const char *fmt, va_list ap);

/* @brief Safe version of fork
 */
pid_t safe_fork(void);

int strlcpy (char *dest, const char *src, int len);

int strlcat (char *dest, const char *src, int len);

void wcx_sleep (long int s, long int us);

int find_pid_by_name (const char *ProcName, int* foundpid);

int slprintf (char *buf, int buflen, char *fmt, ...);

void wcx_sleep (long int s, long int us);

void add_to_request (char *request, size_t *request_size, size_t *request_len, char* str, size_t len);

void msg_to_http_server (const char *cmd, const char *msg);

void post_to_http_server (const char *cmd, const char *msg);

void report_plug_event (const char *msg);

char *http_request (const char *request);

#ifdef __cplusplus
}
#endif

#endif /* _SAFE_H_ */
