
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define INTERFACE_NAME_LEN 16
#define MAX_INTERFACES 256

typedef enum interface_type_e {
  PHYSICAL,
  VLAN,
  VXLAN,
  GRE,
} interface_type_t;

typedef struct interface_s {
  char name[INTERFACE_NAME_LEN];
  uint16_t pid;
  interface_type_t type;
} interface_t;

typedef struct fib_s {
} fib_t;

typedef struct yoda_info_s {
  interface_t interfaces[MAX_INTERFACES];
  fib_t fib;
} yoda_info_t;

int main(int argc, char** argv)
{
  yoda_info_t yoda_info;
  while (true) {
  }
}

