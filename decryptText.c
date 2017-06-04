#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

char* decryptText(char *text, char *key);

int main()
{

	char* decryptedText = decryptText("DQNVZ\n", "XMCKL");

	printf("Decrypted Text: %s\n", decryptedText);
}

char* decryptText(char *text, char *key)
{
	int charCount = 0;
	char textChar, keyChar;
	char *decryptedText = malloc(sizeof(char) * 20);
	memset(decryptedText, '\0', 20);

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

		textChar -= keyChar;
		if (textChar < 0)
			textChar += 27;
		textChar %= 27;

		if (textChar == 27)
			textChar = 32;
		else
			textChar += 65;
		printf("text char: %c,\n", textChar);
		decryptedText[charCount++] = textChar;
	}

	return decryptedText;
}
