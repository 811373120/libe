#include "stdafx.h"
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

//#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include "Messages.h"
using namespace std;
struct Client {
	bufferevent *bev;
	char ip[20];
};
vector<Client> clientsbev;
static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
	/* This callback is invoked when there is data to read on bev. */
	//struct evbuffer *input = bufferevent_get_input(bev);
	//struct evbuffer *output = bufferevent_get_output(bev);
	//11111
	///* Copy all the data from the input buffer to the output buffer. */
	//evbuffer_add_buffer(output, input);
	//printf("%s", output);
	Msg s;
	string str;
	#define MAX_LINE 256
	char line[MAX_LINE + 1];
	int n;
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	memset(line, 0, sizeof(line));
	while (n = bufferevent_read(bev, line, MAX_LINE))
	{

		//line[n] = '\0';
		//str += line;
		////printf("fd=%u, read line: %s\n", fd, line);
		//bufferevent_write(bev, line, n);
	}
	memcpy(&s, line, sizeof(Msg));
//	int len = str.size();
//	const char *t=new char[len+1];
//	t = str.c_str();
//	s = (Msg *)t;
	//cout << s.age<<" "<<s.num;
	//cout << s.ip;
	//cout << str;
}
static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	if (events & BEV_EVENT_ERROR)
		perror("Error from bufferevent");
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
		bufferevent_free(bev);
	}
}

static void
accept_conn_cb(struct evconnlistener *listener,
	evutil_socket_t fd, struct sockaddr *address, int socklen,
	void *ctx)
{
	for (auto it = clientsbev.begin(); it != clientsbev.end();it++) {
		printf("%s\n", it->ip);
	}
	/* We got a new connection! Set up a bufferevent for it. */
	//char str[20] = {0};
	char *s = inet_ntoa(((struct sockaddr_in *)address)->sin_addr);
	//memcpy(str, s, strlen(s));
	//printf("one conn from %s", str);
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(
		base, fd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

	bufferevent_enable(bev, EV_READ | EV_WRITE);
	Client c;
	memset(&c, 0, sizeof(c));
	c.bev = bev;
	memcpy(c.ip,s,strlen(s));
	clientsbev.push_back(c);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Got an error %d (%s) on the listener. "
		"Shutting down.\n", err, evutil_socket_error_to_string(err));

	event_base_loopexit(base, NULL);
}

int
main(int argc, char **argv)
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	struct event_base *base;
	struct evconnlistener *listener;
	struct sockaddr_in sin;

	int port = 1234;

	if (argc > 1) {
		port = atoi(argv[1]);
	}
	if (port <= 0 || port>65535) {
		puts("Invalid port");
		return 1;
	}

	base = event_base_new();
	if (!base) {
		puts("Couldn't open event base");
		return 1;
	}

	/* Clear the sockaddr before using it, in case there are extra
	* platform-specific fields that can mess us up. */
	memset(&sin, 0, sizeof(sin));
	/* This is an INET address */
	sin.sin_family = AF_INET;
	/* Listen on 0.0.0.0 */
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	/* Listen on the given port. */
	sin.sin_port = htons(port);

	listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
		LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
		(struct sockaddr*)&sin, sizeof(sin));
	if (!listener) {
		perror("Couldn't create listener");
		return 1;
	}
	evconnlistener_set_error_cb(listener, accept_error_cb);

	event_base_dispatch(base);
	return 0;
}