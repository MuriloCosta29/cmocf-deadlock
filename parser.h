#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef enum { CMD_REQUEST, CMD_RELEASE, CMD_PRINT } CommandType;

typedef struct {
  CommandType type;
  int customer; // not used by CMD_PRINT
} Command;

// reads customer.txt: one customer per line, values separated by commas.
// customers = number of lines, resources = values in the first line.
// *maximum is allocated here (flat [customers][resources]), caller frees it.
// returns 0 = ok | -1 = error.
int parse_customers(const char *path, int *customers, int *resources,
                    int **maximum);

// reads the next line of commands.txt into cmd and values[resources].
// the customer must be in [0, customers).
// returns 1 = got a command | 0 = end of file | -1 = bad line.
int parse_command(FILE *file, int customers, int resources, Command *cmd,
                  int *values);

#endif
