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

void check_reg_idx(uint8_t idx, uint16_t nregs) {
  if(idx > nregs) {
    printf("reg index %d(%x) is above the maximum allowed\n", idx, idx);
    exit(-1);
  }
}

void op_loadi_n(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr, uint8_t inst) {
  uint8_t imm_val = inst - OP_LOADI_0;
  uint8_t reg_index = bytecode[*pc_ptr];
  // printf("pc_ptr intval %d\n", *pc_ptr);

  // TODO: raise error
  check_reg_idx(reg_index, vm->irep.nregs);
  // printf("inst %d\n", inst);
  // printf("imm_val %d\n", imm_val);
  // printf("bc[pc_ptr] %d\n", bytecode[*pc_ptr]);
  // printf("OP_LOADI__1 %d\n", OP_LOADI__1);
  //printf("loading %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].intval = imm_val;
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
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].intval = imm_val;
}

void op_loadineg(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr, uint8_t inst) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("loadi-ing %d to reg %d\n", imm_val, reg_index);
  vm->regs[reg_index].type = T_INT;
  vm->regs[reg_index].intval = -imm_val;
}

mrbz_val *op_return(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  // printf("reg_index is: %d\n", reg_index);

  // TODO: raise error
  check_reg_idx(reg_index, vm->irep.nregs);

  // printf("vm->r[ri]: %d\n",vm->regs[reg_index]);
  // printf("returning type %d\n", vm->regs[reg_index].type);
  mrbz_val* foo = vm->regs + reg_index;
  // printf("returning %s\n", foo->strval);
  return vm->regs + reg_index;
}

void op_move(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t dest_reg = next_byte(bytecode, pc_ptr);
  uint8_t src_reg = next_byte(bytecode, pc_ptr);
  //printf("moving. reg %d to %d, intval is %d\n", src_reg, dest_reg, vm->regs[src_reg]);
  check_reg_idx(dest_reg, vm->irep.nregs);
  check_reg_idx(src_reg, vm->irep.nregs);
  // TODO: check this assignment is valid with the compiler
  vm->regs[dest_reg] = vm->regs[src_reg];
}

void op_add(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("adding. reg %d to %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index+1].intval);
  vm->regs[reg_index].intval += vm->regs[reg_index + 1].intval;
}

void op_addi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("adding. %d to reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval += imm_val;
}

void op_sub(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("subtracting. reg %d from %d, values are %d and %d\n", reg_index + 1, reg_index, vm->regs[reg_index + 1].intval, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval -= vm->regs[reg_index + 1].intval;
}

void op_subi(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t imm_val = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("subtracting. %d from reg %d, reg intval is %d\n", imm_val, reg_index, vm->regs[reg_index].intval);
  vm->regs[reg_index].intval -= imm_val;
}

void op_mul(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("multiplying. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].intval *= vm->regs[reg_index + 1].intval;
}

void op_div(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // register(a)'s intval + register(a+1)'s intval in register(a)
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  //printf("dividing. reg %d by %d, values are %d and %d\n", reg_index, reg_index + 1, vm->regs[reg_index].intval, vm->regs[reg_index + 1].intval);
  vm->regs[reg_index].intval /= vm->regs[reg_index + 1].intval;
}

void op_loadnil(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  vm->regs[reg_index].type = T_NIL;
}

void* mrbz_irep_pool_entry_ptr(mrbz_irep* irep_p, uint8_t idx) {
  // TODO: pool is in RITE binary. Pool belongs to IREP
  // printf("mrbz_irep_pool_entry_ptr: %d\n", irep_p->pool+idx);
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
      printf("Unsupported IREP_TT in constant pool: %x\n", *irep_p->pool);
      exit(-1);
    };
    idx--;
  }

  return pool_cpy;
}

void op_string(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);

  uint8_t pool_index = next_byte(bytecode, pc_ptr);
  void* pool_entry_base = mrbz_irep_pool_entry_ptr(vm->irep, pool_index);
  vm->regs[reg_index].type = T_STRING;

  // TODO: Add length to mrbz value, and copy length
  uint16_t str_len = (uint16_t)pool_entry_base[1];
  // TODO: seems like my demo runs out of RAM? Let's point this to static str...
  //vm->regs[reg_index].strval = malloc(str_len * sizeof(char) + 1);
  vm->regs[reg_index].strval = pool_entry_base + 3;
  // + 1 is there for now to null-terminate the string
  // printf("copying string: %s\n", pool_entry_base+5);
  // printf("regs: %x\n", vm->regs);
  // printf("dest: %x\n", vm->regs[reg_index].strval);
  // strncpy(vm->regs[reg_index].strval, pool_entry_base + 5, str_len + 1);
  // printf("copied string: %s\n", vm->regs[reg_index].strval);
}

void op_gt(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  // TODO: generic object support
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  check_reg_idx(reg_index, vm->irep.nregs);
  // Only supports integer for now
  if(vm->regs[reg_index].type == T_INT && vm->regs[reg_index+1].type == T_INT) {
    // TODO: handle freeing... GCing...
    if(vm->regs[reg_index].intval > vm->regs[reg_index+1].intval) {
      vm->regs[reg_index].type = T_TRUE;
    } else {
      vm->regs[reg_index].type = T_FALSE;
    }
  } else {
    printf("non-integer comparison not supported\n");
    exit(-1);
  }
}

void op_jmpnot(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode,  pc_ptr);
  int16_t jump_by = next_word(bytecode, pc_ptr);

  if(vm->regs[reg_index].type == T_NIL || vm->regs[reg_index].type == T_FALSE) {
    *pc_ptr += jump_by;
  }
}

void op_jmp(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  int16_t jump_by = next_word(bytecode, pc_ptr);

  *pc_ptr += jump_by;
}

void op_ssend(mrbz_vm *vm, unsigned char* bytecode, uint16_t* pc_ptr) {
  uint8_t reg_index = next_byte(bytecode, pc_ptr);
  uint8_t sym_index = next_byte(bytecode, pc_ptr);
  uint8_t arg_info = next_byte(bytecode, pc_ptr); // mostly ignored

  // FIXME: right now, shortcutting due to time constraints.
  // sym index 0 is automatically "puts"
  if(sym_index != 0) {
    printf("method call invoked with unsupported symbol index\n");
    exit(-1);
  }
  if(arg_info != 1) {
    printf("method call invoked with unsupported args\n");
    exit(-1);
  }

  // FIXME: even worse, but this will do for now :P
  // puts is called with R1 as arg, so R[1+1] is the arg sent to puts
  fprintf(stdout, "%s", vm->regs[reg_index+1].strval);
}

void mrbz_vm_run(mrbz_vm *vm, mrbz_val* rval, unsigned char* bytecode) {
  // TODO: check more header vaules
  // we know header length is 20, so skip for now
  uint16_t pc = 20;
  // to view contents
  // for(uint16_t cp = pc; cp < 100; cp ++) {
    // printf("bytecode at %d: %d\n", cp, bytecode[cp]);
  // }
  pc += 4; // TODO: skipping "IREP"

  // TODO: it's 4 bytes..
  // BUT! uint32_t is not supported?
  uint16_t section_len = bytecode[pc+3] |
                 (bytecode[pc+2] << 8) ;
                 // ((uint32_t)bytecode[pc+1] << 16) |
                 // ((uint32_t)bytecode[pc] << 24);
  printf("section length: %d / 0x%x\n", section_len, section_len);
  pc += 4; // 4 bytes for the section length
  pc += 4; // TODO: skipping "0300"
  uint16_t record_len = bytecode[pc+3] |
                 (bytecode[pc+2] << 8) ;
  printf("irep rec len: %d / 0x%x\n", record_len, record_len);
  pc += 4; // skipping 4 bytes for irep record size

  uint16_t nlocals = (bytecode[pc] << 8) |
                 bytecode[pc+1];
  printf("nlocals is %d / 0x%x\n", nlocals, nlocals);
  pc += 2;
  vm->irep.nregs = (bytecode[pc] << 8) |
                 bytecode[pc+1];
  printf("nregs is %d / 0x%x\n", vm->irep.nregs, vm->irep.nregs);
  pc += 2;
  // TODO: rlen, clen...
  printf("rlen is: %d / 0x%x\n", bytecode[pc], bytecode[pc]);
  pc += 2 ;
  printf("clen is: %d / 0x%x\n", bytecode[pc], bytecode[pc]);
  pc += 2;
  // TODO: it's 4 bytes..
  // BUT! uint32_t is not supported?
  uint16_t ilen = bytecode[pc+3] |
                 (bytecode[pc+2] << 8) ;
                 // ((uint32_t)bytecode[pc+1] << 16) |
                 // ((uint32_t)bytecode[pc] << 24);
  printf("ilen: %d / 0x%x\n", ilen, ilen);
  pc += 4; // skip 4 bytes for the ilen

  vm->irep.pool = bytecode + pc + ilen;
  printf("bytecode ptr: %d / 0x%x\n", (uint16_t)(bytecode), (uint16_t)(bytecode));
  printf("pc: %d / 0x%x\n", pc, pc);
  printf("pool ptr: %d / 0x%x\n", (uint16_t)(vm->irep.pool), (uint16_t)(vm->irep.pool));

  const uint8_t *pool_ptr = vm->irep.pool;
  printf("pool_ptr[0] raw value: %d / 0x%x\n", pool_ptr[0], pool_ptr[0]);
  printf("pool_ptr[1] raw value: %d / 0x%x\n", pool_ptr[1], pool_ptr[1]);

  uint16_t temp = pool_ptr[0];
  printf("temp (before shift): 0x%x\n", temp);
  temp = temp << 8;
  printf("temp (after shift): 0x%x\n", temp);

  uint16_t plen_l = (uint16_t)(pool_ptr[0] << 8);
  printf("plen_l: 0x%x\n", plen_l);
  printf("(pool ptr[0]) << 8: %d / 0x%x\n", (uint16_t)(pool_ptr[0] << 8), (uint16_t)(pool_ptr[0] << 8));
  //uint16_t plen = (pool_ptr[0] << 8) | pool_ptr[1];
  uint16_t plen_r = (uint16_t)pool_ptr[1];
  printf("plen_l: 0x%x, plen_r: 0x%x\n", plen_l, plen_r );
  uint16_t plen = plen_l | plen_r;
  pool_ptr += 2;  // skip plen
  printf("plen is: %d / 0x%x\n", plen, plen);

  // plen by pos
  uint16_t plen1 = (bytecode[pc+ilen] << 8) | bytecode[pc+ilen+1];
  printf("plen1 ptr is: %d / 0x%x\n", &(bytecode[pc+ilen]), &(bytecode[pc+ilen]));
  printf("plen1 is: %d / 0x%x\n", plen1, plen1);

  // bytes
  printf("*(pool ptr): %d / 0x%x\n", (uint16_t)(pool_ptr[0]), (uint16_t)(pool_ptr[0]));
  printf("*(plen1 ptr): %d / 0x%x\n", (bytecode[pc+ilen]), (bytecode[pc+ilen]));
  printf("*(pool ptr+1): %d / 0x%x\n", (uint16_t)(pool_ptr[1]), (uint16_t)(pool_ptr[1]));
  printf("*(plen1 ptr+1): %d / 0x%x\n", (bytecode[pc+ilen+1]), (bytecode[pc+ilen+1]));

  // Iterate through all pool entries
  for(uint16_t i = 0; i < plen; i++) {
    uint16_t str_len;
    uint8_t pool_type = *pool_ptr;
    pool_ptr++;  // skip type byte
  printf("i: %d, type: 0x%x\n", i, pool_type);

    switch(pool_type) {
      case IREP_TT_STR:
      case IREP_TT_SSTR:
        str_len = (*pool_ptr << 8) | *(pool_ptr + 1);
        pool_ptr += 2;  // skip length bytes
        pool_ptr += str_len + 1;  // skip string data + null terminator
        printf("str pool type: %x\n", pool_type);
        break;

      case IREP_TT_INT32:
        pool_ptr += 4;  // skip 4 bytes for int32
        printf("int pool type\n");
        break;

      default:
        printf("Unknown pool type: %x\n", pool_type);
        exit(-1);
    }
  }

  // printf("bytecode start is: %d, pc (iseq start index) is: %d, iseq length is: %d\n", bytecode, pc, ilen);

  // initialise R0 with an object
  vm->regs[0].type = T_OBJECT;

  // while instructions are left
  int exiting = 0;
  mrbz_val *retval = NULL;

  while (!exiting) {
    uint8_t instruction;
    instruction = bytecode[pc++];
    // printf("PC: %d(%x), OP: %s\n", pc, pc, op_names[instruction]);
    switch(instruction) {
      case OP_NOP: break;
      case OP_MOVE: op_move(vm, bytecode, &pc); break;
      case OP_LOADI: op_loadi(vm, bytecode, &pc, instruction); break;
      case OP_LOADINEG: op_loadineg(vm, bytecode, &pc, instruction); break;
      case OP_LOADI_0: // fall through
      case OP_LOADI_1: // fall through
      case OP_LOADI_2: // fall through
      case OP_LOADI_3: // fall through
      case OP_LOADI_4: // fall through
      case OP_LOADI_5: // fall through
      case OP_LOADI_6: // fall through
      case OP_LOADI_7: op_loadi_n(vm, bytecode, &pc, instruction); break;
      case OP_LOADNIL: op_loadnil(vm, bytecode, &pc); break;
      case OP_JMP: op_jmp(vm, bytecode, &pc); break;
      case OP_JMPNOT: op_jmpnot(vm, bytecode, &pc); break;
      case OP_SSEND: op_ssend(vm, bytecode, &pc); break;
      // FIXME: Currently this return exits from any level...
      case OP_RETURN: retval = op_return(vm, bytecode, &pc); exiting = 1; break;
      case OP_ADD: op_add(vm, bytecode, &pc); break;
      case OP_ADDI: op_addi(vm, bytecode, &pc); break;
      case OP_SUB: op_sub(vm, bytecode, &pc); break;
      case OP_SUBI: op_subi(vm, bytecode, &pc); break;
      case OP_MUL: op_mul(vm, bytecode, &pc); break;
      case OP_DIV: op_div(vm, bytecode, &pc); break;
      case OP_GT: op_gt(vm, bytecode, &pc); break;
      case OP_STRING: op_string(vm, bytecode, &pc); break;
      case OP_STOP: exiting = 1; break;
      default:
        printf("unsupported instruction: 0x%x\n", instruction);
        printf("OP: %s\n", op_names[instruction]);
        break;
    }
  }

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
      case T_TRUE:
      case T_FALSE:
        // these types don't have values - only types. Fall through
      default:
        // FIXME: unknown / unhandled type
        break;
    }
  }
}
