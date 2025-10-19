#ifndef __MRUBYZ_H__
#  define __MRUBYZ_H__

#include <stdint.h>

typedef enum {
  T_INT    = 0,
  T_STRING = 1,
  T_TRUE   = 2,
  T_FALSE  = 3,
  T_NIL  = 4,
  T_OBJECT  = 5,
} mrbz_type;

struct mrbz_value {
  mrbz_type type;
  // TODO: support any mrbz type
  union {
    int16_t intval;
    char* strval;
  };
};
typedef struct mrbz_value mrbz_val;

struct mrubyz_irep {
  const uint8_t *pool;
  const uint16_t *syms;
};

typedef struct mrubyz_irep mrbz_irep;

struct mrbz_virtual_machine {
  // FIXME: fixed to 5
  mrbz_val regs[5];
  mrbz_irep irep;
};
typedef struct mrbz_virtual_machine mrbz_vm;

void mrbz_vm_run(mrbz_vm*, mrbz_val*, unsigned char*);

#endif
