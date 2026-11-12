#ifndef BANKER_H
#define BANKER_H

#include <stdio.h>

typedef struct {
  int customers;   // number of customers (rows)
  int resources;   // number of resource types (columns)
  int *available;  // [resources] free instances of each resource
  int *maximum;    // [customers][resources] maximum demand of each customer
  int *allocation; // [customers][resources] resources each customer holds now
  int *need;       // [customers][resources] maximum - allocation} Bank;
} Bank;

int bank_init(Bank *bank, int customers, int resources, const int *total,
              const int *maximum);

void bank_free(Bank *bank);

// returns = 1 (is safe) | = 0 (is not safe).
int bank_is_safe(const Bank *bank);

typedef enum {
  REQ_GRANTED,
  REQ_EXCEEDS_MAXIMUM,
  REQ_NOT_ENOUGH,
  REQ_UNSAFE,
} RequestStatus;

RequestStatus bank_request(Bank *bank, int customer, const int *request);

typedef enum {
  REL_OK,
  REL_EXCEEDS_ALLOCATION,
} ReleaseStatus;

ReleaseStatus bank_release(Bank *bank, int customer, const int *release);

#endif
