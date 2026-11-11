#include "banker.h"
#include <stddef.h>
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
