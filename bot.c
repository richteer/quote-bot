/* bot.c
** libstrophe XMPP client library -- basic usage example
**
** Copyright (C) 2005-2009 Collecta, Inc. 
**
**  This software is provided AS-IS with no warranty, either express
**  or implied.
**
**  This software is distributed under license and may not be copied,
**  modified or distributed except as expressly authorized under the
**  terms of the license contained in the file LICENSE.txt in this
**  distribution.
*/

/* simple bot example
**  
** This example was provided by Matthew Wild <mwild1@gmail.com>.
**
** This bot responds to basic messages and iq version requests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <strophe.h>
#include "quotereader.h"

char * room;
char roomsuffix[] = "@conference.xmpp.cslabs.clarkson.edu";

int version_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
	xmpp_stanza_t *reply, *query, *name, *version, *text;
	char *ns;
	xmpp_ctx_t *ctx = (xmpp_ctx_t*)userdata;
	printf("Received version request from %s\n", xmpp_stanza_get_attribute(stanza, "from"));
	
	reply = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(reply, "iq");
	xmpp_stanza_set_type(reply, "result");
	xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
	xmpp_stanza_set_attribute(reply, "to", xmpp_stanza_get_attribute(stanza, "from"));
	
	query = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(query, "query");
    ns = xmpp_stanza_get_ns(xmpp_stanza_get_children(stanza));
    
	if (ns) {
        xmpp_stanza_set_ns(query, ns);
    }

	name = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(name, "name");
	xmpp_stanza_add_child(query, name);
	
	text = xmpp_stanza_new(ctx);
	xmpp_stanza_set_text(text, "libstrophe example bot");
	xmpp_stanza_add_child(name, text);
	
	version = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(version, "version");
	xmpp_stanza_add_child(query, version);
	
	text = xmpp_stanza_new(ctx);
	xmpp_stanza_set_text(text, "1.0");
	xmpp_stanza_add_child(version, text);
	
	xmpp_stanza_add_child(reply, query);

	xmpp_send(conn, reply);
	xmpp_stanza_release(reply);
	return 1;
}


int message_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
	xmpp_stanza_t *reply, *body, *text;
	char *intext, *replytext;
	xmpp_ctx_t *ctx = (xmpp_ctx_t*)userdata;	
	char * username;
	char * chatroom;

	if(!xmpp_stanza_get_child_by_name(stanza, "body")) return 1;
	if(!strcmp(xmpp_stanza_get_attribute(stanza, "type"), "error")) return 1;
	
	intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));
	
	printf("Incoming message from %s: %s\n", xmpp_stanza_get_attribute(stanza, "from"), intext);

	reply = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(reply, "message");
	if (!strcmp(xmpp_stanza_get_type(stanza), "groupchat")) {
		chatroom = calloc(1,sizeof(roomsuffix) + strlen(room) + 1);
		strcpy(chatroom,room);
		strcat(chatroom, roomsuffix);
		xmpp_stanza_set_type(reply, "groupchat");
		xmpp_stanza_set_attribute(reply, "to", chatroom);
		xmpp_stanza_set_attribute(reply, "from", xmpp_stanza_get_attribute(stanza, "to"));
	} else {
		xmpp_stanza_set_type(reply, "chat");
		xmpp_stanza_set_attribute(reply, "to", xmpp_stanza_get_attribute(stanza, "from"));
	}
	username = xmpp_stanza_get_attribute(stanza, "from");
	
	body = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(body, "body");


	if (!strncmp(intext,"!quote",strlen("!quote"))) {
		replytext = strdup(get_random_quote());
	}
	else {
		return 1;
	}
	
	text = xmpp_stanza_new(ctx);
	xmpp_stanza_set_text(text, replytext);
	xmpp_stanza_add_child(body, text);
	xmpp_stanza_add_child(reply, body);
	
	xmpp_send(conn, reply);
	xmpp_stanza_release(reply);
	free(replytext);
	return 1;
}

/* define a handler for connection events */
void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status, 
		  const int error, xmpp_stream_error_t * const stream_error,
		  void * const userdata)
{
    char * chatroom;
	
	xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;

    if (status == XMPP_CONN_CONNECT) {
	xmpp_stanza_t* pres;
	fprintf(stderr, "DEBUG: connected\n");
	xmpp_handler_add(conn,version_handler, "jabber:iq:version", "iq", NULL, ctx);
	xmpp_handler_add(conn,message_handler, NULL, "message", NULL, ctx);
	
	/* Send initial <presence/> so that we appear online to contacts */
	pres = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(pres, "presence");
	xmpp_send(conn, pres);
	xmpp_stanza_release(pres);
    }
    else {
	fprintf(stderr, "DEBUG: disconnected\n");
	xmpp_stop(ctx);
    }

	/* Join the chatroom specified by argv[3] */
	xmpp_stanza_t *joinmuc;
	joinmuc = xmpp_stanza_new(ctx);
	xmpp_stanza_t * child = xmpp_stanza_new(ctx);
	xmpp_stanza_t * history = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(child, "x");
	xmpp_stanza_set_attribute(child, "xmlns", "http://jabber.org/protocol/muc");

	chatroom = calloc(1,sizeof(roomsuffix) + strlen(room) + strlen("/quotebot")+ 1);
	strcpy(chatroom, room);
	strcat(chatroom, roomsuffix);
	strcat(chatroom, "/quotebot");

	xmpp_stanza_set_name(joinmuc,"presence");
	xmpp_stanza_set_attribute(joinmuc, "to", chatroom);
	xmpp_stanza_set_name(history, "history");
	xmpp_stanza_set_attribute(history, "maxchars","0");
	xmpp_stanza_add_child(child,history);
	xmpp_stanza_add_child(joinmuc,child);
	xmpp_send(conn, joinmuc);

}

int main(int argc, char **argv)
{
    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;
    char *jid, *pass;

    /* take a jid and password on the command line */
    if (argc != 4) {
	fprintf(stderr, "Usage: bot <jid> <pass> <chatroom> \n\n");
	return 1;
    }
   
	if (0 != init_quotes()) {
		fprintf(stderr,"failed to initialize quotes\n");
		return 1;
	}

    jid = argv[1];
    pass = argv[2];
	room = argv[3];

    /* init library */
    xmpp_initialize();

    /* create a context */
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG); /* pass NULL instead to silence output */
    ctx = xmpp_ctx_new(NULL, log);

    /* create a connection */
    conn = xmpp_conn_new(ctx);

    /* setup authentication information */
    xmpp_conn_set_jid(conn, jid);
    xmpp_conn_set_pass(conn, pass);

    /* initiate connection */
    xmpp_connect_client(conn, NULL, 0, conn_handler, ctx);
	
    /* enter the event loop - 
       our connect handler will trigger an exit */
    xmpp_run(ctx);

    /* release our connection and context */
    xmpp_conn_release(conn);
    xmpp_ctx_free(ctx);

    /* final shutdown of the library */
    xmpp_shutdown();

    return 0;
}
