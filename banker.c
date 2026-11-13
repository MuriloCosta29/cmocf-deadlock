#include "banker.h"
#include <stdlib.h>
#include <string.h>

int bank_init(Bank *bank, int customers, int resources, const int *total,
              const int *maximum) {

  // how many ints each matrix has (row * columns). -> Count ints
  size_t cells = (size_t)customers * resources;

  // Build memo in HEAP
  bank->customers = customers; // come from .txt, don't need mem alloc
  bank->resources = resources;
  bank->available = malloc(resources * sizeof(int));
  bank->maximum = malloc(cells * sizeof(int));
  bank->allocation = calloc(cells, sizeof(int));
  bank->need = malloc(cells * sizeof(int));

  // Free memo if fail
  if (!bank->available || !bank->maximum || !bank->allocation || !bank->need) {
    bank_free(bank);
    return -1;
  }

  // Fill arrays
  memcpy(bank->available, total, resources * sizeof(int));
  memcpy(bank->maximum, maximum, cells * sizeof(int));
  memcpy(bank->need, maximum, cells * sizeof(int));
  return 0;
}

void bank_free(Bank *bank) {
  free(bank->available);
  free(bank->maximum);
  free(bank->allocation);
  free(bank->need);
}

int bank_is_safe(const Bank *bank) {
  int *work = malloc(bank->resources * sizeof(int));
  int *finish = calloc(bank->customers, sizeof(int));
  if (!work || !finish) {
    free(work);
    free(finish);
    return 0;
  }
  memcpy(work, bank->available, bank->resources * sizeof(int));

  // search loop
  int progress = 1;
  while (progress) {
    progress = 0;
    for (int i = 0; i < bank->customers; i++) {
      if (finish[i])
        continue;

      int can_finish = 1;
      for (int j = 0; j < bank->resources; j++) {
        if (bank->need[i * bank->resources + j] > work[j]) {
          can_finish = 0;
          break;
        }
      }

      if (can_finish) {
        for (int j = 0; j < bank->resources; j++)
          work[j] += bank->allocation[i * bank->resources + j];
        finish[i] = 1;
        progress = 1;
      }
    }
  }

  // verification
  int safe = 1;
  for (int i = 0; i < bank->customers; i++) {
    if (!finish[i]) {
      safe = 0;
      break;
    }
  }
  free(work);
  free(finish);
  return safe;
}

RequestStatus bank_request(Bank *bank, int customer, const int *request) {
  int row = customer * bank->resources; // where this customer's row starts

  // check 1: request can't pass what the customer still needs
  for (int j = 0; j < bank->resources; j++) {
    if (request[j] > bank->need[row + j])
      return REQ_EXCEEDS_MAXIMUM;
  }

  // check 2: bank must have enough free resources
  for (int j = 0; j < bank->resources; j++) {
    if (request[j] > bank->available[j])
      return REQ_NOT_ENOUGH;
  }

  // pretend to grant: change the real bank, then test it
  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] -= request[j];
    bank->allocation[row + j] += request[j];
    bank->need[row + j] -= request[j];
  }

  // safe -> keep the change
  if (bank_is_safe(bank))
    return REQ_GRANTED;

  // unsafe -> rollback: undo exactly what we just did
  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] += request[j];
    bank->allocation[row + j] -= request[j];
    bank->need[row + j] += request[j];
  }
  return REQ_UNSAFE;
}

ReleaseStatus bank_release(Bank *bank, int customer, const int *release) {
  int row = customer * bank->resources; // where this customer's row starts

  // can't give back more than the customer holds
  for (int j = 0; j < bank->resources; j++) {
    if (release[j] > bank->allocation[row + j])
      return REL_EXCEEDS_ALLOCATION;
  }

  // release never makes the state unsafe: no safety check, no rollback
  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] += release[j];
    bank->allocation[row + j] -= release[j];
    bank->need[row + j] += release[j];
  }
  return REL_OK;
}

// prints each value followed by a space, then pads with spaces up to width
static void print_row(FILE *out, const int *row, int n, int width) {
  int written = 0;
  for (int j = 0; j < n; j++)
    written += fprintf(out, "%d ", row[j]);
  if (written < width)
    fprintf(out, "%*s", width - written, "");
}

void bank_print(const Bank *bank, FILE *out) {
  fprintf(out, "MAXIMUM   |   ALLOCATION  |   NEED\n");

  for (int i = 0; i < bank->customers; i++) {
    int row = i * bank->resources; // where customer i's row starts
    // &matrix[row] = pointer to the start of this customer's row
    print_row(out, &bank->maximum[row], bank->resources, 10);
    fprintf(out, "|   ");
    print_row(out, &bank->allocation[row], bank->resources, 12);
    fprintf(out, "|   ");
    print_row(out, &bank->need[row], bank->resources, 0);
    fprintf(out, "\n");
  }

  fprintf(out, "AVAILABLE ");
  print_row(out, bank->available, bank->resources, 0);
  fprintf(out, "\n");
}
