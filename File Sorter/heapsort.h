#ifndef _HEAPSORT_H
#define _HEAPSORT_H
#include"customer.h"

customer *info(FILE *file, int location, customer *c);

void order(FILE *file, int index, customer *ind, int largest, customer *lar);

void heapify(FILE *file, int index, int size, customer *lar, customer *node);

int heapsort(const char *filename);

#endif
