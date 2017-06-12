
redirectd: redirectd.c
	gcc -g -o $@ $^ -lmicrohttpd --std=gnu99

clean: redirectd
	rm redirectd

.PHONY: clean


