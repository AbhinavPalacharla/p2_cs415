#ifndef MCP_H_
#define MCP_H_

typedef struct
{
    char **command_list;
    int num_token;
} command_line;

int count_token(char *buf, const char *delim);

command_line str_filler(char *buf, const char *delim);

void free_command_line(command_line *cmd_line);

void removeSubstring(char *str, const char *sub);

#endif