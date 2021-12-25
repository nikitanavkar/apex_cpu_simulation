/*
 * apex_macros.h
 * Contains APEX cpu pipeline macros
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _MACROS_H_
#define _MACROS_H_

#define FALSE 0x0
#define TRUE 0x1

/* Integers */
#define DATA_MEMORY_SIZE 4096

/* Size of integer register file */
#define REG_FILE_SIZE 16

/* Numeric OPCODE identifiers for instructions */
#define OPCODE_ADD 0x0
#define OPCODE_SUB 0x1
#define OPCODE_MUL 0x2
#define OPCODE_DIV 0x3
#define OPCODE_AND 0x4
#define OPCODE_OR 0x5
#define OPCODE_XOR 0x6
#define OPCODE_MOVC 0x7
#define OPCODE_LOAD 0x8
#define OPCODE_STORE 0x9
#define OPCODE_BZ 0xa
#define OPCODE_BNZ 0xb
#define OPCODE_HALT 0xc
#define OPCODE_ADDL 0xd
#define OPCODE_SUBL 0xe
#define OPCODE_LDI 0xf
#define OPCODE_STI 0x10
#define OPCODE_BP 0x11
#define OPCODE_BNP 0x12
#define OPCODE_CMP 0x13
#define OPCODE_NOP 0x14
#define OPCODE_JUMP 0x15

#define FETCH_STAGE 0x00
#define DECODE_STAGE 0x01
#define EXECUTE_STAGE 0x02
#define MEMORY_STAGE 0x03
#define WRITE_BACK_STAGE 0x04

#define COMMAND_DISPLAY 0
#define COMMAND_SIMULATE 1
#define COMMAND_SINGLE_STEP 2
#define COMMAND_SHOW_MEMORY 3

#define ENABLE_DEBUG_MESSAGES 0
#define ENABLE_SINGLE_STEP 1
#define DISABLE_SINGLE_STEP 0

#endif
