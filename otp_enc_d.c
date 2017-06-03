#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

const int BUFFERSIZE = 10000;

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t game_thread, time_thread;
int game_return, time_return;

struct socketThread {
	pthread_t thread;
	int inUse;
};

struct socketThread socketThreads[5];

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void listenAndAcceptConnections(char* port);
int verifyClient(int fd);
void sendToClient(int socketFD, char *string);
char* receiveFromClient(int socketFD, char *string);
char* encryptText(char *text, char *key);

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
	char key [BUFFERSIZE];
	char plainText[BUFFERSIZE];
	struct sockaddr_in serverAddress, clientAddress;
	int threadCount = 0;

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

	while (1) {
		//getAvailableThread();
		int i;
		for (i = 0; i < 5; ++i) {
			if (!socketThreads[i].inUse) {
				pthread_mutex_lock(&myMutex);
				socketThreads[i].inUse = 1;
				game_return = pthread_create(&socketThreads[i].thread, NULL, acceptConnection, &listenSocketFD);
			}
		}
	}
	close(listenSocketFD); // Close the listening socket
}


void *acceptConnection(int listenSocketFD)
{
	struct sockaddr_in clientAddress;
	int establishedConnectionFD, portNumber, charsRead, packetSize;
	socklen_t sizeOfClientInfo;
	char *clientConfirmationCode = "otp_enc";
	char *serverConfirmationCode = "otp_enc_d";
	char buffer[BUFFERSIZE];
	char key [BUFFERSIZE];
	char plainText[BUFFERSIZE];
	// Get the size of the address for the client that will connect
	sizeOfClientInfo = sizeof(clientAddress);
	 // Accept a connection, blocking if one is not available until one connects
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

	if (establishedConnectionFD < 0)
		error("ERROR on accept");

	// Get the message from the client and display it
	memset(buffer, '\0', 256);
	//charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
	charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
	if (charsRead < 0) {
		error("ERROR reading data from socket"); fflush(stdout);
	}
	else if (charsRead < sizeof(buffer) - 1) {
		printf("SERVER: There may be more data from socket.\n"); fflush(stdout);
	}

	if (charsRead < 0)
		error("ERROR reading from socket");
	else if (strcmp(buffer, clientConfirmationCode) == 0) {
		printf("SERVER: client is verified: \"%s\".\n", buffer);

		sendToClient(establishedConnectionFD, serverConfirmationCode);

		//key = receiveFromClient(establishedConnectionFD, buffer);
		memset(key, '\0', BUFFERSIZE);
		charsRead = recv(establishedConnectionFD, key, sizeof(key) - 1, 0);
		if (charsRead < 0) {
			error("ERROR reading data from socket"); fflush(stdout);
		}
		else if (charsRead < sizeof(key) - 1) {
			printf("SERVER: There may be more data from socket.\n"); fflush(stdout);
		}

		printf("SERVER received key: %s\n", key); //fflush(stdout);

		//plainText = receiveFromClient(establishedConnectionFD, buffer);
		charsRead = recv(establishedConnectionFD, plainText, sizeof(plainText) - 1, 0);
		if (charsRead < 0) {
			error("ERROR reading data from socket"); fflush(stdout);
		}
		else if (charsRead < sizeof(plainText) - 1) {
			printf("SERVER: There may be more data from socket.\n"); fflush(stdout);
		}

		printf("SERVER received plain text: %s\n", plainText); //fflush(stdout);

		char *encryptedText = encryptText(plainText, key);
		sendToClient(establishedConnectionFD, encryptedText);
	}
	else {
		printf("SERVER: client is not verified: %s . Closing Connection.\n", buffer);
	}

	close(establishedConnectionFD); // Close the existing socket which is connected to the client
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
		error("SERVER: ERROR writing to socket");
	else if (charsWritten < strlen(string))
		printf("SERVER: WARNING: Not all data written to socket!\n");
	else {
		printf("SERVER charsWritten: %d\n", charsWritten); fflush(stdout);
		printf("SERVER strlen: %zu\n", strlen(string)); fflush(stdout);
	}
}

char* receiveFromClient(int socketFD, char *string)
{
	int charsRead;
	memset(string, '\0', BUFFERSIZE);
	charsRead = recv(socketFD, string, sizeof(string) - 1, 0);
	if (charsRead < 0) {
		error("ERROR reading data from socket"); fflush(stdout);
	}
	else if (charsRead < sizeof(*string) - 1) {
		printf("SERVER: There may be more data from socket.\n"); fflush(stdout);
	}

	return string;
}

char* encryptText(char *text, char *key)
{
	int charCount = 0;
	char textChar, keyChar;
	char *encryptedText = malloc(sizeof(char) * BUFFERSIZE);
	memset(encryptedText, '\0', 20);

	while (text[charCount] != '\n') {
		textChar = text[charCount];
		if (textChar != 32)
			textChar -= 65;
		else
			textChar -= 7;

		keyChar = key[charCount];
		if (keyChar != 32)
			keyChar -= 65;
		else
			keyChar -= 7;

		textChar += keyChar;
		textChar %= 27;

		if (textChar == 27)
			textChar = 32;
		else
			textChar += 65;

		encryptedText[charCount++] = textChar;
	}

	return encryptedText;
}
