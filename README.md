# Simple-Telnet

## Italiano
Sviluppo di una shell remota.

### Componenti
- Il Client effettua un'operazione e può ricevere il codice di ritorno (client datagram) o l'output del stdout della shell remota (client stream)
- Il Server funge da shell remota

### Istruzioni
- Compilazione:
  - $ gcc server.c -o server                              <- directory server
  - $ gcc client_datagram.c -o client_datagram            <- directory client
  - $ gcc client_stream.c -o client_stream                <- directory client

- Esecuzione:
  - $ ./server serverSocketPort
  - $ ./client_datagram serverIpAddress serverPort
  - $ ./client_stream serverIpAddress serverPort

---

## English
Development of a remote shell.

### Components
- The Client performs operations and can receive the return code (datagram client) or the stdout output from the remote shell (stream client)
- The Server acts as a remote shell

### Instructions
- Compile:
  - $ gcc server.c -o server                              <- server directory
  - $ gcc client_datagram.c -o client_datagram            <- client directory
  - $ gcc client_stream.c -o client_stream                <- client directory

- Run:
  - $ ./server serverSocketPort
  - $ ./client_datagram serverIpAddress serverPort
  - $ ./client_stream serverIpAddress serverPort
