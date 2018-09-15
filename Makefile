
all:
	gcc serv1.c keyvalue.h -o serv1 
	gcc serv2.c keyvalue.h -o serv2
	gcc serv3.c keyvalue.h -o serv3
	gcc -pthread serv4.c keyvalue.h -o serv4
	gcc client.c -o client

serv2:
	gcc serv2.c keyvalue.h -o serv2

debug_header:
	gcc -D_debug -D_debugheader serv1.c keyvalue.h -o serv1

debug_client:
	gcc -D_debug -D_dontdebugthis client.c -o client

client:
	gcc client.c -o client

debug_everything_old:
#	gcc -D_debug client.c -o client
	gcc -D_debug serv1.c keyvalue.h -o serv1
	gcc -D_debug serv2.c keyvalue.h -o serv2

debug_everything:
#	gcc -D_debug client.c -o client
	gcc -D_debug -D_debugheader serv1.c keyvalue.h -o serv1
	gcc -D_debug -D_debugheader serv2.c keyvalue.h -o serv2
	gcc -D_debug -D_debugheader serv3.c keyvalue.h -o serv3
	gcc -pthread -D_debugheader -D_debug serv4.c keyvalue.h -o serv4

clean:
	rm *.o
	rm serv1
	rm serv2
	rm client
