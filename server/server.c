#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

// limiti ipotetici - nella realtà non vi è un numero massimo di caratteri o comandi, ma esiste solo un limite complessivo di memoria usata (comando bash: getconf ARG_MAX)
#define MAX_DIM_CMD 256
#define MAX_NUM_ARGS 32

#define MAX_DIM_BUF 256


typedef struct {
	char command[MAX_DIM_CMD];
	char args[MAX_NUM_ARGS][MAX_DIM_CMD];
	int  nArgs;
} Request;


int main(int argc, char** argv) {

	const int on = 1;
	int listensd, connsd, udpsd, port, len, maxfd, pid, status, nread, countArgs, countCmd, i;	
	char *argsExec[MAX_NUM_ARGS + 2];
	
	fd_set rset;
	
   struct sockaddr_in clientaddr, serveraddr;

	Request req;
	
	
	//controllo argomenti
	if(argc != 2) {
		printf("[SERVER] : Errore -> Numero di argomenti errato\n"),
		exit(1);
	}
	
	// controllo della porta
	nread = 0;
	while(argv[1][nread] != '\0') {
		if(argv[1][nread] < '0' || argv[1][nread] > '9') {
			printf("[SERVER] : Errore -> Il numero di porta non è un intero\n");
			exit(2);
		}
		nread++;
	}

	port = atoi(argv[1]);
	if(port < 1024 || port > 65535) {
		printf("[SERVER] : Errore -> Il numero di porta deve essere compreso tra 1024 e 65535\n");
		exit(2);
	}
	
	
	// init indirizzo server
	memset((char *) &serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
   serveraddr.sin_port = htons(port);   
   
   printf("[SERVER] : Avviato correttamente.\n");
   
   
   // creazione socket d'ascolto (TCP)
   if((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   	perror("[SERVER] : Errore -> Impossibile creare la socket di ascolto TCP");
   	exit(3);
   }
   
   // set delle opzioni della socket (TCP)
   if(setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
   	perror("[SERVER] : Errore -> Impossibile impostare la socket TCP con le opzioni");
   	exit(3);
   }
   
   // bind della socket (TCP)
   if(bind(listensd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
   	perror("[SERVER] : Errore -> Impossibile effettuare la bind TCP");
   	exit(3);
   }
   
   // creazione coda d'ascolto della socket (TCP)
   if(listen(listensd, 5) < 0) {
   	perror("[SERVER] : Errore -> Impossibile creare la coda di ascolto TCP");
   	exit(3);
   }
   
   
   // creazione socket (UDP)
   if((udpsd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
   	perror("[CLIENT] : Errore -> Impossibile creare la socket UDP");
   	exit(4);
   }
   
   // set delle opzioni della socket (UDP)
   if(setsockopt(udpsd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
   	perror("[SERVER] : Errore -> Impossibile impostare la socket UDP con le opzioni");
   	exit(3);
   }
   
   // bind della socket (UDP)
   if(bind(udpsd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
   	perror("[SERVER] : Errore -> Impossibile effettuare la bind UDP");
   	exit(3);
   }
   
   
   printf("[SERVER] : In ascolto...\n\n");
   
   
   // pulizia e settaggio della maschera dei file descriptor
   FD_ZERO(&rset);
   maxfd = (listensd > udpsd ? listensd : udpsd) + 1;
   
   // ricezioni eventi tramite select
   while(1) {
   	
   	// set e selezione degli eventi in attesa
   	FD_SET(listensd, &rset);
   	FD_SET(udpsd, &rset);
   	
   	if(select(maxfd, &rset, NULL, NULL, NULL) < 0) {
   		if(errno == EINTR) continue;
   		else {
   			perror("[SERVER] : Errore -> Impossibile attendere eventi mediante select");
   			exit(4);
   		}	
   	}
   	
   	// "raccolta" passiva dei processi zombie dei figli TCP
   	// al contrario i figli UDP vengono "raccolti" dal waitpid nel padre del frammento di codice UDP
   	while(waitpid(-1, NULL, WNOHANG) > 0);
   	
   	
   	// gestione richieste UDP
   	if(FD_ISSET(udpsd, &rset)) {
   	
   		printf("[SERVER] : Richiesta UDP ricevuta.\n");
   		
   		// ricezione richiesta
   		len = sizeof(struct sockaddr_in);
   		if(recvfrom(udpsd, &req, sizeof(req), 0, (struct sockaddr *) &clientaddr, &len) < 0) {
   			perror("[SERVER] : Errore -> Impossibile ricevere la richiesta UDP dal client");
   			exit(5); 
   		}
   		
   		pid = fork();
   		switch(pid) {
   			case -1: // errore
   				perror("[SERVER] : Errore -> Impossibile creare il processo figlio");
   				break;
   				
   			case 0: // figlio
   				
   				// redirezione di stdout e stderr per evitare di stampare l'output sul terminale del server
   				if((nread = open("/dev/null", O_WRONLY)) >= 0) {
   					// stdout -> /dev/null
   					// stderr -> /dev/null
   					
   					close(1);
   					close(2);
   					dup(nread);
   					dup(nread);
   					
   					close(nread);
   				}
   				
   				
   				// l'ultimo argomento nella exec deve essere sempre NULL
   				argsExec[0] = req.command;
   				
   				for(i=0; i<req.nArgs; i++)
    					argsExec[i+1] = req.args[i];
					argsExec[req.nArgs+1] = NULL;
					
					   				
   				// esecuzione del comando
   				execvp(req.command, argsExec);
   				perror("[SERVER] : Errore -> Impossibile eseguire il comando inviato");
   				exit(127);		
   				break;
   				
   			default: // padre
   				
   				// attesa della terminazione del figlio per prelevare il codice di ritorno
   				if(waitpid(pid, &status, 0) < 0) {
   					perror("[SERVER] : Errore -> Non è stato possibile leggere il valore di ritorno del comando");
       				nread = -1;
   				}
   				
   				if(WIFEXITED(status)) nread = WEXITSTATUS(status);
   				else if(WIFSIGNALED(status)) nread = 128 + WTERMSIG(status);
   				else nread = -1;
   				
   				printf("[SERVER] : Risultato dell'operazione UDP: %d\n", nread);
   					
   			
   				// invio della risposta al client
   				if(sendto(udpsd, &nread, sizeof(int), 0, (struct sockaddr *) &clientaddr, len) < 0) {
   					perror("[SERVER] : Errore -> Impossibile inviare la risposta UDP al client");
   					exit(6); 
   				}
   				
   				break;
   		}
   		
   	}
   	
   	// gestione richieste TCP
   	if(FD_ISSET(listensd, &rset)) {
   	
   		printf("[SERVER] : Richiesta TCP ricevuta.\n");
   		
   		len = sizeof(struct sockaddr_in);
   		if((connsd = accept(listensd, (struct sockaddr *) &clientaddr, &len)) < 0) {
   			if(errno == EINTR) continue;
   			else {
   				perror("[SERVER] : Errore -> Impossibile accettare la richiesta TCP del client");
   				exit(7);
   			}
   		}
   		
   		
   		// ricezione richiesta
   		if((nread = read(connsd, &req, sizeof(req))) < 0) {
   			perror("[SERVER] : Errore -> Impossibile ricevere la richiesta TCP dal client");
   			continue;
   		}
   		
   		
   		switch(fork()) {
   			case -1: // errore
   				perror("[SERVER] : Errore -> Impossibile creare il processo figlio");
   				break;
   				
   			case 0: // figlio
   				
   				printf("[SERVER] : Eseguo il comando\n");
   				
   				// l'ultimo argomento nella exec deve essere sempre NULL
   				argsExec[0] = req.command;
   				
   				for(i=0; i<req.nArgs; i++)
    					argsExec[i+1] = req.args[i];
					argsExec[req.nArgs+1] = NULL;
   				
   				
   				// redirezione di stdout e stderr
   				close(1);
   				close(2);
   				dup(connsd);
   				dup(connsd);
   				
   				// esecuzione del comando
   				execvp(req.command, argsExec);
   				perror("[SERVER] : Errore -> Impossibile eseguire il comando inviato");   				
   				break;
   				
   			default: // padre
   				close(connsd); 				
   				break;
   		}
   		
   	}
   	
   }
   
   // non raggiungibile
	exit(0);
}

