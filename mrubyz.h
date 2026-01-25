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
  T_ARRAY  = 6,
  T_CLASS = 7,
  T_PROC = 8,
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

typedef struct mrbz_cls {
  const char* name;
} mrbz_class;

typedef struct mrbz_value {
  mrbz_type type;
  // TODO: support any mrbz type
  union {
    int16_t intval;
    char* strval;
    mrbz_array arrval;
    mrbz_class *clsval_ptr;
    uint8_t proc_index;
  } u;
} mrbz_val;

typedef struct mrubyz_irep {
  const uint8_t *pool;
  const uint8_t *syms;
  // hold pointers to start of symbols
	// FIXME: hardcoded length
  const char* syms_list[32];
  uint16_t nregs;
  uint16_t nlocals;
  uint16_t rlen;
  // pointer to the IREP's instructions start
  const unsigned char* iseq;
} mrbz_irep;

typedef struct mrbz_virtual_machine {
  mrbz_class object_class;
  mrbz_class* target_class;
  // dynamically allocated. Maybe simpler to set it to a larger fixed number?
  mrbz_val *regs;
  mrbz_irep *ireps;
  uint8_t current_irep;
} mrbz_vm;

void mrbz_vm_run(mrbz_vm*, mrbz_val*, unsigned char*);

void debug_print(const char* fmt, ...);

#endif
