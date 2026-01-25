#include "mrubyz.h"
#include "opcode.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Conditional (only in debug builds)
#ifdef DEBUG
#  define debug_out(...) printf(__VA_ARGS__)
#else
#  define debug_out(...) ((void)0)  // No-op in release
#endif

#ifdef DEBUG
#  define ASSERT(cond) do { if (!(cond)) { printf("ASSERT: %s:%d\r", __FILE__, __LINE__); exit(-1); } } while(0)
#else
#  define ASSERT(cond) ((void)0)
#endif

#ifdef __SMS__
#  include <sms.h>
#  include <SMSlib.h>
#endif

// TODO: Put it in another file, like compat.h?
#ifndef __SMS__
void gotoxy(uint8_t x, uint8_t y) {
  debug_out("gotoxy(%d, %d) called, but this is not SMS\n", x, y);
}

void SMS_VRAMmemset(uint16_t a, uint8_t b, uint16_t c) {
  debug_out("SMS_VRAMmemset(%d, %d, %d) called, but this is not SMS\n", a, b, c);
}

void wait_vblank_noint() {
  debug_out("wait_vblank_noint() called, but this is not SMS\n");
}

#endif

__sfr __at 0xDC io_port_dc;

const char* op_names[] = {
  "OP_NOP        = 0x00",
  "OP_MOVE       = 0x01",
  "OP_LOADL      = 0x02",
  "OP_LOADI      = 0x03",
  "OP_LOADINEG   = 0x04",
  "OP_LOADI__1   = 0x05",
  "OP_LOADI_0    = 0x06",
  "OP_LOADI_1    = 0x07",
  "OP_LOADI_2    = 0x08",
  "OP_LOADI_3    = 0x09",
  "OP_LOADI_4    = 0x0A",
  "OP_LOADI_5    = 0x0B",
  "OP_LOADI_6    = 0x0C",
  "OP_LOADI_7    = 0x0D",
  "OP_LOADI16    = 0x0E",
  "OP_LOADI32    = 0x0F",
  "OP_LOADSYM    = 0x10",
  "OP_LOADNIL    = 0x11",
  "OP_LOADSELF   = 0x12",
  "OP_LOADT      = 0x13",
  "OP_LOADF      = 0x14",
  "OP_GETGV      = 0x15",
  "OP_SETGV      = 0x16",
  "OP_GETSV      = 0x17",
  "OP_SETSV      = 0x18",
  "OP_GETIV      = 0x19",
  "OP_SETIV      = 0x1A",
  "OP_GETCV      = 0x1B",
  "OP_SETCV      = 0x1C",
  "OP_GETCONST   = 0x1D",
  "OP_SETCONST   = 0x1E",
  "OP_GETMCNST   = 0x1F",
  "OP_SETMCNST   = 0x20",
  "OP_GETUPVAR   = 0x21",
  "OP_SETUPVAR   = 0x22",
  "OP_GETIDX     = 0x23",
  "OP_SETIDX     = 0x24",
  "OP_JMP        = 0x25",
  "OP_JMPIF      = 0x26",
  "OP_JMPNOT     = 0x27",
  "OP_JMPNIL     = 0x28",
  "OP_JMPUW      = 0x29",
  "OP_EXCEPT     = 0x2A",
  "OP_RESCUE     = 0x2B",
  "OP_RAISEIF    = 0x2C",
  "OP_SSEND      = 0x2D",
  "OP_SSENDB     = 0x2E",
  "OP_SEND       = 0x2F",
  "OP_SENDB      = 0x30",
  "OP_CALL       = 0x31",
  "OP_SUPER      = 0x32",
  "OP_ARGARY     = 0x33",
  "OP_ENTER      = 0x34",
  "OP_KEY_P      = 0x35",
  "OP_KEYEND     = 0x36",
  "OP_KARG       = 0x37",
  "OP_RETURN     = 0x38",
  "OP_RETURN_BLK = 0x39",
  "OP_BREAK      = 0x3A",
  "OP_BLKPUSH    = 0x3B",
  "OP_ADD        = 0x3C",
  "OP_ADDI       = 0x3D",
  "OP_SUB        = 0x3E",
  "OP_SUBI       = 0x3F",
  "OP_MUL        = 0x40",
  "OP_DIV        = 0x41",
  "OP_EQ         = 0x42",
  "OP_LT         = 0x43",
  "OP_LE         = 0x44",
  "OP_GT         = 0x45",
  "OP_GE         = 0x46",
  "OP_ARRAY      = 0x47",
  "OP_ARRAY2     = 0x48",
  "OP_ARYCAT     = 0x49",
  "OP_ARYPUSH    = 0x4A",
  "OP_ARYDUP     = 0x4B",
  "OP_AREF       = 0x4C",
  "OP_ASET       = 0x4D",
  "OP_APOST      = 0x4E",
  "OP_INTERN     = 0x4F",
  "OP_SYMBOL     = 0x50",
  "OP_STRING     = 0x51",
  "OP_STRCAT     = 0x52",
  "OP_HASH       = 0x53",
  "OP_HASHADD    = 0x54",
  "OP_HASHCAT    = 0x55",
  "OP_LAMBDA     = 0x56",
  "OP_BLOCK      = 0x57",
  "OP_METHOD     = 0x58",
  "OP_RANGE_INC  = 0x59",
  "OP_RANGE_EXC  = 0x5A",
  "OP_OCLASS     = 0x5B",
  "OP_CLASS      = 0x5C",
  "OP_MODULE     = 0x5D",
  "OP_EXEC       = 0x5E",
  "OP_DEF        = 0x5F",
  "OP_ALIAS      = 0x60",
  "OP_UNDEF      = 0x61",
  "OP_SCLASS     = 0x62",
  "OP_TCLASS     = 0x63",
  "OP_DEBUG      = 0x64",
  "OP_ERR        = 0x65",
  "OP_EXT1       = 0x66",
  "OP_EXT2       = 0x67",
  "OP_EXT3       = 0x68",
  "OP_STOP       = 0x69"
};

void check_reg_idx(uint8_t idx, uint16_t nregs) {
  if(idx > nregs) {
    debug_out("reg index %d(%x) is above the maximum allowed\n", idx, idx);
    exit(-1);
  }
}

void op_loadi_n(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr, uint8_t inst) {
  int16_t imm_val = inst - OP_LOADI_0;
  uint8_t reg_index = bytecode[*pc_ptr];
  // debug_out("pc_ptr intval %d\n", *pc_ptr);

  // TODO: raise error
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  // debug_out("inst %d\n", inst);
  // debug_out("imm_val %d\n", imm_val);
  // debug_out("bc[pc_ptr] %d\n", bytecode[*pc_ptr]);
  // debug_out("OP_LOADI__1 %d\n", OP_LOADI__1);
  //debug_out("loading %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].u.intval = imm_val;
  *pc_ptr = *pc_ptr + 1;
}

// returns the byte at the current CP, and advances the CP by 1
uint8_t next_byte(unsigned char* bytecode, uint16_t *pc_ptr) {
  return bytecode[(*pc_ptr)++];
}

// returns the 16-bit word at the current CP, and advances the CP by 2
uint16_t next_word(unsigned char* bytecode, uint16_t *pc_ptr) {
  return (bytecode[(*pc_ptr)++] << 8) | (bytecode[(*pc_ptr)++]);
}

void op_loadi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr, uint8_t inst) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].u.intval = imm_val;
}

void op_loadineg(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr, uint8_t inst) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].u.intval = -imm_val;
}

mrbz_val *op_return(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  debug_out("op_return, reg_index is: %d\n", reg_index);

  // TODO: raise error
  check_reg_idx(reg_index, vm->ireps[0].nregs);

  debug_out("op_return, vm->r[ri]: %d\n",vm->regs[reg_index]);
  debug_out("op_return, returning type %d\n", vm->regs[reg_index].type);
  mrbz_val* foo = vm->regs + reg_index;
  debug_out("op_return, returning %s (if str)\n", foo->u.strval);
  debug_out("op_return, returning %d (if int)\n", foo->u.intval);
  return vm->regs + reg_index;
}

void op_move(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t dest_reg = next_byte(bytecode, pc_ptr);
  uint8_t src_reg = next_byte(bytecode, pc_ptr);
  //debug_out("moving. reg %d to %d, intval is %d\n", src_reg, dest_reg, vm->regs[src_reg]);
  check_reg_idx(dest_reg, vm->ireps[0].nregs);
  check_reg_idx(src_reg, vm->ireps[0].nregs);
  // TODO: check this assignment is valid with the compiler
  vm->regs[dest_reg] = vm->regs[src_reg];
}

void op_add(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("adding. reg %d to %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index+1].intval);
  vm->regs[reg_index].u.intval += vm->regs[reg_index + 1].u.intval;
}

void op_addi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("adding. %d to reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].u.intval += imm_val;
}

void op_sub(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("subtracting. reg %d from %d, values are %d and %d\n", reg_index + 1, reg_index, vm->regs[reg_index + 1].intval, vm->regs[reg_index].intval);
  vm->regs[reg_index].u.intval -= vm->regs[reg_index + 1].u.intval;
}

void op_subi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("subtracting. %d from reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].u.intval -= imm_val;
}

void op_mul(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("multiplying. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].u.intval *= vm->regs[reg_index + 1].u.intval;
}

void op_div(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  //debug_out("dividing. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].u.intval /= vm->regs[reg_index + 1].u.intval;
}

void op_loadnil(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  vm->regs[reg_index].type = T_NIL;
}

void op_loadt(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  vm->regs[reg_index].type = T_TRUE;
}

void op_loadf(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  vm->regs[reg_index].type = T_FALSE;
}

void op_array2(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // BBB, R[a] = array containing R[b..b+c]
  uint8_t dest_index = next_byte(bytecode, pc_ptr);
  uint8_t arr_start_index = next_byte(bytecode, pc_ptr);
  uint8_t arr_len = next_byte(bytecode, pc_ptr);
  check_reg_idx(arr_start_index + arr_len, vm->ireps[0].nregs);
  mrbz_val* content = (mrbz_val*)malloc(sizeof(mrbz_val) * (arr_len+1)); // add one for the NULL

  if (content == NULL) {
    printf("malloc of len %d failed. Out of memory maybe", arr_len);
    exit(-1);
  }

  for (uint8_t i = 0; i < arr_len; i++) {
    content[i] = vm->regs[arr_start_index + i];
  }

  mrbz_array arr;
  arr.data = content;
  arr.len = arr_len;

  vm->regs[dest_index].type = T_ARRAY;
  vm->regs[dest_index].u.arrval = arr;
}

void* mrbz_irep_pool_entry_ptr(mrbz_irep* irep_p, uint8_t idx) {
  // TODO: pool is in RITE binary. Pool belongs to IREP
  // debug_out("mrbz_irep_pool_entry_ptr: %d\n", irep_p->pool+idx);
  const uint8_t *pool_cpy = irep_p->pool;
  const uint16_t plen = *(uint16_t*)(irep_p->pool);
  pool_cpy += 2; // skip plen
  while(idx > 0) {
    switch(*pool_cpy) {
    case IREP_TT_STR:
    case IREP_TT_SSTR:
      pool_cpy++; // skip type
      uint16_t len = (*(pool_cpy++) << 8) | *(pool_cpy++);
      pool_cpy += len +1;
      break;
    default:
      debug_out("Unsupported IREP_TT in constant pool: %x\n", *irep_p->pool);
      exit(-1);
    };
    idx--;
  }

  return pool_cpy;
}

void op_string(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);

  uint8_t pool_index = next_byte(bytecode, pc_ptr);
  void* pool_entry_base = mrbz_irep_pool_entry_ptr(&(vm->ireps[0]), pool_index);
  vm->regs[reg_index].type = T_STRING;

  // TODO: Add length to mrbz value, and copy length
  uint16_t str_len = (uint16_t)pool_entry_base[1];
  // TODO: seems like my demo runs out of RAM? Let's point this to static str...
  //vm->regs[reg_index].u.strval = malloc(str_len * sizeof(char) + 1);
  vm->regs[reg_index].u.strval = pool_entry_base + 3;
  // + 1 is there for now to null-terminate the string
  // debug_out("copying string: %s\n", pool_entry_base+5);
  // debug_out("regs: %x\n", vm->regs);
  // debug_out("dest: %x\n", vm->regs[reg_index].u.strval);
  // strncpy(vm->regs[reg_index].u.strval, pool_entry_base + 5, str_len + 1);
  // debug_out("copied string: %s\n", vm->regs[reg_index].u.strval);
}

void op_method(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t irep_index = next_byte(bytecode, pc_ptr);

  vm->regs[reg_index].type = T_PROC;
  vm->regs[reg_index].u.proc_index = irep_index;
}

void op_tclass(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  vm->regs[reg_index].type = T_CLASS;
  vm->regs[reg_index].u.clsval_ptr = vm->target_class;
}

void op_eq(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  if(vm->regs[reg_index].type == T_INT && vm->regs[reg_index+1].type == T_INT) {
    if(vm->regs[reg_index].u.intval == vm->regs[reg_index+1].u.intval) {
      vm->regs[reg_index].type = T_TRUE;
    } else {
      vm->regs[reg_index].type = T_FALSE;
    }
  } else if(vm->regs[reg_index].type == T_STRING && vm->regs[reg_index+1].type == T_STRING) {
    if(strcmp(vm->regs[reg_index].u.strval, vm->regs[reg_index+1].u.strval) == 0) {
      vm->regs[reg_index].type = T_TRUE;
    } else {
      vm->regs[reg_index].type = T_FALSE;
    }
  } else {
    printf("unsupported eq types: %d == %d\r", vm->regs[reg_index].type, vm->regs[reg_index+1].type);
    // Crash for now
    exit(-1);
  }
}

void op_gt(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // TODO: generic object support
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->ireps[0].nregs);
  // Only supports integer for now
  if(vm->regs[reg_index].type == T_INT && vm->regs[reg_index+1].type == T_INT) {
    // TODO: handle freeing... GCing...
    if(vm->regs[reg_index].u.intval > vm->regs[reg_index+1].u.intval) {
      vm->regs[reg_index].type = T_TRUE;
    } else {
      vm->regs[reg_index].type = T_FALSE;
    }
  } else {
    printf("non-integer comparison not supported\r");
    exit(-1);
  }
}

void op_jmpnot(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  int16_t jump_by = next_word(bytecode, pc_ptr);

  // TODO: optimise by ordering NIL, FALSE before everything else
  if(vm->regs[reg_index].type == T_NIL || vm->regs[reg_index].type == T_FALSE) {
    *pc_ptr += jump_by;
  }
}

void op_jmpif(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  int16_t jump_by = next_word(bytecode, pc_ptr);

  // TODO: optimise by ordering NIL, FALSE before everything else
  if(vm->regs[reg_index].type != T_NIL && vm->regs[reg_index].type != T_FALSE) {
    *pc_ptr += jump_by;
  }
}

void op_getidx(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);

  // TODO: check vm->regs[reg_index] is array and vm->regs[reg_index+1] is int
  vm->regs[reg_index] = vm->regs[reg_index].u.arrval.data[vm->regs[reg_index+1].u.intval];
}

void op_jmp(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  int16_t jump_by = next_word(bytecode, pc_ptr);

  *pc_ptr += jump_by;
}

const char* symbol_at(mrbz_vm *vm, uint8_t sym_index) {
  return (unsigned char*)(vm->ireps[0].syms_list[sym_index]);
}

void dispatch_array_method(mrbz_vm *vm, mrbz_val *receiver, uint8_t reg_index, const char *sym, uint8_t arg_count) {
  if (!strcmp(sym, "size")) {
    vm->regs[reg_index].type = T_INT;
    vm->regs[reg_index].u.intval = receiver->u.arrval.len;
  } else {
    printf("Unknown array method: %s\r", sym);
    exit(-1);
  }
}

void dispatch_int_method(mrbz_vm *vm, mrbz_val *receiver, uint8_t reg_index, const char *sym, uint8_t arg_count) {
  if (!strcmp(sym, "&")) {
    if (vm->regs[reg_index + 1].type != T_INT) {
      printf("Unexpected argument type for `&`\r");
      exit(-1);
    }
    vm->regs[reg_index].type = T_INT;
    vm->regs[reg_index].u.intval = receiver->u.intval & vm->regs[reg_index + 1].u.intval;
  } else if (!strcmp(sym, "!=")) {
    if (vm->regs[reg_index + 1].type != T_INT) {
      printf("Unexpected argument type for `!=`\r");
      exit(-1);
    }
    vm->regs[reg_index].type = (receiver->u.intval != vm->regs[reg_index + 1].u.intval) ? T_TRUE : T_FALSE;
  } else {
    printf("Unknown int method: %s\r", sym);
    exit(-1);
  }
}

// special case for bang (!)
void dispatch_bang(mrbz_vm *vm, mrbz_val *receiver, uint8_t reg_index, const char *sym, uint8_t arg_count) {
  if(receiver->type == T_FALSE || receiver->type == T_NIL) {
    vm->regs[reg_index].type = T_TRUE;
  } else {
    vm->regs[reg_index].type = T_FALSE;
  }
}

void op_send(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t sym_index = next_byte(bytecode, pc_ptr);
  // No support for keyword arguments for now (& 0x0F -> lower 4 bits)
  uint8_t arg_count = next_byte(bytecode, pc_ptr) & 0x0F;

  uint16_t syms_len = ((uint16_t)vm->ireps[0].syms[0] << 8) | (uint16_t)vm->ireps[0].syms[1];
  if(sym_index >= syms_len) {
    printf("sym_index (%d) out of bounds (%d max)\r", sym_index, syms_len);
    exit(-1);
  }

  const char* sym = symbol_at(vm, sym_index);

  mrbz_val *receiver = &vm->regs[reg_index];

  // Special case for bang (!) method
  if(!strcmp(sym, "!")) {
    dispatch_bang(vm, receiver, reg_index, sym, arg_count);
    return;
  }

  switch(receiver->type) {
    case T_ARRAY:
      dispatch_array_method(vm, receiver, reg_index, sym, arg_count);
      break;
    case T_INT:
      dispatch_int_method(vm, receiver, reg_index, sym, arg_count);
      break;
    default:
      printf("OP_SEND not supported for type %d\r", receiver->type);
      exit(-1);
  }
}

void op_ssend(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t sym_index = next_byte(bytecode, pc_ptr);
  uint8_t arg_info = next_byte(bytecode, pc_ptr); // mostly ignored
  const char* sym;
  static uint8_t count = 0;

  uint16_t syms_len = ((uint16_t)vm->ireps[0].syms[0] << 8) | (uint16_t)vm->ireps[0].syms[1];
  if(sym_index >= syms_len) {
    printf("sym_index (%d) out of bounds (%d max)\r", sym_index, syms_len);
    exit(-1);
  }

  sym = symbol_at(vm, sym_index);

  // Hardcoding for now... will fix later
  if(!strcmp(sym, "puts")) {
    // puts is called with R1 as arg, so R[1+1] is the arg sent to puts
    fprintf(stdout, "%s\r", vm->regs[reg_index+1].u.strval);
    vm->regs[reg_index].type = T_NIL; // Use reg[reg_index] for return
  } else if(!strcmp(sym, "print")) {
    // TODO: use same implementation b/w puts and print
    if(vm->regs[reg_index+1].type == T_STRING) {
      fprintf(stdout, "%s", vm->regs[reg_index+1].u.strval);
    } else if(vm->regs[reg_index+1].type == T_INT) {
      fprintf(stdout, "%d", vm->regs[reg_index+1].u.intval);
    }
    vm->regs[reg_index].type = T_NIL; // Use reg[reg_index] for return
  } else if (!strcmp(sym, "read_buttons")) {
    vm->regs[reg_index].type = T_INT;
    vm->regs[reg_index].u.intval = io_port_dc;
  } else if (!strcmp(sym, "gotoxy")) {
    if(arg_info != 2) {
      printf("Unexpected argument count\r");
      exit(-1);
    }
    uint8_t x = vm->regs[reg_index + 1].u.intval;
    uint8_t y = vm->regs[reg_index + 2].u.intval;
    gotoxy(x, y);
    vm->regs[reg_index].type = T_NIL;
  } else if (!strcmp(sym, "foo")) {
    printf("'foo' called\r");
    vm->regs[reg_index].type = T_TRUE; // Use reg[reg_index] for return
  } else if (!strcmp(sym, "bar")) {
    vm->regs[reg_index].type = T_INT; // Use reg[reg_index] for return
    vm->regs[reg_index].u.intval = 5;
  } else if (!strcmp(sym, "clear_screen")) {
    SMS_VRAMmemset(0x3800, 0x00, 32*28*2);
  } else if (!strcmp(sym, "wait_vblank")) {
    // XXX: SMS_waitForVBlank() does not seem to work?
    wait_vblank_noint();
  } else {
    printf("unknown symbol call: %s\r", sym);
    exit(-1);
  }
}

void parse_irep_record(mrbz_irep *irep, unsigned char *bytecode, uint16_t *pc) {
  uint16_t record_start = *pc;
  // TODO: Ignoring upper 2 bytes as we currently only support 16 bits
  // Perhaps error if upper bits are not zero?
  uint16_t record_len = bytecode[*pc+3] | (bytecode[*pc+2] << 8) ;
  debug_out("irep rec len: %d / 0x%x\n", record_len, record_len);
  *pc += 4; // skipping 4 bytes for irep record size

  uint16_t nlocals = (bytecode[*pc] << 8) |
                 bytecode[*pc+1];
  debug_out("nlocals is %d / 0x%x\n", nlocals, nlocals);
  *pc += 2;
  uint16_t nregs = (bytecode[*pc] << 8) | bytecode[*pc+1];
  *pc += 2;

  uint16_t rlen = (bytecode[*pc] << 8) |
                 bytecode[*pc+1];
  *pc += 2 ;

  irep->nlocals = nlocals;
  irep->nregs = nregs;
  irep->rlen = rlen;
  debug_out("nregs is %d / 0x%x\n", nregs, nregs);
  debug_out("rlen is: %d / 0x%x\n", rlen, rlen);

  // FIXME: this only makes sense when there is no method dispatch
  if (irep->nregs > 16) {
    printf("IREP needs %d regs, max is %d\r", irep->nregs, 16);
    exit(-1);
  }

  debug_out("clen is: %d / 0x%x\n", bytecode[*pc], bytecode[*pc]);
  *pc += 2;
  // TODO: Ignoring upper 2 bytes as we currently only support 16 bits
  // Perhaps error if upper bits are not zero?
  uint16_t ilen = bytecode[*pc+3] |
                 (bytecode[*pc+2] << 8) ;
                 // ((uint32_t)bytecode[*pc+1] << 16) |
                 // ((uint32_t)bytecode[*pc] << 24);
  debug_out("ilen: %d / 0x%x\n", ilen, ilen);
  *pc += 4; // skip 4 bytes for the ilen

  irep->pool = bytecode + *pc + ilen;
  debug_out("bytecode ptr: %d / 0x%x\n", (uint16_t)(bytecode), (uint16_t)(bytecode));
  debug_out("*pc: %d / 0x%x\n", *pc, *pc);
  debug_out("pool ptr: %d / 0x%x\n", (uint16_t)(irep->pool), (uint16_t)(irep->pool));

  const uint8_t *pool_ptr = irep->pool;
  debug_out("pool_ptr[0] raw value: %d / 0x%x\n", pool_ptr[0], pool_ptr[0]);
  debug_out("pool_ptr[1] raw value: %d / 0x%x\n", pool_ptr[1], pool_ptr[1]);

  uint16_t temp = pool_ptr[0];
  debug_out("temp (before shift): 0x%x\n", temp);
  temp = temp << 8;
  debug_out("temp (after shift): 0x%x\n", temp);

  uint16_t plen_l = (uint16_t)(pool_ptr[0] << 8);
  debug_out("plen_l: 0x%x\n", plen_l);
  debug_out("(pool ptr[0]) << 8: %d / 0x%x\n", (uint16_t)(pool_ptr[0] << 8), (uint16_t)(pool_ptr[0] << 8));
  //uint16_t plen = (pool_ptr[0] << 8) | pool_ptr[1];
  uint16_t plen_r = (uint16_t)pool_ptr[1];
  debug_out("plen_l: 0x%x, plen_r: 0x%x\n", plen_l, plen_r );
  uint16_t plen = plen_l | plen_r;
  debug_out("plen is: %d / 0x%x\n", plen, plen);
  debug_out("*(pool ptr): %d / 0x%x\n", (uint16_t)(pool_ptr[0]), (uint16_t)(pool_ptr[0]));
  debug_out("*(pool ptr+1): %d / 0x%x\n", (uint16_t)(pool_ptr[1]), (uint16_t)(pool_ptr[1]));
  pool_ptr += 2;  // skip plen

  // plen by pos
  uint16_t plen1 = (bytecode[*pc+ilen] << 8) | bytecode[*pc+ilen+1];
  debug_out("plen1 ptr is: %d / 0x%x\n", &(bytecode[*pc+ilen]), &(bytecode[*pc+ilen]));
  debug_out("plen1 is: %d / 0x%x\n", plen1, plen1);

  // bytes
  debug_out("*(plen1 ptr): %d / 0x%x\n", (bytecode[*pc+ilen]), (bytecode[*pc+ilen]));
  debug_out("*(plen1 ptr+1): %d / 0x%x\n", (bytecode[*pc+ilen+1]), (bytecode[*pc+ilen+1]));

  // Iterate through all pool entries
  for(uint16_t i = 0; i < plen; i++) {
    uint16_t str_len;
    uint8_t pool_type = *pool_ptr;
    pool_ptr++;  // skip type byte
    debug_out("i: %d, type: 0x%x\n", i, pool_type);

    switch(pool_type) {
      case IREP_TT_STR:
      case IREP_TT_SSTR:
        str_len = (*pool_ptr << 8) | *(pool_ptr + 1);
        pool_ptr += 2;  // skip length bytes
        debug_out("str found in pool: %s\n", pool_ptr);
        pool_ptr += str_len + 1;  // skip string data + null terminator
        debug_out("str pool type: %x\n", pool_type);
        break;

      case IREP_TT_INT32:
        pool_ptr += 4;  // skip 4 bytes for int32
        debug_out("int pool type\n");
        break;

      default:
        debug_out("Unknown pool type: %x\n", pool_type);
        exit(-1);
    }
  }

  // Save syms section ptr
  irep->syms = pool_ptr;
  uint16_t syms_len = ((uint16_t)pool_ptr[0] << 8) | (uint16_t)pool_ptr[1];
  pool_ptr += 2; // skip syms_len
  debug_out("symbol block length: %d\n", syms_len);

  for(uint16_t count = 0; count < syms_len; count++) {
    uint16_t sym_len = ((uint16_t)pool_ptr[0] << 8) | (uint16_t)pool_ptr[1];
    debug_out("symbol length: %d\n", sym_len);
    pool_ptr += 2; // skip sym_len
    debug_out("symbol found: %s\n", pool_ptr);
    irep->syms_list[count] = pool_ptr;
    while(*pool_ptr) { pool_ptr++; } // move to null terminator
    pool_ptr++; // skip the null
  }

  *pc = record_start + record_len; // move to the next IREP
}

void parse_ireps(mrbz_vm **vm, unsigned char *bytecode, uint16_t *pc) {
  ASSERT(strncmp(bytecode+*pc, "IREP", 4) == 0);
  *pc += 4; // skipping over "IREP"

  // TODO: Ignoring upper 2 bytes as we currently only support 16 bits
  // Perhaps error if upper bits are not zero?
  uint16_t section_len = bytecode[*pc+3] |
                 (bytecode[*pc+2] << 8) ;
                 // ((uint32_t)bytecode[*pc+1] << 16) |
                 // ((uint32_t)bytecode[*pc] << 24);
  debug_out("section length: %d / 0x%x\n", section_len, section_len);
  *pc += 4; // 4 bytes for the section length

  ASSERT(strncmp(bytecode+*pc, "0300", 4) == 0);
  *pc += 4; // skipping over "0300"

  // TODO: currently we only support top IREP + its children, no nesting
  // So we check the rlen here and allocate space

  // Skip record_len (4) + nlocals (2) + nregs (2) -> 8 bytes
  uint16_t rlen = (bytecode[*pc+8] << 8) | bytecode[*pc+9];
  (*vm)->ireps = calloc(rlen+1, sizeof(mrbz_irep));

  for (uint16_t i = 0; i < rlen+1; i++) {
    parse_irep_record(&(*vm)->ireps[i], bytecode, pc);
  }
}

void mrbz_vm_run(mrbz_vm *vm, mrbz_val* rval, unsigned char* bytecode) {
  // TODO: check more header vaules
  // we know header length is 20, so skip for now
  vm->object_class.name = "Object";
  vm->target_class = &(vm->object_class);
  vm->current_irep = 0;
  uint16_t pc = 20;
  // to view contents
  // for(uint16_t cp = pc; cp < 100; cp ++) {
    // debug_out("bytecode at %d: %d\n", cp, bytecode[cp]);
  // }
  parse_ireps(&vm, bytecode, &pc);

  // Allocate registers - fixed to 16 for now
  vm->regs = (mrbz_val*)malloc(sizeof(mrbz_val) * 16);
  if (vm->regs == NULL) {
    debug_out("Failed to allocate %d registers\n", 16);
    exit(-1);
  }
  // Initialize all registers to nil
  for (uint8_t i = 0; i < 16; i++) {
    vm->regs[i].type = T_NIL;
  }


  // debug_out("bytecode start is: %d, pc (iseq start index) is: %d, iseq length is: %d\n", bytecode, pc, ilen);

  // initialise R0 with an object
  vm->regs[0].type = T_OBJECT;

  // Initialise object_class
  // Initialise current class

  // while instructions are left
  int exiting = 0;
  mrbz_val *retval = NULL;

  while (!exiting) {
    uint8_t instruction;
    instruction = bytecode[pc++];
    // debug_out("PC: %d(%x), OP: %s\n", pc, pc, op_names[instruction]);
    switch(instruction) {
      case OP_NOP: break;
      case OP_MOVE: op_move(vm, bytecode, &pc); break;
      case OP_LOADI: op_loadi(vm, bytecode, &pc, instruction); break;
      case OP_LOADINEG: op_loadineg(vm, bytecode, &pc, instruction); break;
      case OP_LOADI__1: // fall through
      case OP_LOADI_0: // fall through
      case OP_LOADI_1: // fall through
      case OP_LOADI_2: // fall through
      case OP_LOADI_3: // fall through
      case OP_LOADI_4: // fall through
      case OP_LOADI_5: // fall through
      case OP_LOADI_6: // fall through
      case OP_LOADI_7: op_loadi_n(vm, bytecode, &pc, instruction); break;
      case OP_LOADNIL: op_loadnil(vm, bytecode, &pc); break;
      case OP_LOADT: op_loadt(vm, bytecode, &pc); break;
      case OP_LOADF: op_loadf(vm, bytecode, &pc); break;
      case OP_GETIDX: op_getidx(vm, bytecode, &pc); break;
      case OP_JMP: op_jmp(vm, bytecode, &pc); break;
      case OP_JMPIF: op_jmpif(vm, bytecode, &pc); break;
      case OP_JMPNOT: op_jmpnot(vm, bytecode, &pc); break;
      case OP_SEND: op_send(vm, bytecode, &pc); break;
      case OP_SSEND: op_ssend(vm, bytecode, &pc); break;
      // FIXME: Currently this return exits from any level...
      case OP_RETURN: retval = op_return(vm, bytecode, &pc); exiting = 1; break;
      case OP_ADD: op_add(vm, bytecode, &pc); break;
      case OP_ADDI: op_addi(vm, bytecode, &pc); break;
      case OP_SUB: op_sub(vm, bytecode, &pc); break;
      case OP_SUBI: op_subi(vm, bytecode, &pc); break;
      case OP_MUL: op_mul(vm, bytecode, &pc); break;
      case OP_DIV: op_div(vm, bytecode, &pc); break;
      case OP_EQ: op_eq(vm, bytecode, &pc); break;
      case OP_GT: op_gt(vm, bytecode, &pc); break;
      case OP_ARRAY2: op_array2(vm, bytecode, &pc); break;
      case OP_STRING: op_string(vm, bytecode, &pc); break;
      case OP_METHOD: op_method(vm, bytecode, &pc); break;
      case OP_TCLASS: op_tclass(vm, bytecode, &pc); break;
      case OP_STOP: exiting = 1; break;
      default:
        printf("unsupported instruction: 0x%x\r", instruction);
        printf("OP: %s\r", op_names[instruction]);
        // Crash on unsupported instruction
        exit(-1);
    }
  }

  // set the return val
  if(retval != NULL) {
    debug_out("end of vmrun. Type: %d\n", retval->type);
    rval->type = retval->type;
    switch(rval->type) {
      case T_INT:
        debug_out("intval: %d\n", retval->u.intval);
        rval->u.intval = retval->u.intval;
        break;
      case T_STRING:
        debug_out("strval: %d\n", retval->u.strval);
        rval->u.strval = retval->u.strval;
        break;
      case T_TRUE:
      case T_FALSE:
        // these types don't have values - only types. Fall through
      default:
        // FIXME: unknown / unhandled type
        break;
    }
  }
}
