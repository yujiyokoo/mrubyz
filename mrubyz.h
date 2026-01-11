#ifndef __MRUBYZ_H__
#  define __MRUBYZ_H__

#define DEBUG_OUTPUT 1

#include <stdint.h>

typedef enum {
  T_INT    = 0,
  T_STRING = 1,
  T_TRUE   = 2,
  T_FALSE  = 3,
  T_NIL  = 4,
  T_OBJECT  = 5,
  T_ARRAY  = 6,
} mrbz_type;

typedef enum irep_const_type {
  IREP_TT_STR   = 0,
  IREP_TT_SSTR  = 2,
  IREP_TT_INT32 = 1,
  IREP_TT_INT64 = 3,
} const_type;

typedef struct {
  struct mrbz_value *data;
  uint8_t len;
} mrbz_array;

struct mrbz_value {
  mrbz_type type;
  // TODO: support any mrbz type
  union {
    int16_t intval;
    char* strval;
    mrbz_array arrval;
  } u;
};
typedef struct mrbz_value mrbz_val;

struct mrubyz_irep {
  const uint8_t *pool;
  const uint8_t *syms;
  // hold pointers to start of symbols
	// FIXME: hardcoded length
  const char* syms_list[64];
  uint8_t nregs;
  uint8_t nlocals;
};

typedef struct mrubyz_irep mrbz_irep;

struct mrbz_virtual_machine {
  // dynamically allocated. Maybe simpler to set it to a larger fixed number?
  mrbz_val *regs;
  mrbz_irep irep;
};
typedef struct mrbz_virtual_machine mrbz_vm;

void mrbz_vm_run(mrbz_vm*, mrbz_val*, unsigned char*);

void debug_print(const char* fmt, ...);

#endif
