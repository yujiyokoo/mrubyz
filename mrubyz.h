#ifndef __MRUBYZ_H__
#  define __MRUBYZ_H__

#include <stdint.h>

struct mrbz_virtual_machine {
  // FIXME: fixed to 4
  int regs[4];
};

struct mrbz_value {
  // TODO: Use enum
  int type;
  // TODO: support any mrbz type
  uint16_t value;
};

typedef struct mrbz_value mrbz_val;
typedef struct mrbz_virtual_machine mrbz_vm;

void mrbz_vm_run(mrbz_vm*, mrbz_val*, unsigned char*);

#endif
