#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

const int BUFFERSIZE = 10000;

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void listenAndAcceptConnections(char* port);
int verifyClient(int fd);
void sendToClient(int socketFD, char *string);


int main(int argc, char *argv[])
{

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	listenAndAcceptConnections(argv[1]);
	
	return 0; 
}


void listenAndAcceptConnections(char* port)
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, packetSize;
	socklen_t sizeOfClientInfo;
	char *clientConfirmationCode = "otp_enc";
	char *serverConfirmationCode = "otp_enc_d";
	char buffer[BUFFERSIZE];
	char key[BUFFERSIZE];
	char plainText[BUFFERSIZE];
	struct sockaddr_in serverAddress, clientAddress;

	memset(key, '\0', BUFFERSIZE);
	memset(plainText, '\0', BUFFERSIZE);
																			 // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(port); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

												// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

							   // Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

	if (establishedConnectionFD < 0) 
		error("ERROR on accept");

	// Get the message from the client and display it
	memset(buffer, '\0', 256);
	charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket

	if (charsRead < 0) 
		error("ERROR reading from socket");
	else if (strcmp(buffer, clientConfirmationCode) == 0) {
		printf("SERVER: client is verified: \"%s\".\n", buffer);

		sendToClient(establishedConnectionFD, serverConfirmationCode);

		printf("can you see me?\n");

		packetSize = sizeof(key) -1 ;

		while (charsRead != packetSize || charsRead != 0) {
			printf("charsRead: %d\n", charsRead); fflush(stdout);
			charsRead = recv(establishedConnectionFD, key, packetSize, 0);
			if (charsRead < 0) {
				error("ERROR reading plain text from socket");
				break;
			}
			packetSize -= charsRead;
		}
 // Read the client's message from the socket

		printf("SERVER received key: %s\n", key);

		packetSize = sizeof(plainText) - 1;

		while (charsRead != packetSize || charsRead != 0) {
			printf("charsRead: %s\n", charsRead); fflush(stdout);
			charsRead = recv(establishedConnectionFD, key, packetSize, 0);
			if (charsRead < 0) {
				error("ERROR reading plain text from socket");
				break;
			}
			packetSize -= charsRead;
		}

		printf("SERVER received plain text: %s\n", plainText);
	}
	else {
		printf("SERVER: client is not verified: %s . Closing Connection.\n", buffer);
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
		close(listenSocketFD); // Close the listening socket
	}
	

}


int verifyClient(int fd)
{
	char path[1024];
	char result[1024];

	/* Read out the link to our file descriptor. */
	sprintf(path, "/proc/self/fd/%d", fd);
	memset(result, 0, sizeof(result));
	readlink(path, result, sizeof(result) - 1);

	printf("\nclient: %s\n", result);

	if (result == "otp_enc")
		return 1;
	else
		return 0;
}


void sendToClient(int socketFD, char *string)
{
	int charsWritten;
	charsWritten = send(socketFD, string, strlen(string), 0); // Write to the server
	if (charsWritten < 0)
		error("CLIENT: ERROR writing to socket");
	else if (charsWritten < strlen(string))
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	else {
		printf("charsWritten: %d\n", charsWritten);
		printf("strlen: %d\n", strlen(string));
	}
}