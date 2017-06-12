
#include <sys/unistd.h> // gethostname()
#include <getopt.h>     // for getopt()
#include <stdarg.h>     // for variable arguments
#include <stdlib.h>     // for malloc()
#include <time.h>       // strftime()
#include <stdio.h>      // for fprintf()
#include <netdb.h>      // for getnameinfo()
#include <string.h>     // memset()

#include <microhttpd.h>


static size_t slen (
    const char *a)
{
    size_t n = 0;
    while (a != NULL && a[n]) n++;
    return n;
}


static char *scat (
    char *first, 
    ...)
{
    va_list argp;
    char *buf, *p;
    size_t n;
    size_t i = 0;
 
    n = slen(first);

    va_start(argp, first);
    while((p = va_arg(argp, char *)) != NULL)
        n += slen(p);
    va_end(argp);
    
    buf = malloc(n + 1);
    if(buf == NULL)
        return NULL;

    if (first != NULL){
        for (p = first, n = 0; p[n]; n++) buf[i++] = p[n];
        free(first);
    }

    va_start(argp, first);
    while((p = va_arg(argp, char *)) != NULL)
        for (n = 0; p[n]; n++) buf[i++] = p[n];
    va_end(argp);
    buf[i] = 0;

    return buf;
}


static char *scopy (
    const char *s)
{
    char *buf;
    int i, n;

    if (s == NULL)
        return NULL;

    for (n = 0; s[n]; n++) ;

    buf = malloc(n + 1);
    if (buf == NULL)
        return NULL;

    for (i = 0; i < n; i++) buf[i] = s[i];
    buf[i] = 0;
    return buf;
}


static char *
getfqdn()
{
    char hostname[1024], *p;
    struct addrinfo hints, *info;
    int rv;

    // gethostname() uses /etc/hostname to get the short name

    rv = gethostname(hostname, sizeof hostname);
    if (rv) return NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_CANONNAME;

    // getaddrinfo() uses /etc/hosts to get the FQDN

    rv = getaddrinfo(hostname, NULL, &hints, &info);
    if (rv) return NULL;

    p = scopy(info->ai_canonname);
    freeaddrinfo(info);
    return p;
}


////////////////////////////

const char *host = NULL;


static int handler (
    void *cls,
    struct MHD_Connection *conn,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data, 
    size_t *upload_data_size, 
    void **con_cls)
{
    int status = 301;
    char *new_url;

    {
    time_t rawtime;
    struct tm *info;
    const struct sockaddr *sa;
    char remote[1024], date[80];

    sa = MHD_get_connection_info (conn,
             MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;

    if (getnameinfo (sa, sizeof (struct sockaddr), remote, sizeof remote, 
                     NULL, 0, NI_NUMERICHOST))
       return MHD_NO;

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(date, sizeof date, "%d/%b/%Y:%H:%M:%S %z", info); 

    printf ("%s - - [%s] \"%s %s %s\" %d 0\n", remote, date, method, url, version, status);
    fflush (stdout);
    }

    // get new url

    {
    new_url = scat (NULL, "https://", host, url, NULL);
    if (new_url == NULL) return MHD_NO;
    }

    // create a response

    {
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer (
        0, NULL, MHD_RESPMEM_PERSISTENT);

    if (response == NULL){
        free(new_url);
        return MHD_NO;
    }

    MHD_add_response_header (response, MHD_HTTP_HEADER_LOCATION, new_url);
    MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_TYPE, 
                            "application/octet-stream");
    ret = MHD_queue_response (conn, status, response);

    MHD_destroy_response(response);
    free(new_url);

    return ret;
    }
}
 

int main (
    int argc, 
    char *const *argv)
{
    int port = 80;
    unsigned int timeout = 120; // seconds
    struct MHD_Daemon *d;
    int c, flags;

    flags = MHD_USE_SELECT_INTERNALLY | // faster for this application
            MHD_USE_POLL;               // want to handle many connections

    while ((c = getopt(argc, argv, "hvp:H:")) != -1){
       switch (c){
       case 'v':
           flags |= MHD_USE_DEBUG;
           break;
       case 'p':
           port = atoi(optarg);
           break;
       case 'H':
           host = optarg;
           break;
       case 'h':
           fprintf(stderr, "Usage: %s [options]\n", argv[0]);
           fprintf(stderr, "Options are:\n");
           fprintf(stderr, "  -v             Enable verbose mode\n");
           fprintf(stderr, "  -p portnumber  Server Port [%d]\n", port);
           fprintf(stderr, "  -H hostname    Host to redirect to\n");
           return 0;
       case ':':
       case '?':
           return 0;
       }
    }

    if (host == NULL)
        host = getfqdn();

    if (host == NULL)
        return 1;    

    d = MHD_start_daemon (
        flags, port, 
        NULL, NULL,      // apc callback to check ip and extra arg
        &handler, NULL,  // dh handler and extra arg
        MHD_OPTION_CONNECTION_TIMEOUT, timeout,
        MHD_OPTION_END);

    if (d == NULL)
        return 1;
    
    fprintf(stderr, "listening on port %d\n", port);
    pause();
    return 0;
}

