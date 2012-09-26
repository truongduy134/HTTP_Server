CC = gcc
LIBS = -pthread

all: server client testCmd

server: server.o m_http.o helperlib.o queue.o
	$(CC) $(LIBS) server.o m_http.o helperlib.o queue.o -o server
server.o: server.c m_http.h helperlib.h queue.h
	$(CC) -c server.c

client: client.o helperlib.o
	$(CC) $(LIBS) client.o helperlib.o -o client
client.o: client.c helperlib.h
	$(CC) -c client.c

testCmd: testCmd.o helperlib.o m_http.o
	$(CC) testCmd.o helperlib.o m_http.o -o testCmd
testCmd.o: testCmd.c helperlib.h m_http.h
	$(CC) -c testCmd.c

m_http.o: m_http.c m_http.h helperlib.h
	$(CC) -c m_http.c
helperlib.o: helperlib.c helperlib.h
	$(CC) -c helperlib.c
queue.o: queue.c queue.h
	$(CC) -c queue.c
