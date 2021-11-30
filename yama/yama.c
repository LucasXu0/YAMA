//
//  yama.c
//  yama
//
//  Created by xurunkang on 2021/11/30.
//

#include "yama.h"
#include "stack_logging.h"
#import <mach/mach_init.h>
#import <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define checkRet(ret) if (ret != KERN_SUCCESS) return 0

const char *readable_type_flags(uint32_t type_flags)
{
    if (type_flags == stack_logging_type_alloc) {
        return "alloc";
    } else if (type_flags == stack_logging_type_dealloc) {
        return "dealloc";
    } else if (type_flags == stack_logging_type_vm_allocate) {
        return "vm_allocate";
    } else if (type_flags == stack_logging_type_vm_deallocate) {
        return "vm_deallocate";
    } else if (type_flags == stack_logging_type_free) {
        return "free";
    } else if (type_flags == stack_logging_type_generic) {
        return "generic";
    } else if (type_flags == stack_logging_type_mapped_file_or_shared_mem) {
        return "mapped_file_or_shared_mem";
    } else {
        return "unknow";
    }
}

static yama_logging_context_t logging_context = { 0, false };
static struct backtrace_uniquing_table *table;

int yama_initialize(void)
{
    return turn_on_stack_logging(stack_logging_mode_all);
}

void enumerator(mach_stack_logging_record_t record, void *context)
{
    if (!context) return;
    
    if (record.argument <= logging_context.minimum_size) {
        return;
    }
    
    if (logging_context.only_print_alive_only) {
        
    } else {
        printf("[%s] address = 0x%llx, size = %lld\n", readable_type_flags(record.type_flags), record.address, record.argument);
    }
}

extern uint64_t __mach_stack_logging_shared_memory_address;
int yama_start_logging(void)
{
    task_t task = current_task();
    boolean_t lite_mode;
    kern_return_t ret = KERN_SUCCESS;
    ret = __mach_stack_logging_start_reading(task, __mach_stack_logging_shared_memory_address, &lite_mode);
    checkRet(ret);
    table = __mach_stack_logging_copy_uniquing_table(task);
    if (!table) return 0;
    ret = __mach_stack_logging_enumerate_records(task, 0, enumerator, (void *)table);
    checkRet(ret);
    return 1;
}

int yama_stop_logging(void)
{
    if (!table) return 0;
    __mach_stack_logging_stop_reading(current_task());
    __mach_stack_logging_uniquing_table_release(table);
    table = NULL;
    return 1;
}

#pragma GCC diagnostic pop
