#define _GUN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MCP.h"

int count_token(char* buf, const char* delim)
{
	if (buf == NULL) {
		return 0;
	}

	int numTokens = 0;

	char *bufCopy = (char *) malloc(sizeof(char) * (strlen(buf) + 1));

	strcpy(bufCopy, buf);

	if (bufCopy[strlen(buf) - 1] == '\n') {
		bufCopy[strlen(buf) - 1] = '\0';
	}

	char *token = strtok(bufCopy, delim);

	while(token != NULL) {
		token = strtok(NULL, delim);
		numTokens++;
	}

	free(bufCopy);

	return numTokens;
}

command_line str_filler (char* buf, const char* delim)
{
	command_line command;

	command.num_token = count_token(buf, delim);
	command.command_list = (char **) malloc(sizeof(char *) * (command.num_token + 1));

	char *bufCopy = (char *) malloc(sizeof(char) * (strlen(buf) + 1));

	strcpy(bufCopy, buf);

	if (bufCopy[strlen(buf) - 1] == '\n') {
		bufCopy[strlen(buf) - 1] = '\0';
	}

	char *token = strtok(bufCopy, delim);

	int i = 0;

	while(token != NULL) {
		command.command_list[i] = (char *) malloc(sizeof(char) * (strlen(token) + 1));
		strcpy(command.command_list[i], token);
		token = strtok(NULL, delim);
		i++;
	}

	command.command_list[i] = NULL;

	free(bufCopy);

	return command;
}

void free_command_line(command_line* command)
{
	for(int i = 0; i < command->num_token; i++) {
		free(command->command_list[i]);
	}

	free(command->command_list);
}

void removeSubstring(char *str, const char *sub) {
    int len = strlen(sub);
    while ((str = strstr(str, sub))) {
        memmove(str, str + len, strlen(str + len) + 1);
    }
}