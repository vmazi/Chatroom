all: chatserver chatclient

chatserver: tcpserver.c tcpserver.h libserv.a
	gcc -o chatserver tcpserver.c tcpserver.h libserv.a -Wall -Werror -g -pthread

chatclient: tcpclient.c libserv.a
	gcc -o chatclient tcpclient.c libserv.a -g -Wall -Werror -pthread

libserv.a: tokenizer.o hashtable.o
	ar -r libserv.a tokenizer.o hashtable.o 

tokenizer.o: tokenizer.c tokenizer.h
	gcc -c tokenizer.c tokenizer.h -Wall -Werror -g

hashtable.o: hashtable.c hashtable.h
	gcc -c hashtable.c hashtable.h -Wall -Werror -g




clean: 
	rm -r -f chatclient chatserver Makefile~ libserv.a tokenizer.h.gch hashtable.h.gch tokenizer.o hashtable.o tokenizer.c~ hashtable.c~ tokenizer.h~ hashtable.h~ tcpserver.c~ tcpserver.h~ tcpclient.h~ tcpclient.c~ readme.pdf~ testplan.txt~ testcases.txt~ chathist.txt chathist.txt~