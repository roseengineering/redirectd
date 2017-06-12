
If you want to run a web server on HTTPS, you probably also want to run another
web service on HTTP to redirect connections over to your HTTPS server.  Installing a 
heavyweight server on port 80 just to do redirects can seem like overkill.  

The included program, redirectd, tries to fill this gap.  All it does is redirects, 
nothing else.  Any HTTP request, regardless of the method, to the service will 
result in a 301 redirection to HTTPS.  All connections are logged to standard output.

For example a GET request to the service listening on say example.com on port 8000
would result in the following response:

```
HTTP/1.1 301 Moved Permanently
Connection: Keep-Alive
Content-Length: 0
Content-Type: application/octet-stream
Location: https://example.com/path/to
Date: Mon, 12 Jun 2017 00:40:19 GMT
```

The url path is preserved.  However all query strings are stripped.

The port the serivce listens on by default 80 but can be changed using the -p option.
The service attempts to find the fully qualified hostname of the current machine
using /etc/hosts.  If it determines wrong or if the service needs to redirect the
connection to another host, this can be changed using the -H option.  The -v option 
tells the linked libmicrohttpd library to turn on debugging mode.  So if the service 
does not run try it again but with -v.

The install.sh script will compile the redirectd serivce as well as install it 
in /usr/bin.  The provided systemd file redirectd.service will also be installed 
and enabled.  The code requires the library libmicrohttpd to be installed.

Benchmarking with "ab" using localhost gives the numbers:

```
$ ab -c 100 -n 100000 http://localhost:8000/path/to

Concurrency Level:      100
Time taken for tests:   4.419 seconds
Complete requests:      100000
Failed requests:        0
Non-2xx responses:      100000
Total transferred:      16400000 bytes
HTML transferred:       0 bytes
Requests per second:    22628.98 [#/sec] (mean)
Time per request:       4.419 [ms] (mean)
Time per request:       0.044 [ms] (mean, across all concurrent requests)
Transfer rate:          3624.17 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.8      1      10
Processing:     1    4   4.0      3      38
Waiting:        0    3   3.9      3      38
Total:          1    4   4.1      3      38

Percentage of the requests served within a certain time (ms)
  50%      3
  66%      4
  75%      4
  80%      5
  90%      6
  95%     13
  98%     22
  99%     24
 100%     38 (longest request)
```
