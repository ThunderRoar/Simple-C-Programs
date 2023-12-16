#ifndef _CUSTOMER_H
#define _CUSTOMER_H

#define CUSTOMER_NAME_MAX 44

// Total bytes of this struct: 48
typedef struct customer {
  char name[CUSTOMER_NAME_MAX];   // NUL-padded but not always NUL-terminated!
  unsigned loyalty;               // loyalty points
} customer;

#endif // _CUSTOMER_H
