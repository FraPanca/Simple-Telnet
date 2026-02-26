#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

// limiti ipotetici - nella realtà non vi è un numero massimo di caratteri o comandi, ma esiste solo un limite complessivo di memoria usata (comando bash: getconf ARG_MAX)
#define MAX_DIM_CMD 256
#define MAX_NUM_ARGS 32


typedef struct {
	char command[MAX_DIM_CMD];
	char args[MAX_NUM_ARGS][MAX_DIM_CMD];
	int  nArgs;
} Request;


int main(int argc, char** argv) {
	
	int sd, port, len, nread, countArgs, countCmd, ris;	
	char cmd[MAX_DIM_CMD*(MAX_NUM_ARGS+2)];
	
	struct hostent *host;
   struct sockaddr_in clientaddr, serveraddr;

	Request req;
	
	
	//controllo argomenti
	if(argc != 3) {
		printf("[CLIENT] : Errore -> Numero di argomenti errato\n"),
		exit(1);
	}
	
	
	// init indirizzo server
	memset((char *) &serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	
	host = gethostbyname(argv[1]);
	if(host == NULL) {
		printf("[CLIENT] : Errore -> L'host %s non è stato trovato in /etc/hosts\n", argv[1]);
		exit(2); 
	}
	
	// controllo della porta
	nread = 0;
	while(argv[2][nread] != '\0') {
		if(argv[2][nread] < '0' || argv[2][nread] > '9') {
			printf("[CLIENT] : Errore -> Il numero di porta non è un intero\n");
			exit(3);
		}
		nread++;
	}

	port = atoi(argv[2]);
	if(port < 1024 || port > 65535) {
		printf("[CLIENT] : Errore -> Il numero di porta deve essere compreso tra 1024 e 65535\n");
		exit(3);
	}
	
	serveraddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
	serveraddr.sin_port = htons(port);
	
	
	// init indirizzo client
	memset((char *) &clientaddr, 0, sizeof(struct sockaddr_in));
   clientaddr.sin_family = AF_INET;
   clientaddr.sin_addr.s_addr = INADDR_ANY;
   clientaddr.sin_port = 0;
   
   
   printf("[CLIENT] : Avviato correttamente.\n");
   
   // creazione socket
   if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
   	perror("[CLIENT] : Errore -> Impossibile creare la socket");
   	exit(4);
   }
   
   // bind della socket
   if(bind(sd, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0) {
   	perror("[CLIENT] : Errore -> Impossibile effettuare la bind");
   	exit(4);
   }
   
   printf("[CLIENT] : Socket creata correttamente.\n");
   
   
   // richiesta del comando
   printf("[CLIENT] : Inserisci il comando:\t");
   gets(cmd);
   
   // formattazione della richiesta
   nread = 0;
   while(cmd[nread] != ' ' && cmd[nread] != '\0') {
   	req.command[nread] = cmd[nread];   	
   	nread++;
   }
   req.command[nread++] = '\0';
   
   
   countCmd = 0;
   countArgs = 0;
   while(cmd[nread] != '\0') {
   	if(cmd[nread] == ' ') {
   		if(countCmd > 0) {
   			req.args[countArgs++][countCmd] = '\0';
            countCmd = 0;
         }
        
        	while(cmd[nread] == ' ') nread++;
        	continue;
      }
      
    	req.args[countArgs][countCmd++] = cmd[nread++];
	}
	
   if(countCmd > 0) req.args[countArgs++][countCmd] = '\0';
   req.nArgs = countArgs;
   
   
   // comunicazione col server
   len = sizeof(serveraddr);
   
   // invio richiesta al server
   if(sendto(sd, &req, sizeof(req), 0, (struct sockaddr *) &serveraddr, len) < 0) {
   	perror("[CLIENT] : Errore -> Non è stato possibile inviare la richiesta al server");
   	exit(5);   
   }
   
   printf("[CLIENT] : Richiesta inviata correttamente.\n[CLIENT] : In attesa di risposta...\n");
   
   // ricezione risposta dal server
   if(recvfrom(sd, &ris, sizeof(int), 0, (struct sockaddr *) &serveraddr, &len) < 0) {
   	perror("[CLIENT] : Errore -> Non è stato possibile ricevere la risposta dal server");
   	exit(5); 
   }
   
   printf("[CLIENT] : Risposta ricevuta!\n[CLIENT] : Valore di ritorno del comando: %d\n[CLIENT] : Termino...\n", ris);
   
   // chiusura della socket
   close(sd);
   
	exit(0);
}

