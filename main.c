#include "banker.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

// prints "a b c" (no trailing space)
static void print_values(FILE *out, const int *values, int n) {
  for (int j = 0; j < n; j++)
    fprintf(out, j == 0 ? "%d" : " %d", values[j]);
}

int main(int argc, char *argv[]) {
  // customer.txt first: it tells how many resources the arguments must have
  int customers, resources;
  int *maximum;
  if (parse_customers("customer.txt", &customers, &resources, &maximum) != 0) {
    fprintf(stderr, "error: cannot read customer.txt\n");
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
  for (int j = 0; j < resources; j++)
    total[j] = atoi(argv[j + 1]);

  Bank bank;
  if (!total || !values ||
      bank_init(&bank, customers, resources, total, maximum) != 0) {
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

  // validate every command before running any (nothing is written on error)
  Command cmd;
  int status;
  int number = 0; // which command we are checking, for the error message
  while ((status = parse_command(commands, customers, resources, &cmd,
                                 values)) == 1)
    number++;
  if (status < 0) {
    fprintf(stderr, "error: invalid command %d in commands.txt\n", number + 1);
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

    } else { // CMD_RELEASE
      if (bank_release(&bank, cmd.customer, values) == REL_OK) {
        fprintf(out, "Release from customer %d the resources ", cmd.customer);
        print_values(out, values, resources);
        fprintf(out, "\n");
      } else {
        fprintf(out, "The customer %d release ", cmd.customer);
        print_values(out, values, resources);
        fprintf(out, " was denied because exceed its allocation\n");
      }
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
