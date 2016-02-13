#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>
#include "safe.h"

#define OUTCHAR(c)  (buflen > 0? (--buflen, *buf++ = (c)): 0)

void *safe_malloc (int size) {
    void * retval = NULL;
    retval = malloc (size);
    if (!retval) {
        exit (1);
    }
    return (retval);
}

char *safe_strdup (const char *s) {
    char *retval = NULL;
    if (!s) {
        exit (1);
    }
    retval = strdup (s);
    if (!retval) {
        exit (1);
    }
    return (retval);
}

int safe_asprintf (char **strp, const char *fmt, ...) {
    va_list ap;
    int retval;

    va_start (ap, fmt);
    retval = safe_vasprintf (strp, fmt, ap);
    va_end (ap);

    return (retval);
}

int safe_vasprintf (char **strp, const char *fmt, va_list ap) {
    int retval;

    retval = vasprintf (strp, fmt, ap);

    if (retval == -1) {
        exit (1);
    }
    return (retval);
}

pid_t safe_fork (void) {
    pid_t result;
    result = fork();

    if (result == -1) {
        exit (1);
    } else if (result == 0) {
    }

    return result;
}

/*
 * strlcpy - like strcpy/strncpy, doesn't overflow destination buffer,
 * always leaves destination null-terminated (for len > 0).
 */
int strlcpy (char *dest, const char *src, int len) {
    int ret = strlen (src);

    if (len != 0) {
        if (ret < len)
            strcpy (dest, src);
        else {
            strncpy (dest, src, len - 1);
            dest[len - 1] = 0;
        }
    }

    return ret;
}

/*
 * strlcat - like strcat/strncat, doesn't overflow destination buffer,
 * always leaves destination null-terminated (for len > 0).
 */
int strlcat (char *dest, const char *src, int len) {
    int dlen = strlen (dest);

    return dlen + strlcpy (dest + dlen, src, (len > dlen ? len - dlen : 0));
}

static int vslprintf (char *buf, int buflen, const char *fmt, va_list args) {
    int c, i, n;
    int width, prec, fillch;
    int base, len, neg, quoted;
    unsigned long val = 0;
    char *str, *buf0;
    const char *f;
    unsigned char *p;
    char num[32];
    time_t t;
    struct tm *timenow;
    u_int32_t ip;
    static char hexchars[] = "0123456789abcdef";

    buf0 = buf;
    --buflen;

    while (buflen > 0) {
        for (f = fmt; *f != '%' && *f != 0; ++f)
            ;

        if (f > fmt) {
            len = f - fmt;
            if (len > buflen)
                len = buflen;
            memcpy (buf, fmt, len);
            buf += len;
            buflen -= len;
            fmt = f;
        }

        if (*fmt == 0)
            break;

        c = *++fmt;
        width = 0;
        prec = -1;
        fillch = ' ';

        if (c == '0') {
            fillch = '0';
            c = *++fmt;
        }

        if (c == '*') {
            width = va_arg (args, int);
            c = *++fmt;
        } else {
            while (isdigit (c)) {
                width = width * 10 + c - '0';
                c = *++fmt;
            }
        }

        if (c == '.') {
            c = *++fmt;
            if (c == '*') {
                prec = va_arg (args, int);
                c = *++fmt;
            } else {
                prec = 0;

                while (isdigit (c)) {
                    prec = prec * 10 + c - '0';
                    c = *++fmt;
                }
            }
        }

        str = 0;
        base = 0;
        neg = 0;
        ++fmt;

        switch (c) {
        case 'l':
            c = *fmt++;
            switch (c) {
            case 'd':
                val = va_arg (args, long);
                if (val < 0) {
                    neg = 1;
                    val = -val;
                }
                base = 10;
                break;
            case 'u':
                val = va_arg (args, unsigned long);
                base = 10;
                break;
            default:
                OUTCHAR ('%');
                OUTCHAR ('l');
                --fmt;		/* so %lz outputs %lz etc. */
                continue;
            }
            break;
        case 'd':
            i = va_arg (args, int);
            if (i < 0) {
                neg = 1;
                val = -i;
            } else
                val = i;
            base = 10;
            break;
        case 'u':
            val = va_arg (args, unsigned int);
            base = 10;
            break;
        case 'o':
            val = va_arg (args, unsigned int);
            base = 8;
            break;
        case 'x':
        case 'X':
            val = va_arg (args, unsigned int);
            base = 16;
            break;
        case 'p':
            val = (unsigned long) va_arg (args, void *);
            base = 16;
            neg = 2;
            break;
        case 's':
            str = va_arg (args, char *);
            break;
        case 'c':
            num[0] = va_arg (args, int);
            num[1] = 0;
            str = num;
            break;
        case 'm':
            str = strerror (errno);
            break;
        case 'I':
            ip = va_arg (args, u_int32_t);
            ip = ntohl (ip);
            slprintf (num, sizeof (num), (char*) "%d.%d.%d.%d", (ip >> 24) & 0xff,
                      (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
            str = num;
            break;
        case 't':
            time (&t);
            str = (char*) ctime (&t);
            str += 4;		/* chop off the day name */
            str[15] = 0;	/* chop off year and newline */
            break;
        case 'T':
            time (&t);
            timenow = (struct tm*) localtime (&t);
            str = (char*) asctime (timenow);
            str[ strlen ( (char*) asctime (timenow)) - 1 ] = 0;
            break;
        case 'v':		/* "visible" string */
        case 'q':		/* quoted string */
            quoted = c == 'q';
            p = va_arg (args, unsigned char *);
            if (fillch == '0' && prec >= 0)
                n = prec;
            else {
                n = strlen ( (char *) p);
                if (prec >= 0 && n > prec)
                    n = prec;
            }

            while (n > 0 && buflen > 0) {
                c = *p++;
                --n;

                if (!quoted && c >= 0x80) {
                    OUTCHAR ('M');
                    OUTCHAR ('-');
                    c -= 0x80;
                }

                if (quoted && (c == '"' || c == '\\'))
                    OUTCHAR ('\\');

                if (c < 0x20 || (0x7f <= c && c < 0xa0)) {
                    if (quoted) {
                        OUTCHAR ('\\');
                        switch (c) {
                        case '\t':
                            OUTCHAR ('t');
                            break;
                        case '\n':
                            OUTCHAR ('n');
                            break;
                        case '\b':
                            OUTCHAR ('b');
                            break;
                        case '\f':
                            OUTCHAR ('f');
                            break;
                        default:
                            OUTCHAR ('x');
                            OUTCHAR (hexchars[c >> 4]);
                            OUTCHAR (hexchars[c & 0xf]);
                        }
                    } else {
                        if (c == '\t')
                            OUTCHAR (c);
                        else {
                            OUTCHAR ('^');
                            OUTCHAR (c ^ 0x40);
                        }
                    }
                } else
                    OUTCHAR (c);
            }

            continue;
        case 'B':
            p = va_arg (args, unsigned char *);
            for (n = prec; n > 0; --n) {
                c = *p++;
                if (fillch == ' ')
                    OUTCHAR (' ');
                OUTCHAR (hexchars[ (c >> 4) & 0xf]);
                OUTCHAR (hexchars[c & 0xf]);
            }
            continue;
        default:
            *buf++ = '%';
            if (c != '%')
                --fmt;		/* so %z outputs %z etc. */
            --buflen;
            continue;
        }

        if (base != 0) {
            str = num + sizeof (num);
            *--str = 0;

            while (str > num + neg) {
                *--str = hexchars[val % base];
                val = val / base;
                if (--prec <= 0 && val == 0)
                    break;
            }

            switch (neg) {
            case 1:
                *--str = '-';
                break;
            case 2:
                *--str = 'x';
                *--str = '0';
                break;
            }

            len = num + sizeof (num) - 1 - str;
        } else {
            len = strlen (str);
            if (prec >= 0 && len > prec)
                len = prec;
        }

        if (width > 0) {
            if (width > buflen)
                width = buflen;
            if ( (n = width - len) > 0) {
                buflen -= n;

                for (; n > 0; --n)
                    *buf++ = fillch;
            }
        }

        if (len > buflen)
            len = buflen;

        memcpy (buf, str, len);
        buf += len;
        buflen -= len;
    }

    *buf = 0;
    return buf - buf0;
}

int slprintf (char *buf, int buflen, char *fmt, ...) {
    va_list args;
    int n;

#if defined(__STDC__)
    va_start (args, fmt);
#else
    char *buf;
    int buflen;
    char *fmt;
    va_start (args);
    buf = va_arg (args, char *);
    buflen = va_arg (args, int);
    fmt = va_arg (args, char *);
#endif
    n = vslprintf (buf, buflen, fmt, args);

    va_end (args);

    return n;
}

void wcx_sleep (long int s, long int us) {
    struct timeval tv;
    tv.tv_sec = s;
    tv.tv_usec = us;
    if (select (0, NULL, NULL, NULL, &tv) < 0) {
        perror ("select");
    }
}

int find_pid_by_name (const char *ProcName, int* foundpid) {
    DIR             *dir;
    struct dirent   *d;
    int             pid, i;
    char            *s;
    int             pnlen;

    i = 0;
    foundpid[0] = 0;
    pnlen = strlen (ProcName);

    /* Open the /proc directory. */
    dir = opendir ("/proc");
    if (!dir) {
        //DEBUG_ERR ("cannot open /proc");
        return -1;
    }

    /* Walk through the directory. */
    while ( (d = readdir (dir)) != NULL) {
        char exe [PATH_MAX + 1];
        char path[PATH_MAX + 1];
        int len;
        int namelen;

        /* See if this is a process */
        if ( (pid = atoi (d->d_name)) == 0)
            continue;

        snprintf (exe, sizeof (exe), "/proc/%s/exe", d->d_name);
        if ( (len = readlink (exe, path, PATH_MAX)) < 0)
            continue;
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr (path, '/');
        if (s == NULL)
            continue;
        s++;

        /* we don't need small name len */
        namelen = strlen (s);
        if (namelen < pnlen)
            continue;

        if (!strncmp (ProcName, s, pnlen)) {
            /* to avoid subname like search proc tao but proc taolinke matched */
            if (s[pnlen] == ' ' || s[pnlen] == '\0') {
                foundpid[i] = pid;
                i++;
            }
        }
    }

    foundpid[i] = 0;
    closedir (dir);

    return  0;
}

void *e_malloc (size_t size) {
    void* ptr;

    ptr = malloc (size);
    if (ptr == (void*) 0) {
        syslog (LOG_CRIT, "out of memory");
        (void) fprintf (stderr, "%s: out of memory\n", "wcxHttpd");
        exit (1);
    }
    return ptr;
}

void *e_realloc (void* optr, size_t size) {
    void* ptr;

    ptr = realloc (optr, size);
    if (ptr == (void*) 0) {
        syslog (LOG_CRIT, "out of memory");
        (void) fprintf (stderr, "%s: out of memory\n", "wcxHttpd");
        exit (1);
    }
    return ptr;
}

void add_to_buf (char** bufP, size_t* bufsizeP, size_t* buflenP, char* str, size_t len) {
    if (*bufsizeP == 0) {
        *bufsizeP = len + 500;
        *buflenP = 0;
        *bufP = (char*) e_malloc (*bufsizeP);
    } else if (*buflenP + len >= *bufsizeP) {
        *bufsizeP = *buflenP + len + 500;
        *bufP = (char*) e_realloc ( (void*) * bufP, *bufsizeP);
    }
    (void) memmove (& ( (*bufP) [*buflenP]), str, len);
    *buflenP += len;
    (*bufP) [*buflenP] = '\0';
}

void add_to_request (char *request, size_t *request_size, size_t *request_len, char* str, size_t len) {
    add_to_buf (&request, request_size, request_len, str, len);
}

void msg_to_http_server (const char *cmd, const char *msg) {
#if 0
    char *buf;
    struct sockaddr_in servaddr;
    int sockfd;

    safe_asprintf (&buf, "GET /?cmd=%s&value=%s HTTP/1.1\r\n\r\n", cmd, msg);

    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        free (buf);
        return;
    }

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton (AF_INET, A20_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons (LIANGLIANG_PORT);

    if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) == 0) {
        while (write (sockfd, buf, strlen (buf)) == -1) {
            if (errno != EINTR)
                break;
        }
    }

    close (sockfd);
    free (buf);
#endif
}

void report_plug_event (const char *msg) {
#if 0
    char *buf;
    struct sockaddr_in servaddr;
    int sockfd;

    //safe_asprintf (&buf, "GET /cgi-bin/plug_event?value=%s HTTP/1.1\r\n\r\n", msg);
    safe_asprintf (&buf, "GET /@mac@192.168.43.1@xyz/plug_event?value=%s HTTP/1.1\r\n\r\n", msg);

    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        free (buf);
        return;
    }

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton (AF_INET, A20_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons (LIANGLIANG_PORT);

    if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) == 0) {
        while (write (sockfd, buf, strlen (buf)) == -1) {
            if (errno != EINTR)
                break;
        }
    }

    close (sockfd);
    free (buf);
#endif
}

void post_to_http_server (const char *cmd, const char *msg) {
#if 0
    char *buf;
    struct sockaddr_in servaddr;
    int sockfd;

    safe_asprintf (&buf, "POST /?cmd=%s HTTP/1.1\r\n\
Content-Length: %d\r\n\r\n\
%s\r\n\r\n",
                   cmd, strlen (msg) + 1, msg);

    printf ("post:\n{%s}\n", buf);

    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        free (buf);
        return;
    }

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton (AF_INET, A20_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons (LIANGLIANG_PORT);

    if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) == 0) {
        while (write (sockfd, buf, strlen (buf)) == -1) {
            if (errno != EINTR)
                break;
        }
    }

    close (sockfd);
    free (buf);
#endif
}

char *http_request (const char *request) {
    FILE *pp;
    static char ret_buf[2048];
    char cmd[256];

    snprintf (cmd, sizeof (cmd), "curl -l \"%s\"", request);
    pp = popen (cmd, "r");
    if (!pp) {
        return NULL;
    } else {
        memset (ret_buf, 0, sizeof (ret_buf));
        fgets (ret_buf, sizeof (ret_buf), pp);
        pclose (pp);
        char *p = ret_buf;
        return p;
    }
}
