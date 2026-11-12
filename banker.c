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

  for (int j = 0; j < bank->resources; j++) {
    if (request[j] > bank->need[row + j])
      return REQ_EXCEEDS_MAXIMUM;
  }

  for (int j = 0; j < bank->resources; j++) {
    if (request[j] > bank->available[j])
      return REQ_NOT_ENOUGH;
  }

  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] -= request[j];
    bank->allocation[row + j] += request[j];
    bank->need[row + j] -= request[j];
  }

  if (bank_is_safe(bank))
    return REQ_GRANTED;

  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] += request[j];
    bank->allocation[row + j] -= request[j];
    bank->need[row + j] += request[j];
  }
  return REQ_UNSAFE;
}

ReleaseStatus bank_release(Bank *bank, int customer, const int *release) {
  int row = customer * bank->resources;

  for (int j = 0; j < bank->resources; j++) {
    if (release[j] > bank->allocation[row + j])
      return REL_EXCEEDS_ALLOCATION;
  }

  for (int j = 0; j < bank->resources; j++) {
    bank->available[j] += release[j];
    bank->allocation[row + j] -= release[j];
    bank->need[row + j] += release[j];
  }
  return REL_OK;
}
