#include "parser.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// true if the line has only whitespace (blank lines are skipped)
static int is_blank(const char *line) {
  for (; *line; line++) {
    if (!isspace((unsigned char)*line))
      return 0;
  }
  return 1;
}

// true if fgets cut the line: the buffer is full and there is no '\n' at the
// end
static int line_too_long(const char *line, size_t size) {
  size_t len = strlen(line);
  return len == size - 1 && line[len - 1] != '\n';
}

// reads "3, 2,2" into out (out may be NULL just to count and validate).
// every value must be a number >= 0 that fits in an int.
// returns how many values the line has | -1 = bad line.
static int parse_row(const char *line, int *out) {
  const char *p = line;
  int count = 0;

  while (1) {
    char *end; // strtol stops here
    errno = 0;
    long number = strtol(p, &end, 10);
    if (end == p) // no digits: text, empty value ("1,,2") or trailing comma
      return -1;
    if (errno == ERANGE || number < 0 || number > INT_MAX)
      return -1;
    if (out)
      out[count] = (int)number;
    count++;

    p = end;
    while (*p == ' ' || *p == '\t') // spaces after the number are fine
      p++;
    if (*p == ',') {
      p++; // next value
      continue;
    }
    if (*p == '\0' || *p == '\n' || *p == '\r') // end of line
      return count;
    return -1; // junk after the number, like "3x" or "3 4"
  }
}

int parse_customers(const char *path, int *customers, int *resources,
                    int **maximum) {
  FILE *file = fopen(path, "r");
  if (!file)
    return -1;

  char line[256];
  int rows = 0;
  int cols = 0;

  // pass 1: validate every line. 1st line sets how many resources there are,
  // the other lines must have the same amount
  while (fgets(line, sizeof line, file)) {
    if (is_blank(line)) // skip blank lines
      continue;
    if (line_too_long(line, sizeof line)) {
      fclose(file);
      return -2;
    }
    int count = parse_row(line, NULL);
    if (count < 0 || (rows > 0 && count != cols)) {
      fclose(file);
      return -2;
    }
    if (rows == 0)
      cols = count;
    rows++;
  }

  if (rows == 0) { // empty file
    fclose(file);
    return -2;
  }

  int *matrix = malloc((size_t)rows * cols * sizeof(int));
  if (!matrix) {
    fclose(file);
    return -1;
  }

  // pass 2: the file is valid now, just read the numbers into the flat matrix
  rewind(file);
  int i = 0;
  while (fgets(line, sizeof line, file)) {
    if (is_blank(line))
      continue;
    parse_row(line, &matrix[i * cols]);
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
  } while (is_blank(line));

  if (line_too_long(line, sizeof line))
    return -1;

  const char *p = line;
  while (isspace((unsigned char)*p)) // skip spaces before the command
    p++;

  // "*" has no customer and no values, nothing else may follow it
  if (*p == '*') {
    if (!is_blank(p + 1))
      return -1;
    cmd->type = CMD_PRINT;
    return 1;
  }

  // "RQ 0 0 2 1" -> op = RQ, customer = 0, then the values
  char op[4]; // 3 chars so "RQ0" or "RQX" is read whole and rejected
  int used;   // chars read so far, to continue from there
  if (sscanf(p, "%3s%n", op, &used) != 1)
    return -1;
  p += used;

  if (strcmp(op, "RQ") == 0)
    cmd->type = CMD_REQUEST;
  else if (strcmp(op, "RL") == 0)
    cmd->type = CMD_RELEASE;
  else
    return -1;

  char *end; // strtol stops here
  errno = 0;
  long customer = strtol(p, &end, 10);
  if (end == p || errno == ERANGE)
    return -1;
  // customer index must exist, or the bank would read outside its matrices
  if (customer < 0 || customer >= customers)
    return -1;
  cmd->customer = (int)customer;
  p = end;

  // exactly one value per resource, each a number >= 0 that fits in an int
  for (int j = 0; j < resources; j++) {
    errno = 0;
    long number = strtol(p, &end, 10);
    if (end == p || errno == ERANGE || number < 0 || number > INT_MAX)
      return -1;
    values[j] = (int)number;
    p = end;
  }

  // nothing but spaces may follow the last value
  if (!is_blank(p))
    return -1;
  return 1;
}
