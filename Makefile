all:http_server

http_server:http_conn.cpp http_server.cpp
	g++ -o $@ $^ -lpthread

.PHONY:clean

clean:
	rm -f http_server
