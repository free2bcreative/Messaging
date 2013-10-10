# Makefile for Messaging labs

CXX=		g++ $(CCFLAGS)
CLIENT=		Client.o client-main.o
SERVER=		Server.o AllUsers.o User.o Message.o server-main.o Handler.o
OBJS =		$(CLIENT) $(SERVER) 

LIBS=		-pthread

CCFLAGS= -g -Wall

all:	client-main server-main

client-main:$(CLIENT)
	$(CXX) -o msg $(CLIENT) $(LIBS)

server-main:$(SERVER)
	$(CXX) -o msgd $(SERVER) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) msg msgd


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
