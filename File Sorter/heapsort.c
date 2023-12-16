#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heapsort.h"
#include "customer.h"

// Retrieve the information from the file
customer *info(FILE *file, int location, customer *c) {
    if (file == NULL) return NULL;
    if (fseek(file, location * sizeof(customer), SEEK_SET) != 0) return NULL;
    if (fread(c, sizeof(customer), 1, file) != 1) return NULL;
    if (fseek(file, 0L, SEEK_SET) != 0) return NULL;
    return c;
}

// Swap the two objects in the file
void order(FILE *file, int index, customer *ind, int largest, customer *lar)
{
    if (file == NULL) return;
    // Find the first location which is index or root and replace with largest data in file
    if (fseek(file, index * sizeof(customer), SEEK_SET) != 0) {
        fprintf(stderr, "Not able to move the pointer in the file...\n");
        return;
    }
    // Writing the info onto the file
    if (fwrite(lar, sizeof(customer), 1, file) != 1) {
        fprintf(stderr, "Error writing into ofile 1...\n");
        return;
    }

    // Find the second location which is of the largest one child and replace with ind data in file
    if (fseek(file, largest * sizeof(customer), SEEK_SET) != 0) {
        fprintf(stderr, "Not able to move the pointer in the file...\n");
        return;
    }
    // Writing the info onto the file
    if (fwrite(ind, sizeof(customer), 1, file) != 1) {
        fprintf(stderr, "Error writing into ofile 2...\n");
        return;
    }

    return;
}

void heapify(FILE *file, int index, int size, customer *lar, customer *node) {
    // Iterate over the left nodes
    for (int i = (2 * index + 1); i < size; i = (2 * index + 1)) {
        int largest = i;   // let left node be the largest
        int right = i + 1; // right node

        // compare the left node with the right node to check the largest
        if (right < size) {
            lar = info(file, largest, lar); // contains left node info
            node = info(file, right, node); // contains right node info
            if (lar->loyalty < node->loyalty) {
                largest = right;
            }
            else if (lar->loyalty == node->loyalty && strncmp(lar->name, node->name, CUSTOMER_NAME_MAX) < 0) {
                largest = right;
            }
        }

        node = info(file, index, node); // contains the parent node info
        lar = info(file, largest, lar); // info on the largest node

        // compare the largest child node with the parent node
        if (node->loyalty < lar->loyalty) {
            order(file, index, node, largest, lar);
        }
        else if (lar->loyalty == node->loyalty && strncmp(node->name, lar->name, CUSTOMER_NAME_MAX) < 0) {
            order(file, index, node, largest, lar);
        }
        else {
            break;
        }
        // Sets the index to the largest node for the next iteration
        index = largest;
    }
}

int heapsort(const char *filename) {
    FILE *ifile = fopen(filename, "rb+");
    if (ifile == NULL) {
        fprintf(stderr, "Error opening source file...\n");
        return 0;
    }

    if (fseek(ifile, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Not able to move the pointer in the file...\n");
        return 0;
    }
    // Number of customer struct in file
    int size = ftell(ifile) / sizeof(customer);

    customer *lar = (customer *)malloc(sizeof(customer));
    if (lar == NULL) return 0;
    customer *node = (customer *)malloc(sizeof(customer));
    if (node == NULL) return 0;
    // Build Max Heap
    for (int i = size / 2; i >= 0; i--) {
        heapify(ifile, i, size, lar, node);
    }

    customer *first = (customer *)malloc(sizeof(customer));
    if (first == NULL) return 0;
    customer *switcher = (customer *)malloc(sizeof(customer));
    if (switcher == NULL) return 0;
    // Iterative Heapsort starts
    for (int i = 1; i < size; i++) {
        first = info(ifile, 0, first);
        switcher = info(ifile, size - i, switcher);

        // Move the max item to the end of the file then
        // call the heapify to build the max heap again
        order(ifile, 0, first, size - i, switcher);
        heapify(ifile, 0, size - i, lar, node);
    }

    // Release the allocated memory of customer data
    free(first);
    free(switcher);
    free(node);
    free(lar);

    // Close file
    if (fclose(ifile) == EOF) {
        fprintf(stderr, "Error closing file in function heapsort...\n");
        return 0;
    }

    return 1;
}
