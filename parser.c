#include "parser.h"
#include <stdlib.h>
#include <string.h>

int parse_customers(const char *path, int *customers, int *resources,
                    int **maximum) {
  FILE *file = fopen(path, "r");
  if (!file)
    return -1;

  char line[256];
  int rows = 0;
  int cols = 0;

  // pass 1: count customers (lines) and resources (commas in 1st line + 1)
  while (fgets(line, sizeof line, file)) {
    if (line[0] == '\n') // skip blank lines
      continue;
    if (rows == 0) {
      cols = 1;
      for (char *p = line; *p; p++) {
        if (*p == ',')
          cols++;
      }
    }
    rows++;
  }

  if (rows == 0) { // empty file
    fclose(file);
    return -1;
  }

  int *matrix = malloc((size_t)rows * cols * sizeof(int));
  if (!matrix) {
    fclose(file);
    return -1;
  }

  // pass 2: read the numbers into the flat matrix
  rewind(file);
  int i = 0;
  while (fgets(line, sizeof line, file)) {
    if (line[0] == '\n')
      continue;
    char *p = line;
    for (int j = 0; j < cols; j++) {
      matrix[i * cols + j] = (int)strtol(p, &p, 10); // p moves past the number
      if (*p == ',')
        p++; // skip the comma
    }
    i++;
  }
  fclose(file);

  *customers = rows;
  *resources = cols;
  *maximum = matrix;
  return 0;
}

int parse_command(FILE *file, int customers, int resources, Command *cmd,
                  int *values) {
  char line[256];

  // read the next non-blank line
  do {
    if (!fgets(line, sizeof line, file))
      return 0;
  } while (line[0] == '\n');

  // "*" has no customer and no values
  if (line[0] == '*') {
    cmd->type = CMD_PRINT;
    return 1;
  }

  // "RQ 0 0 2 1" -> op = RQ, customer = 0, then the values
  char op[3];
  int customer;
  int used; // chars read so far, to continue from there
  if (sscanf(line, "%2s %d%n", op, &customer, &used) != 2)
    return -1;

  // customer index must exist, or the bank would read outside its matrices
  if (customer < 0 || customer >= customers)
    return -1;

  if (strcmp(op, "RQ") == 0)
    cmd->type = CMD_REQUEST;
  else if (strcmp(op, "RL") == 0)
    cmd->type = CMD_RELEASE;
  else
    return -1;
  cmd->customer = customer;

  char *p = line + used;
  for (int j = 0; j < resources; j++) {
    int n;
    if (sscanf(p, "%d%n", &values[j], &n) != 1)
      return -1;
    p += n;
  }
  return 1;
}
