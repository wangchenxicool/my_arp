#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sendrecv.h"
#include "curl.h"

/* Auxiliary function that waits on the socket. */
static int wait_on_socket (int sockfd, int for_recv, long timeout_ms) {
    struct timeval tv;
    fd_set infd, outfd, errfd;
    int res;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    FD_ZERO (&infd);
    FD_ZERO (&outfd);
    FD_ZERO (&errfd);

    FD_SET (sockfd, &errfd); /* always check for error */

    if (for_recv) {
        FD_SET (sockfd, &infd);
    } else {
        FD_SET (sockfd, &outfd);
    }

    /* select() returns the number of signalled sockets or -1 */
    res = select (sockfd + 1, &infd, &outfd, &errfd, &tv);
    return res;
}

//example: *request = "GET /cgi-bin/net_cfg?cmd=get_ch_info HTTP/1.0\r\nHost: curl.haxx.se\r\n\r\n";
int sendrecv (const char *request, char *response, int response_len) {
    CURL *curl;
    CURLcode res;
    int sockfd; /* socket */
    size_t iolen;

    printf ("........sendrecv start..\n");

    curl = curl_easy_init();
    if (curl) {
        /* set URL */
        curl_easy_setopt (curl, CURLOPT_URL, "http://api.heclouds.com/devices/634648/datapoints");
        //curl_easy_setopt (curl, CURLOPT_URL, "http://api.heclouds.com/devices/634648/");
        //curl_easy_setopt (curl, CURLOPT_URL, "http://127.0.0.1/devices/634648/");
        //curl_easy_setopt (curl, CURLOPT_URL, "127.0.0.1");

        /* set Head */
        //curl_easy_setopt (curl, CURLOPT_HEADER, "api-key:nJCjW8bhvx=aWcIJcogptgbNo0I=");

        /* Now specify we want to GET data */
        //curl_easy_setopt (curl, CURLOPT_GET, 1L);

        /* Now specify the POST data */
        //curl_easy_setopt (curl, CURLOPT_POSTFIELDS, "first=0");

        /* Do not do the transfer - only connect to host */
        curl_easy_setopt (curl, CURLOPT_CONNECT_ONLY, 1L);

        res = curl_easy_perform (curl);

        if (CURLE_OK != res) {
            printf ("Error: %s\n", strerror (res));
            return -1;
        }

        res = curl_easy_getinfo (curl, CURLINFO_LASTSOCKET, &sockfd);

        if (CURLE_OK != res) {
            printf ("Error: %s\n", curl_easy_strerror (res));
            return -1;
        }

        /* wait for the socket to become ready for sending */
        if (!wait_on_socket (sockfd, 0, 60000L)) {
            printf ("Error: timeout.\n");
            return -1;
        }

        res = curl_easy_send (curl, request, strlen (request), &iolen);

        if (CURLE_OK != res) {
            printf ("Error: %s\n", curl_easy_strerror (res));
            return -1;
        }

        if (NULL != response) {
            /* read the response */
            for (;;) {
                //char buf[1024];

                wait_on_socket (sockfd, 1, 60000L);

                usleep (100 * 1000);

                res = curl_easy_recv (curl, response, response_len, &iolen);

                if (CURLE_OK != res)
                    break;

                printf ("Received %u bytes. rcv:{%s}\n", iolen, response);

                //char *p = strstr (response, "\r\n\r\n");
                //if (p)
                //{
                //printf ("body:\n{%s}\n", p + strlen("\r\n\r\n"));
                //}
            }
            printf ("rcv:\n%s\n", response);
        }

        /* always cleanup */
        curl_easy_cleanup (curl);
    }

    printf ("........sendrecv end..\n");
    return 0;
}

#if 0
int main () {
    char response[1024];
    const char *request = "GET /cgi-bin/net_cfg?cmd=get_ch_info HTTP/1.0\r\n\r\n";

    int ret = sendrecv (request, response, sizeof (response));
    if (0 == ret) {
        char *p = strstr (response, "\r\n\r\n");
        if (p) {
            printf ("....body:\n{%s}\n", p + strlen ("\r\n\r\n"));
        }
    }
}
#endif
