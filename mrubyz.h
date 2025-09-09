#ifndef __MRUBYZ_H__
#  define __MRUBYZ_H__

#include <stdint.h>

typedef enum {
  T_INT = 0,
  T_STRING = 1,
} mrbz_type;

struct mrbz_value {
  mrbz_type type;
  // TODO: support any mrbz type
  union {
    uint16_t intval;
    char* strval;
  };
};
typedef struct mrbz_value mrbz_val;

struct mrbz_virtual_machine {
  // FIXME: fixed to 5
  mrbz_val regs[5];
};
typedef struct mrbz_virtual_machine mrbz_vm;

void mrbz_vm_run(mrbz_vm*, mrbz_val*, unsigned char*);

#endif
