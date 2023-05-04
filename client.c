#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "3490"	// la porta necessaria per il server

int main(int argc, char *argv[])
{
	int s;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	//Prendo le info del client e verifico se la porta è libera
	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// p è una lista concatenata: per prendere il primo valore devo fare ai_next
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((s = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("Errore nel Socket");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}


	//Invio e controllo
	if ((numbytes = sendto(s, argv[2], strlen(argv[2]), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("Errore nell'invio del messaggio");
		exit(1);
	}

	//libero tutto
	freeaddrinfo(servinfo);

	printf("Inviati %d bytes a %s\n", numbytes, argv[1]);
	close(s);

	return 0;
}
