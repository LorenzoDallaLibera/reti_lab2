//Lorenzo Dalla Libera 2009289
//Alessandro Rossi 2003713
//Rachele Milani 1204804

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

#define MYPORT "3490"	// the port users will be connecting to

#define MAXBUFLEN 100
#define CONFERMA_RICEZIONE "ricevuto"

// get sockaddr, IPv4 or IPv6:


int main(void)
{
	int sockfd;
	struct addrinfo *p;
	//int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;


	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	//struct addrinfo  *res;


	//riservo la memoria per hints, in maniera da non aver problemi coi puntatori
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;



	//APERTURA DEL SOCKET IN ASCOLTO (Server)

	//PRIMA VERIFICA: prendo le info del server e verifico se la porta è libera
    if ((status = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		printf("Errore di gettaddrinfo: %s\n", gai_strerror(status));
		exit(1);
	}

	//SECONDA VERIFICA: verifico se i dati sono coerenti (problemi di firewall? ecc...)
	//p=servinfo è una lista concatenata: per prendere il primo valore devo fare ai_next
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("Errore nel Socket");
			continue;
		}
		break;
	}

	//TERZA VERIFICA: verifico se il socket è stato aperto (altri errori?)
	if (p == NULL) {
		fprintf(stderr, "Fallita connessione con il socket\n");
		return 2;
	}



//Se tutto ok-->
//SECONDA PARTE: BIND

	//faccio il "binding". Il bind è un'operazione che associa un socket ad un indirizzo
	//ripasso tutti i risultati. Aggancio il primo disponibile
	for(p = servinfo; p != NULL; p = p->ai_next) {
		//APERTURA DEL SOCKET IN ASCOLTO (Server)
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
			close(sockfd);
			perror("Errore nel bind");
			continue;
		}
		break;
	}

	//(non fondamentale) ricontrollo: nel frattempo è cambiato qualcosa?
	if (p == NULL) {
		fprintf(stderr, "Errore nel bind\n");
		return 2;
	}

	//Pulisco la memoria
	//Non fondamentale ma buona pratica per evitare "memory leak"
	freeaddrinfo(servinfo);

	printf("Da qui in poi il Server in attesa...\n");


	while(1){
		//Ricevo il messaggio
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}


		//save data to inet_ntop IP4
		char ss[INET6_ADDRSTRLEN];
		inet_ntop(their_addr.ss_family, &(((struct sockaddr_in6 *)&their_addr)->sin6_addr), ss, sizeof ss);
		printf("-->Pacchetto ricevuto da %s...\n", ss);
		printf("...di lunghezza %d bytes...\n", numbytes);
		buf[numbytes] = '\0';
		printf("...Contentente: \"%s\"...\n", buf);

		//conferma di ricezione
		if ((numbytes = sendto(sockfd, CONFERMA_RICEZIONE, strlen(CONFERMA_RICEZIONE), 0, (struct sockaddr *)&their_addr, (socklen_t)addr_len)) == -1) {
			perror("Errore nell'invio del messaggio");
			exit(1);
		}

		//fermo il server se ricevo il comando "crash"
		if(strcmp(buf,"crash")==0){
			printf("Spegnimento\n");
			break;
		}

		printf("In ricezione...\n");

	}

	close(sockfd);

	return 0;
}
