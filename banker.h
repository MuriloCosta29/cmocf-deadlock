#ifndef BANKER_H
#define BANKER_H

typedef struct {
  int customers;   // Clients quantitative
  int resources;   // Amount of resources we have.
  int *available;  // Disponible amount of every type
  int *maximum;    // Limit of every client
  int *allocation; // Resources that each client has already recieved.
  int *need;       // Resources that each client might need
} Bank;

#endif
