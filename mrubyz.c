#include "mrubyz.h"
#include "opcode.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void op_loadi_n(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p, uint8_t inst) {
	uint8_t imm_val = inst - OP_LOADI_0;
	uint8_t reg_index = bytecode[*curr_p] - 1;
  // printf("curr_p value %d\n", *curr_p);

	// TODO: raise error
	if(reg_index >= 4) {
		// printf("register %d not available\n", reg_index);
		exit(-1);
	} else {
		// printf("inst %d\n", inst);
		// printf("imm_val %d\n", imm_val);
		// printf("bc[curr_p] %d\n", bytecode[*curr_p]);
		// printf("OP_LOADI__1 %d\n", OP_LOADI__1);
		vm->regs[reg_index] = imm_val;
    curr_p++;
	}
}

int op_return(mrbz_vm *vm, unsigned char* bytecode, uint16_t* curr_p) {
	uint8_t reg_index = bytecode[*curr_p] - 1;
	curr_p++;

	// TODO: raise error
	if(reg_index >= 4) {
		printf("register %d not available\n", reg_index);
		exit(-1);
	} else {
		// printf("vm->r[ri]: %d\n", vm->regs[reg_index]);
		return vm->regs[reg_index];
	}
}

void mrbz_vm_run(mrbz_vm *vm, mrbz_val* rval, unsigned char* bytecode) {
  // TODO: check header
  // we know header length is 20, so skip for now
  uint16_t curr = 20;
  // printf("content: %.4s\n", (bytecode+curr));
	curr += 4; // TODO: skipping IREP

	// TODO: it's 4 bytes..
	// BUT! uint32_t is not supported?
  uint16_t section_len = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
                 // ((uint32_t)bytecode[curr+1] << 16) |
                 // ((uint32_t)bytecode[curr] << 24);
  //printf("length: %d\n", len)
	curr += 4;
	curr += 4; // TODO: skipping "0300"
  uint16_t record_len = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
  // printf("irep rec len: %d\n", record_len);
	curr += 4;

	// TODO: skipping...
  curr += 2 + 2 + 2 + 2 ;
	// TODO: it's 4 bytes..
	// BUT! uint32_t is not supported?
  uint16_t ilen = bytecode[curr+3] |
                 (bytecode[curr+2] << 8) ;
                 // ((uint32_t)bytecode[curr+1] << 16) |
                 // ((uint32_t)bytecode[curr] << 24);
  // printf("ilen: %d\n", ilen);
	curr += 4;


  // while instructions are left
	int exiting = 0;
	int retval = -1;

  while (!exiting) {
		uint8_t instruction;
		instruction = bytecode[curr++];
		switch(instruction) {
			case OP_LOADI_0: // fall through
			case OP_LOADI_1: // fall through
			case OP_LOADI_2: // fall through
			case OP_LOADI_3: // fall through
			case OP_LOADI_4: // fall through
			case OP_LOADI_5: // fall through
			case OP_LOADI_6: // fall through
			case OP_LOADI_7: op_loadi_n(vm, bytecode, &curr, instruction); break;
      // FIXME: Currently this return exits from any level...
			case OP_RETURN: retval = op_return(vm, bytecode, &curr); exiting = 1; break;
			case OP_STOP: exiting = 1; break;
		}
	}
    // pop off first
    // execute

  // printf("retval: %d\n", retval);
  // set the return val
  rval->type = 0;
  rval->value = retval;
}
