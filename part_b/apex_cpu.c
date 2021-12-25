/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 * 
 * This file is updated for data forwarding (Part B) logic by Nikita Navkar
 */  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apex_cpu.h"
#include "apex_macros.h"
// Initalization
int command;
int cycle_count;
CPU_Stage pipeline_logs[5]; 
/* 
 * To Display Memory Content
 * Note: You can edit this function to print in more detail
 */
static void 
show_memory(const APEX_CPU *cpu)
{
    printf("\n============================== STATE OF DATA MEMORY ===============================\n\n");
    for(int cnt = 0; cnt < DATA_MEMORY_SIZE; cnt++) 
    {
        printf("|           MEM[%04d]           |               Data Value = %d             \n", cnt, cpu->data_memory[cnt]);
    }
}

static int 
map_commands(char const *arguments[])
{
    int ret_val = FALSE;
    if(arguments[2])
    {
        if (strcmp(arguments[2], "simulate") == 0)
        {
            if(arguments[3])
            {
                command = COMMAND_SIMULATE;
                cycle_count = atoi(arguments[3]);
                ret_val = TRUE;
            }
        }
        else if (strcmp(arguments[2], "display") == 0)
        {
            if(arguments[3])
            {
                command = COMMAND_DISPLAY;
                cycle_count = atoi(arguments[3]);
                ret_val = TRUE;
            }
        }
        else if (strcmp(arguments[2], "single_step") == 0)
        {
            command = COMMAND_SINGLE_STEP;
            ret_val = TRUE;
        }
        else if (strcmp(arguments[2], "show_mem") == 0)
        {
            if(arguments[3])
            {
                command = COMMAND_SHOW_MEMORY;
                cycle_count = atoi(arguments[3]);
                ret_val = TRUE;
            }
        }
        else 
        {
            printf("Command not found exiting - %s\n", arguments[1]);
        }
    }

    return ret_val;
}

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}
// Print opcode string, source regiters, destination register and literal value for all input instruction
static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LDI:
        case OPCODE_LOAD:
        {
           printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break; 
        }
         case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }
        case OPCODE_STI:
        {
           printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break; 
        }
        case OPCODE_MOVC:
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }
        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        case OPCODE_HALT:
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }
    }
}

/* This function will prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const APEX_CPU *cpu)
{
    for(int cnt = 0; cnt < 5; cnt++)
    {
        char *stage_name;
        switch (cnt)
        {
        case FETCH_STAGE:
            stage_name = "FETCH_____STAGE";
            break;

        case DECODE_STAGE:
            stage_name = "DECODE_RF_STAGE";
            break;

        case EXECUTE_STAGE:
            stage_name = "EX________STAGE";
            break;

        case MEMORY_STAGE:
            stage_name = "MEMORY____STAGE";
            break;

        case WRITE_BACK_STAGE:
            stage_name = "WRITEBACK_STAGE";
            break;
        default:
            break;
        }
        if(pipeline_logs[cnt].has_insn)
        {
            int I = (pipeline_logs[cnt].pc - 4000) / 4;

            printf("%d. Instruction at %s ---->        ", cnt+1, stage_name);

            printf("(I%d: %d) ", I, pipeline_logs[cnt].pc);

            print_instruction(&pipeline_logs[cnt]);

            pipeline_logs[cnt].has_insn = 0;
        }
        else
        {
            printf("%d. Instruction at %s --->         EMPTY", cnt+1, stage_name);
        }
        printf("\n");
    }
}

/* This function will print the register files
 *
 * Note: You are not supposed to edit this function
 */
static void
show_register_files(const APEX_CPU *cpu)
{
    printf("\n=============================== STATE OF ARCHITECTURAL REGISTER FILE =============================\n\n");
    
    for(int cnt = 0; cnt < REG_FILE_SIZE; cnt++)
    {
        if(cpu->state[cnt])
        {
            printf("|           REG[%04d]           |           Value = %4d|            Status = INVALID      |\n", cnt, cpu->regs[cnt]);
        }
        else
        {
            printf("|           REG[%04d]           |           Value = %4d|            Status = VALID        |\n", cnt, cpu->regs[cnt]);
        }
    }
}


/*
 * Note: You can edit this function to print in more detail
 */ 
 /* This function will the pipeline data for each command
 */
static void 
print_pipeline_logs(const APEX_CPU *cpu)
{
    switch (command)
    {
        case COMMAND_SIMULATE:
            {
                show_register_files(cpu);
                show_memory(cpu);
                if(cpu->clock >= cycle_count)
                {
                    printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                    exit(1);
                }
            }
            break;
        case COMMAND_DISPLAY:
            {
                if(cpu->clock >= cycle_count)
                {
                    show_register_files(cpu);
                    show_memory(cpu);
                    printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                    exit(1);
                }
                else
                {
                    printf("\n............................. CLOCK CYCLE %d ..............................\n\n", cpu->clock+1);
                    print_stage_content(cpu);
                }
            }
            break;
    
        case COMMAND_SINGLE_STEP:
            {
                printf("\n.......................... CLOCK CYCLE %d ...........................\n\n", cpu->clock+1);
                print_stage_content(cpu);
            }
            break;
    
        default:
            break;
    }
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;
    if(cpu->halt != cpu->halted)
    {
        if(cpu->halt)
        {
            /* Store current PC in fetch latch */
            cpu->fetch.pc = cpu->pc;

            /* Index into code memory using this pc and copy all instruction fields
            * into fetch latch  */
            current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
            strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
            cpu->fetch.opcode = current_ins->opcode;
            cpu->fetch.rd = current_ins->rd;
            cpu->fetch.rs1 = current_ins->rs1;
            cpu->fetch.rs2 = current_ins->rs2;
            cpu->fetch.imm = current_ins->imm;

            /* Update PC for next instruction */
            cpu->pc += 4;

            pipeline_logs[FETCH_STAGE] = cpu->fetch;
            
            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->halt_instruction_received = TRUE;
            } 
        }
        else
        {
            cpu->decode = cpu->fetch;
            pipeline_logs[FETCH_STAGE] = cpu->fetch;
            if(cpu->halt_instruction_received)
            {
                cpu->halt_instruction_received = FALSE;
                cpu->fetch.has_insn = FALSE;
                return;
            }        
        }
    }
    else if(!cpu->halt)
    {
        if (cpu->fetch.has_insn)
        {
            /* This fetches new branch target instruction from next cycle */
            if (cpu->fetch_from_next_cycle == TRUE)
            {
                cpu->fetch_from_next_cycle = FALSE;
                /* Skip this cycle*/
                return;
            }

            /* Store current PC in fetch latch */
            cpu->fetch.pc = cpu->pc;

            /* Index into code memory using this pc and copy all instruction fields
             * into fetch latch  */
            current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
            strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
            cpu->fetch.opcode = current_ins->opcode;
            cpu->fetch.rd = current_ins->rd;
            cpu->fetch.rs1 = current_ins->rs1;
            cpu->fetch.rs2 = current_ins->rs2;
            cpu->fetch.imm = current_ins->imm;

            /* Update PC for next instruction */
            cpu->pc += 4;

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;

            /* Copy data from fetch latch to debug fetch latch*/
            pipeline_logs[FETCH_STAGE] = cpu->fetch;

            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->fetch.has_insn = FALSE;
            }
        }   
    }
    else
    {
        pipeline_logs[FETCH_STAGE] = cpu->fetch;
    }
    cpu->halted = cpu->halt;
}


/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {   
                
                if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }
            if (!(cpu->halt))
            {
                 if (cpu->decode.rs2==cpu->exe_dest_reg) 
                {
                    cpu->decode.rs2_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs2==cpu->mem_dest_reg)
                {
                    cpu->decode.rs2_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if(cpu->state[cpu->decode.rs2])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                   cpu->halt=0;
                }
            }
             
            if (!(cpu->halt))
            {
                if ((cpu->decode.rs1==cpu->decode.rd)||(cpu->decode.rs2==cpu->decode.rd))
                {
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rd])
                {
                    cpu->halt=1;
                }
                else
                {
                    cpu->state[cpu->decode.rd] = 1;
                    cpu->halt=0;
                }
            }
            
                break;
                    
              
            }
            case OPCODE_CMP:
            {
                if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }
                if (!(cpu->halt))
                {
                    if (cpu->decode.rs2==cpu->exe_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->execute.dataforwarding_exe_buffer;
                        cpu->halt=0;   
                    }
                    else if (cpu->decode.rs2==cpu->mem_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->memory.result_buffer;
                        cpu->halt=0;
                    }
                    else if(cpu->state[cpu->decode.rs2])
                    {
                        cpu->halt=1;
                    }
                    else
                    {
                        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        cpu->halt=0;
                    }
                }
                break;
            }
            case OPCODE_STI:
            {   if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }
                if (!(cpu->halt))
                {
                    if (cpu->decode.rs2==cpu->exe_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->execute.dataforwarding_exe_buffer;
                        cpu->halt=0;   
                    }
                    else if (cpu->decode.rs2==cpu->mem_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->memory.dataforwarding_mem_buffer;
                        cpu->halt=0;
                    }
                    else if(cpu->state[cpu->decode.rs2])
                    {
                        cpu->halt=1;
                    }
                    else
                    {
                        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        cpu->halt=0;
                    }

                    cpu->state[cpu->decode.rs2] = 1;

                }

                break;
            }
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:
            {   
                if (cpu->decode.rs1==cpu->decode.rd)
                {
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rd])
                {
                    cpu->halt=1;
                }
                else
                {
                    cpu->state[cpu->decode.rd] = 1;
                    cpu->halt=0;
                }
                if (!(cpu->halt))
                {
                    if (cpu->decode.rs1==cpu->exe_dest_reg)
                    {
                        cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                        cpu->halt=0;   
                    }
                    else if (cpu->decode.rs1==cpu->mem_dest_reg)
                    {
                        cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                        cpu->halt=0;
                    }
                    else if (cpu->state[cpu->decode.rs1])
                    {
                        cpu->halt=1;
                    }
                    else
                    {
                        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                        cpu->halt=0;
                    }
                }
                break;
            }

            case OPCODE_LDI:
            {
                if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }

                if (!(cpu->halt))
                {
                    if(cpu->state[cpu->decode.rd])
                    {
                        cpu->halt=1;
                    }
                    else
                    {
                        cpu->state[cpu->decode.rd] = 1;
                        cpu->halt=0;
                    }
                    cpu->state[cpu->decode.rs1]=1;
                }
                break;

            }
            case OPCODE_STORE:
            {
                if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.dataforwarding_exe_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if (cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }
                if (!(cpu->halt))
                {
                    if (cpu->decode.rs2==cpu->exe_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->execute.result_buffer;
                        cpu->halt=0;   
                    }
                    else if (cpu->decode.rs2==cpu->mem_dest_reg)
                    {
                        cpu->decode.rs2_value = cpu->memory.dataforwarding_mem_buffer;
                        cpu->halt=0;
                    }
                    else if(cpu->state[cpu->decode.rs2])
                    {
                        cpu->halt=1;
                    }
                    else
                    {
                        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        cpu->halt=0;
                    }
                }
                    break;
            }
            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                
                if(cpu->state[cpu->decode.rd])
                {
                    cpu->halt=1;
                }
                else{
                cpu->state[cpu->decode.rd] = 1;
                cpu->halt=0;
                }
                break;
            }
            case OPCODE_JUMP:
            {
                if (cpu->decode.rs1==cpu->exe_dest_reg)
                {
                    cpu->decode.rs1_value = cpu->execute.result_buffer;
                    cpu->halt=0;   
                }
                else if (cpu->decode.rs1==cpu->mem_dest_reg)
                {
                    cpu->decode.rs2_value = cpu->memory.dataforwarding_mem_buffer;
                    cpu->halt=0;
                }
                else if(cpu->state[cpu->decode.rs1])
                {
                    cpu->halt=1;
                }
                else
                {
                   cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                   cpu->halt=0;
                }
                break;
            }
                       
        }

      pipeline_logs[DECODE_STAGE] = cpu->decode;

        if(!cpu->halt)
        {
            /* Copy data from decode latch to execute latch*/
            cpu->execute = cpu->decode;
            cpu->decode.has_insn = FALSE;

        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                /* Set the zero flag or positive flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }
             case OPCODE_SUB:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                /* Set the zero flag or positive flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;

                /* Set the zero flag or positive flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;

                /* Set the zero flag or positive flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }
            case OPCODE_MUL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;

                /* Set the zero flag or positive flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }
            case OPCODE_DIV:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value / cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            }
             case OPCODE_AND:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                
                break;
            }
            case OPCODE_OR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                
                break;
            }
            case OPCODE_XOR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;              
                break;
            }
            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }
            case OPCODE_STORE:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                    break;

            }
            case OPCODE_LDI:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.register_buffer
                    = cpu->execute.rs1_value +4;
                cpu->exe_dest_reg=cpu->execute.rs1;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.register_buffer;    
                break;
            }
            case OPCODE_STI:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                cpu->execute.register_buffer
                    = cpu->execute.rs2_value +4;
                cpu->exe_dest_reg=cpu->execute.rs2;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.register_buffer;
                break;

            }
            case OPCODE_BZ:
            {   
                printf("cpu->zero_flag %d",cpu->zero_flag);
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BP:
            {
                if (cpu->positive_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
             case OPCODE_BNP:
            {
                if (cpu->positive_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_MOVC: 
            {   
                cpu->execute.result_buffer = cpu->execute.imm;
                cpu->exe_dest_reg=cpu->execute.rd;
                cpu->execute.dataforwarding_exe_buffer=cpu->execute.result_buffer;
                break;
            }
            case OPCODE_CMP:
            {
                /* Set the zero flag or positive flag based on the comparison */
                if (cpu->execute.rs1_value == cpu->execute.rs2_value)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                if (cpu->execute.rs1_value > cpu->execute.rs2_value)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }
                break;
            } 
            case OPCODE_JUMP:
            {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                
                break;
            }        

        }

        pipeline_logs[EXECUTE_STAGE] = cpu->execute;
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUB:
            case OPCODE_SUBL:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            {
                cpu->mem_dest_reg=cpu->memory.rd;
                cpu->execute.dataforwarding_mem_buffer=cpu->memory.result_buffer;
                break;
            }
            case OPCODE_LOAD:
            case OPCODE_LDI:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                cpu->mem_dest_reg=cpu->memory.rd;
                cpu->execute.dataforwarding_mem_buffer=cpu->memory.result_buffer;
                break;
            }
            case OPCODE_STORE:
            case OPCODE_STI:
            {  
                /* Read from data memory */
               cpu->data_memory[cpu->memory.memory_address]= cpu->memory.rs1_value;
               cpu->mem_dest_reg=cpu->memory.rd;
               cpu->execute.dataforwarding_mem_buffer=cpu->memory.result_buffer;
               break;
            }
        }

        pipeline_logs[MEMORY_STAGE] = cpu->memory;
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUB:
            case OPCODE_SUBL:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:

            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->state[cpu->writeback.rd]=0;
                break;
            }
            
            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->state[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_LDI:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.register_buffer;
                cpu->state[cpu->writeback.rd] = 0;
                cpu->state[cpu->writeback.rs1] = 0;
                break;
            }
            case OPCODE_STI:
            {  
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.register_buffer;
                cpu->state[cpu->writeback.rs2] = 0;
                break;
            }
           case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->state[cpu->writeback.rd] = 0;
                break;
            }
        }
        pipeline_logs[WRITE_BACK_STAGE] = cpu->writeback;
        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;
        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *arguments[])
{
    APEX_CPU *cpu;
    if (!arguments[1])
    {
        return NULL;
    }
    cpu = calloc(1, sizeof(APEX_CPU));
    if (!cpu)
    {
        return NULL;
    }
    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->state, 0, sizeof(unsigned char) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(pipeline_logs, 0, 5 * sizeof(CPU_Stage));

    if(!map_commands(arguments))
    {
        free(cpu);
        return NULL;  
    }
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(arguments[1], &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }
    cpu->halt=0;
    cpu->exe_dest_reg=16;
    cpu->mem_dest_reg=16;
    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (int i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        } 
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    while (TRUE)
    {
        if (APEX_writeback(cpu))
        {
            switch (command)
            {
            case COMMAND_SIMULATE:
                    print_pipeline_logs(cpu); 
                break;
            
            case COMMAND_DISPLAY:
            case COMMAND_SINGLE_STEP:
                    print_pipeline_logs(cpu);
                    show_register_files(cpu);
                    show_memory(cpu);
                break;

            case COMMAND_SHOW_MEMORY:
                    printf("|         MEM[%04d]         |       Data Value = %d           \n", cycle_count, cpu->data_memory[cycle_count]);
                break;

            default:
                break;
            }
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_pipeline_logs(cpu);

        if (command == COMMAND_SINGLE_STEP)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                show_register_files(cpu);
                show_memory(cpu);
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}