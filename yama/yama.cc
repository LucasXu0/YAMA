//
//  yama.c
//  yama
//
//  Created by xurunkang on 2021/11/30.
//

#include "yama.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "stack_logging.h"
#ifdef __cplusplus
}
#endif
#include <mach/mach_init.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <unordered_map>
#include <mach/task.h>
#include <mach-o/dyld.h>

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

static yama_logging_context_t logging_context = {};
static struct backtrace_uniquing_table *table;

static FILE *logging_file;
static FILE *serialize_table_file;
static FILE *header_file;

int yama_initialize(void)
{
    return turn_on_stack_logging(stack_logging_mode_all);
}

extern void yama_prepare_logging(yama_logging_context_t *context)
{
    logging_context = *context;
    logging_file = fopen(context->output_logging_file_path, "w+");
    serialize_table_file = fopen(context->output_serialize_table_file_path, "w+");
    header_file = fopen(context->output_headers_file_path, "w+");
}

void enumerator(mach_stack_logging_record_t record, void *context)
{
    fprintf(logging_file, "[%s] 0x%llx, %lld, %llx\n", readable_type_flags(record.type_flags), record.address, record.argument, record.stack_identifier);
    uint32_t out_frams_count = 512;
    mach_vm_address_t *out_frames_buffer = (mach_vm_address_t *)malloc(sizeof(mach_vm_address_t) * out_frams_count);
    __mach_stack_logging_uniquing_table_read_stack((struct backtrace_uniquing_table *)context,
                                                   record.stack_identifier, out_frames_buffer,
                                                   &out_frams_count,
                                                   out_frams_count);
    if (out_frams_count) {
        for (int i = 0; i < out_frams_count; i++) {
            mach_vm_address_t frame = out_frames_buffer[i];
            // fprintf(logging_file, "-> 0x%llx\n", frame);
        }
    }
    free(out_frames_buffer);
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
    mach_vm_size_t table_size = 0;
    void *serialize_table = __mach_stack_logging_uniquing_table_serialize(table, &table_size);
    if (table_size) {
        fwrite(serialize_table, sizeof(char), table_size, serialize_table_file);
    }
    return 1;
}

int yama_stop_logging(void)
{
    if (!table) return 0;
    __mach_stack_logging_stop_reading(current_task());
    __mach_stack_logging_uniquing_table_release(table);
    table = NULL;
    fclose(logging_file);
    logging_file = NULL;
    fclose(header_file);
    header_file = NULL;
    fclose(serialize_table_file);
    serialize_table_file = NULL;
    return 1;
}

static int header_count = 0;
void add_image_callback(const struct mach_header *header, intptr_t slide)
{
    if (!header_file) return;
    Dl_info header_info;
    dladdr(header, &header_info);
    // printf("[%03d] 0x%016lx %s\n", header_count++, (unsigned long)header_info.dli_fbase, header_info.dli_fname);
    fprintf(header_file, "[%03d] 0x%016lx %s\n", header_count++, (unsigned long)header_info.dli_fbase, header_info.dli_fname);
}

void dump_headers(void)
{
    _dyld_register_func_for_add_image(add_image_callback);
}

#pragma GCC diagnostic pop
