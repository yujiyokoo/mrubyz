#include "mrubyz.h"
#include "opcode.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

void op_loadi_n(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p, uint8_t inst) {
  uint8_t imm_val = inst - OP_LOADI_0;
  uint8_t reg_index = bytecode[*curr_p] - 1;
  // printf("curr_p intval %d\n", *curr_p);

  // TODO: raise error
  if(reg_index >= 5) {
    // printf("register %d not available\n", reg_index);
    exit(-1);
  } else {
    // printf("inst %d\n", inst);
    // printf("imm_val %d\n", imm_val);
    // printf("bc[curr_p] %d\n", bytecode[*curr_p]);
    // printf("OP_LOADI__1 %d\n", OP_LOADI__1);
    //printf("loading %d to reg %d\n", imm_val, reg_index);
    vm->regs[reg_index].type = T_INT;
    vm->regs[reg_index].intval = imm_val;
    *curr_p = *curr_p + 1;
  }
}

void op_loadi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p, uint8_t inst) {
  uint8_t reg_index = bytecode[*curr_p] - 1;
  uint8_t imm_val = bytecode[*curr_p+1];
  if(reg_index >= 5) { exit(-1); }
  //printf("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].intval = imm_val;
  *curr_p = *curr_p + 2;
}

void op_loadineg(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p, uint8_t inst) {
  uint8_t reg_index = bytecode[*curr_p] - 1;
  uint8_t imm_val = bytecode[*curr_p+1];
  if(reg_index >= 5) { exit(-1); }
  //printf("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].intval = -imm_val;
  *curr_p = *curr_p + 2;
}

mrbz_val *op_return(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  uint8_t reg_index = bytecode[*curr_p] - 1;
  *curr_p = *curr_p + 1;
  // printf("reg_index is: %d\n", reg_index);

  // TODO: raise error
  if(reg_index >= 5) {
    //printf("register %d not available\n", reg_index);
    exit(-1);
  } else {
    // printf("vm->r[ri]: %d\n",vm->regs[reg_index]);
    // printf("returning type %d\n", vm->regs[reg_index].type);
    mrbz_val* foo = vm->regs + reg_index;
    // printf("returning %s\n", foo->strval);
    return vm->regs + reg_index;
  }
}

void op_move(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  uint8_t dest_reg = bytecode[*curr_p] - 1;
  uint8_t src_reg = bytecode[*curr_p + 1] - 1;
  //printf("moving. reg %d to %d, intval is %d\n", src_reg, dest_reg, vm->regs[src_reg]);
  if(dest_reg >= 5 || src_reg >= 5) { exit(-1); }
  // TODO: check this assignment is valid with the compiler
  vm->regs[dest_reg] = vm->regs[src_reg];
  *curr_p = *curr_p + 2;
}

void op_add(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  //printf("adding. reg %d to %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index+1].intval);
  vm->regs[reg_index].intval += vm->regs[reg_index + 1].intval;
  *curr_p = *curr_p + 1;
}

void op_addi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  uint8_t imm_val = bytecode[*curr_p + 1];
  //printf("adding. %d to reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval += imm_val;
  *curr_p = *curr_p + 2;
}

void op_sub(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  //printf("subtracting. reg %d from %d, values are %d and %d\n", reg_index + 1, reg_index, vm->regs[reg_index + 1].intval, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval -= vm->regs[reg_index + 1].intval;
  *curr_p = *curr_p + 1;
}

void op_subi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  uint8_t imm_val = bytecode[*curr_p + 1];
  //printf("subtracting. %d from reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval -= imm_val;
  *curr_p = *curr_p + 2;
}

void op_mul(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  //printf("multiplying. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].intval *= vm->regs[reg_index + 1].intval;
  *curr_p = *curr_p + 1;
}

void op_div(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  //printf("dividing. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].intval /= vm->regs[reg_index + 1].intval;
  *curr_p = *curr_p + 1;
}

uint16_t flip_endian(uint16_t i) {
  return (i << 8) | (i >> 8);
}

void* mrbz_irep_pool_entry_ptr(mrbz_irep* irep_p, uint8_t idx) {
  // TODO: pool is in RITE binary. Pool belongs to IREP
  // printf("mrbz_irep_pool_entry_ptr: %d\n", irep_p->pool+idx);
  return irep_p->pool+idx;
}

void op_string(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
  // arg0 should be duplicated
  // then copied to register indicated by arg1
  uint8_t reg_index = bytecode[*curr_p] - 1;
  if(reg_index >= 5) { exit(-1); }
  uint8_t pool_index = bytecode[++(*curr_p)];
  void* pool_entry_base = mrbz_irep_pool_entry_ptr(vm->irep, pool_index);
  uint16_t plen = (uint16_t)pool_entry_base[1];
  vm->regs[reg_index].type = T_STRING;
  // TODO: Add length to mrbz value, and copy length
  uint16_t str_len = (uint16_t)pool_entry_base[3];
  vm->regs[reg_index].strval = malloc(str_len * sizeof(char) + 1);
  // + 1 is there for now to null-terminate the string
  // printf("copying string: %s\n", pool_entry_base+5);
  strncpy(vm->regs[reg_index].strval, pool_entry_base + 5, str_len + 1);
  // printf("copied string: %s\n", vm->regs[reg_index].strval);
}

void mrbz_vm_run(mrbz_vm *vm, mrbz_val* rval, unsigned char* bytecode) {
  // TODO: check more header vaules
  // we know header length is 20, so skip for now
  uint16_t curr = 20;
  // to view contents
  // for(uint16_t cp = curr; cp < 100; cp ++) {
    // printf("bytecode at %d: %d\n", cp, bytecode[cp]);
  // }
  curr += 4; // TODO: skipping "IREP"

  // TODO: it's 4 bytes..
  // BUT! uint32_t is not supported?
  uint16_t section_len = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
                 // ((uint32_t)bytecode[curr+1] << 16) |
                 // ((uint32_t)bytecode[curr] << 24);
  //printf("length: %d\n", len)
  curr += 4; // skpping 4 bytes for the section length
  curr += 4; // TODO: skipping "0300"
  uint16_t record_len = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
  // printf("irep rec len: %d\n", record_len);
  curr += 4; // skipping 4 bytes for irep record size

  // TODO: skipping nlocals, nregs, rlen, clen...
  curr += 2 + 2 + 2 + 2 ;
  // TODO: it's 4 bytes..
  // BUT! uint32_t is not supported?
  uint16_t ilen = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
                 // ((uint32_t)bytecode[curr+1] << 16) |
                 // ((uint32_t)bytecode[curr] << 24);
  // printf("ilen: %d\n", ilen);
  curr += 4; // skip 4 bytes for the ilen

  // TODO: init this properly as pointer...?
  vm->irep.pool = bytecode + curr + ilen;

  // printf("bytecode start is: %d, curr (iseq start index) is: %d, iseq length is: %d\n", bytecode, curr, ilen);


  // while instructions are left
  int exiting = 0;
  mrbz_val *retval = NULL;

  while (!exiting) {
    uint8_t instruction;
    instruction = bytecode[curr++];
    //printf("PC: %d, OP: %s\n", curr, op_names[instruction]);
    switch(instruction) {
      case OP_NOP: break;
      case OP_LOADI_0: // fall through
      case OP_LOADI_1: // fall through
      case OP_LOADI_2: // fall through
      case OP_LOADI_3: // fall through
      case OP_LOADI_4: // fall through
      case OP_LOADI_5: // fall through
      case OP_LOADI_6: // fall through
      case OP_LOADI_7: op_loadi_n(vm, bytecode, &curr, instruction); break;
      case OP_LOADI: op_loadi(vm, bytecode, &curr, instruction); break;
      case OP_LOADINEG: op_loadineg(vm, bytecode, &curr, instruction); break;
      // FIXME: Currently this return exits from any level...
      case OP_RETURN: retval = op_return(vm, bytecode, &curr); exiting = 1; break;
      case OP_STOP: exiting = 1; break;
      case OP_MOVE: op_move(vm, bytecode, &curr); break;
      case OP_ADD: op_add(vm, bytecode, &curr); break;
      case OP_SUB: op_sub(vm, bytecode, &curr); break;
      case OP_MUL: op_mul(vm, bytecode, &curr); break;
      case OP_DIV: op_div(vm, bytecode, &curr); break;
      case OP_ADDI: op_addi(vm, bytecode, &curr); break;
      case OP_SUBI: op_subi(vm, bytecode, &curr); break;
      case OP_STRING: op_string(vm, bytecode, &curr); break;
      default:
        //printf("unsupported instruction: 0x%x\n", instruction);
        //printf("OP: %s\n", op_names[instruction]);
        break;
    }
  }
    // pop off first
    // execute

  // printf("retval: %d\n", retval);
  // set the return val
  if(retval != NULL) {
    rval->type = retval->type;
    switch(rval->type) {
      case T_INT:
        rval->intval = retval->intval;
        break;
      case T_STRING:
        rval->strval = retval->strval;
        break;
      default:
        // FIXME: unknown / unhandled type
        break;
    }
  }
}
