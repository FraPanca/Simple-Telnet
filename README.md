# Simple-Telnet
Sviluppo di una shell remota.

==== COMPONENTI ====
 - il Client effettua un'operazione e può ricevere il codice di ritorno o l'output del stdout della shell remota
 - il Server come shell remota


==== ISTRUZIONI ====
- Compilazione:
  - $ gcc server.c -o server					            <- directory server
	- $ gcc client_datagram.c -o client_datagram		<- directory client
	- $ gcc client_stream.c -o client_stream			  <- directory client

- Esecuzione
  - $ ./server serverSocketPort
  - $ ./client_datagram serverIpAddress serverPort
  - $./client_stream serverIpAddress serverPort
