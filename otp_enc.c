#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const int BUFFERSIZE = 10000;

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
void connectAndSendData(char* plainTextPath, char* keyPath, char* port);
void stringifyPlainTextAndKey(FILE *plain, FILE *key, char* plainTextString, char* keyString);
void sendToServer(int socketFD, char* string);
char* receiveFromServer(int socketFD, char *string);

int main(int argc, char *argv[])
{

	if (argc < 3) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	connectAndSendData(argv[1], argv[2], argv[3]);

	return 0;
}


void connectAndSendData(char* plainTextPath, char* keyPath, char* port)
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char *clientConfirmationCode = "otp_enc";
	char *serverConfirmationCode = "otp_enc_d";
	char buffer[BUFFERSIZE];
	char getConfirmation[BUFFERSIZE];
	char plainTextString[BUFFERSIZE];
	char keyString[BUFFERSIZE];
	char encryptedText[BUFFERSIZE];

	FILE *plain, *key;

	plain = fopen(plainTextPath, "r");
	key = fopen(keyPath, "r");

	if (plain != NULL && key != NULL)
		stringifyPlainTextAndKey(plain, key, plainTextString, keyString);
																					  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(port); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

																											// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Initiate three-way handshake (sent client code)
	sendToServer(socketFD, clientConfirmationCode);

	// Get return message from server, confirm server code
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

	//getConfirmation = receiveFromServer(socketFD, buffer);
	memset(getConfirmation, '\0', BUFFERSIZE);
	charsRead = recv(socketFD, getConfirmation, sizeof(getConfirmation) - 1, 0);
	if (charsRead < 0) {
		error("ERROR reading data from socket"); fflush(stdout);
	}
	else if (charsRead < sizeof(getConfirmation) - 1) {
		printf("CLIENT: There may be more data from socket.\n"); fflush(stdout);
	}
	printf("Encrypted Text: %s\n", encryptedText); fflush(stdout);

	if (strcmp(getConfirmation, serverConfirmationCode) == 0) {
		printf("CLIENT: Three-way handshake complete. Begin sending data.\n");

		printf("CLIENT sending key:\n %s\n", keyString);
		printf("CLIENT sending plain text:\n %s\n", plainTextString);

		sendToServer(socketFD, keyString);
		sendToServer(socketFD, plainTextString);

		//encryptedText = receiveFromServer(socketFD, buffer);

		memset(encryptedText, '\0', BUFFERSIZE);
		charsRead = recv(socketFD, encryptedText, sizeof(encryptedText) - 1, 0);
		if (charsRead < 0) {
			error("ERROR reading data from socket"); fflush(stdout);
		}
		else if (charsRead < sizeof(encryptedText) - 1) {
			printf("CLIENT: There may be more data from socket.\n"); fflush(stdout);
		}
		printf("Encrypted Text: %s\n", encryptedText); fflush(stdout);
	}
	else {
		printf("CLIENT: Failed to receive proper server-side confirmation code. Exiting.\n");
	}

	close(socketFD); // Close the socket
}


void stringifyPlainTextAndKey(FILE *plain, FILE *key, char* plainTextString, char* keyString)
{
	int plainTextCount = 0;
	int keyCount = 0;
	char c;

	if (plain != NULL) {
		while ((c = fgetc(plain)) != EOF || (c = fgetc(plain)) != '\n')
		{
			if ((c < 65 && c != 32) || c > 90) {
				printf("Plain text file has bad character: %c --> %d.\n", c, c); fflush(stderr);
				exit(1);
			}

			plainTextString[plainTextCount] = c;
			plainTextCount++;
		}
	}

	while ((c = fgetc(key)) != EOF)
	{
		keyString[keyCount] = c;
		keyCount++;
	}

	if (keyCount < plainTextCount) {
		printf("Key is too short for plain text file.\n");
		printf("key: %d\n", keyCount);
		printf("plain: %d\n", plainTextCount);
		exit(1);
	}

	fclose(plain);
	fclose(key);
}


void sendToServer(int socketFD, char *string)
{
	int charsWritten;
	charsWritten = send(socketFD, string, strlen(string), 0); // Write to the server
	if (charsWritten < 0)
		error("CLIENT: ERROR writing to socket");
	else if (charsWritten < strlen(string))
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	else {
		printf("charsWritten: %d\n", charsWritten);
		printf("strlen: %zu\n", strlen(string));
	}
}

char* receiveFromServer(int socketFD, char *string)
{
	int charsRead;
	memset(string, '\0', BUFFERSIZE);
	charsRead = recv(socketFD, string, sizeof(string) - 1, 0);
	if (charsRead < 0) {
		error("ERROR reading data from socket"); fflush(stdout);
	}
	else if (charsRead < sizeof(*string) - 1) {
		printf("CLIENT: There may be more data from socket.\n"); fflush(stdout);
	}

	return string;
}
