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
	char buffer[256];
	char *plainTextString = malloc(sizeof(char) * BUFFERSIZE);
	char *keyString = malloc(sizeof(char) * BUFFERSIZE);

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

	// Get input message from user
	//printf("CLIENT: Enter text to send to the server, and then hit enter: ");
	//memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	//fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	//buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds


	// Initiate three-way handshake (sent client code)
	sendToServer(socketFD, clientConfirmationCode);

	// Get return message from server, confirm server code
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0)
		error("CLIENT: ERROR reading from socket");

	if (strcmp(buffer, serverConfirmationCode) == 0) {
		printf("Three-way handshake complete. Begin sending data.\n");

		printf("CLIENT sending key:\n %s\n", keyString);
		printf("CLIENT sending plain text:\n %s\n", plainTextString);

		sendToServer(socketFD, keyString);
		sendToServer(socketFD, plainTextString);
	}

	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket
}


void stringifyPlainTextAndKey(FILE *plain, FILE *key, char* plainTextString, char* keyString)
{
	int plainTextCount = 0;
	int keyCount = 0;
	char c;

	while ((c = fgetc(plain)) != '\n')
	{
		if ((c < 65 && c != 32) || c > 90) {
			printf("Plain text file has bad character: %c --> %d.\n", c, c); fflush(stderr);
			exit(1);
		}

		plainTextString[plainTextCount] = c;
		plainTextCount++;
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
