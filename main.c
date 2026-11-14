#include "banker.h"
#include "parser.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// converts a command-line argument to a number of instances (>= 0).
// returns 0 = ok | -1 = not a valid number
static int parse_instances(const char *arg, int *value) {
  char *end; // strtol stops here: if it isn't the end of arg, there was junk
  errno = 0;
  long number = strtol(arg, &end, 10);
  if (end == arg || *end != '\0') // no digits at all, or "12abc"
    return -1;
  if (errno == ERANGE || number < 0 || number > INT_MAX) // too big or negative
    return -1;
  *value = (int)number;
  return 0;
}

// prints "a b c" (no trailing space)
static void print_values(FILE *out, const int *values, int n) {
  for (int j = 0; j < n; j++)
    fprintf(out, j == 0 ? "%d" : " %d", values[j]);
}

// checks all commands before any of them is run: the format of each line
// (parser) and releases bigger than what the customer holds. To know that, the
// commands are tried on a throwaway bank, so the real one stays untouched.
// prints the error and returns 0 = ok | -1 = error
static int check_commands(FILE *file, int customers, int resources,
                          const int *total, const int *maximum, int *values) {
  Bank trial;
  if (bank_init(&trial, customers, resources, total, maximum) != 0) {
    fprintf(stderr, "error: out of memory\n");
    return -1;
  }

  Command cmd;
  int status;
  int number = 0; // which command we are checking, for the error message
  while ((status = parse_command(file, customers, resources, &cmd, values)) ==
         1) {
    number++;
    if (cmd.type == CMD_REQUEST) {
      bank_request(&trial, cmd.customer, values);
    } else if (cmd.type == CMD_RELEASE &&
               bank_release(&trial, cmd.customer, values) != REL_OK) {
      fprintf(stderr,
              "error: invalid command %d in commands.txt: customer %d "
              "releases more than it holds\n",
              number, cmd.customer);
      bank_free(&trial);
      return -1;
    }
  }
  bank_free(&trial);

  if (status < 0) {
    fprintf(stderr, "error: invalid command %d in commands.txt\n", number + 1);
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // customer.txt first: it tells how many resources the arguments must have
  int customers, resources;
  int *maximum;
  int load = parse_customers("customer.txt", &customers, &resources, &maximum);
  if (load == -1) {
    fprintf(stderr, "error: cannot open customer.txt\n");
    return 1;
  }
  if (load == -2) {
    fprintf(stderr, "error: customer.txt has an invalid format: one customer "
                    "per line, same amount of numbers (>= 0) separated by "
                    "commas\n");
    return 1;
  }

  if (argc - 1 != resources) {
    fprintf(stderr, "error: expected %d resource arguments, got %d\n",
            resources, argc - 1);
    free(maximum);
    return 1;
  }

  // arguments = total instances of each resource (./banker 10 5 7)
  int *total = malloc(resources * sizeof(int));
  int *values =
      malloc(resources * sizeof(int)); // values of the current command
  if (!total || !values) {
    fprintf(stderr, "error: out of memory\n");
    free(maximum);
    free(total);
    free(values);
    return 1;
  }

  // every argument must be a valid number
  for (int j = 0; j < resources; j++) {
    if (parse_instances(argv[j + 1], &total[j]) != 0) {
      fprintf(stderr, "error: '%s' is not a valid number of instances\n",
              argv[j + 1]);
      free(maximum);
      free(total);
      free(values);
      return 1;
    }
  }

  Bank bank;
  if (bank_init(&bank, customers, resources, total, maximum) != 0) {
    fprintf(stderr, "error: out of memory\n");
    free(maximum);
    free(total);
    free(values);
    return 1;
  }

  FILE *commands = fopen("commands.txt", "r");
  if (!commands) {
    fprintf(stderr, "error: cannot open commands.txt\n");
    bank_free(&bank);
    free(maximum);
    free(total);
    free(values);
    return 1;
  }

  // check every command before running any (nothing is written on error)
  Command cmd;
  if (check_commands(commands, customers, resources, total, maximum, values) !=
      0) {
    fclose(commands);
    bank_free(&bank);
    free(maximum);
    free(total);
    free(values);
    return 1;
  }

  // all commands are valid: read the file again and run them
  rewind(commands);
  FILE *out = fopen("result.txt", "w");
  if (!out) {
    fprintf(stderr, "error: cannot open result.txt\n");
    fclose(commands);
    bank_free(&bank);
    free(maximum);
    free(total);
    free(values);
    return 1;
  }

  // one command per iteration, in file order
  while (parse_command(commands, customers, resources, &cmd, values) == 1) {
    if (cmd.type == CMD_PRINT) {
      bank_print(&bank, out);

    } else if (cmd.type == CMD_REQUEST) {
      RequestStatus result = bank_request(&bank, cmd.customer, values);
      if (result == REQ_GRANTED) {
        fprintf(out, "Allocate to customer %d the resources ", cmd.customer);
        print_values(out, values, resources);
        fprintf(out, "\n");
      } else if (result == REQ_NOT_ENOUGH) {
        fprintf(out, "The resources ");
        print_values(out, bank.available, resources);
        fprintf(out, " are not enough to customer %d request ", cmd.customer);
        print_values(out, values, resources);
        fprintf(out, " \n"); // the expected output has a space before the \n
      } else {
        fprintf(out, "The customer %d request ", cmd.customer);
        print_values(out, values, resources);
        if (result == REQ_UNSAFE)
          fprintf(out, " was denied because result in an unsafe state\n");
        else
          fprintf(out, " was denied because exceed its maximum allocation\n");
      }

    } else { // CMD_RELEASE (check_commands already made sure it works)
      bank_release(&bank, cmd.customer, values);
      fprintf(out, "Release from customer %d the resources ", cmd.customer);
      print_values(out, values, resources);
      fprintf(out, "\n");
    }
  }

  fclose(commands);
  fclose(out);
  bank_free(&bank);
  free(maximum);
  free(total);
  free(values);
  return 0;
}
