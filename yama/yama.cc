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
#include <cstdlib>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define ENABLE_DEBUG_LOG 0

#define checkRet(ret) if (ret != KERN_SUCCESS) return ret

typedef enum : int {
    YAMA_FILE_TYPE_MACH_HEADERS,
    YAMA_FILE_TYPE_RECORDS,
    YAMA_FILE_TYPE_STACKS,
    YAMA_FILE_TYPE_SERIALIZE_TABLE,
    // ...
    YAMA_FILE_TYPE_ALL,
} YAMA_FILE_TYPE;

#pragma mark - Private
static yama_logging_context_t *logging_context = {};
static FILE **yama_files;
static struct backtrace_uniquing_table *table;

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

static inline const char *string_from_YAMA_FILE_TYPE(YAMA_FILE_TYPE type)
{
    static char const *strings[] = {"MACH_HEADER", "RECORDS", "STACKS", "SERIALIZE_TABLE"};
    return strings[type];
}

static inline const char *path_from_YAMA_FILE_TYPE(YAMA_FILE_TYPE type)
{
    const char *file_prefix = "YAMA_FILE_";
    const char *file_type = string_from_YAMA_FILE_TYPE(type);
    char *file_path = (char *)malloc(sizeof(char) * (strlen(file_prefix) + strlen(file_type) + strlen(logging_context->output_dir)));
    strcpy(file_path, logging_context->output_dir);
    strcat(file_path, file_prefix);
    strcat(file_path, file_type);
#if ENABLE_DEBUG_LOG
    printf("[YAMA] file type(%d) => %s\n", type, file_path);
#endif
    return file_path;
}

int initialize_yama_files(void)
{
    if (!logging_context) return YAMA_LOGGING_CONTEXT_IS_NULL;
    
    yama_files = (FILE **)malloc(sizeof(FILE *) * YAMA_FILE_TYPE_ALL);
    for (int i = 0; i < YAMA_FILE_TYPE_ALL; i++) {
        const char *file_path = path_from_YAMA_FILE_TYPE((YAMA_FILE_TYPE)i);
        yama_files[i] = fopen(file_path, "w+");
    }
    
    return YAMA_SUCCESS;
}

void uninitialize_yama_filse(void)
{
    for (int i = 0; i < YAMA_FILE_TYPE_ALL; i++) {
        if (yama_files[i]) {
            fclose(yama_files[i]);
            yama_files[i] = NULL;
        }
    }
}

void inline yama_fwrite(YAMA_FILE_TYPE type, const void * __restrict __ptr, size_t __size, size_t __nitems)
{
    if (yama_files[type]) {
        fwrite(__ptr, __size, __nitems, yama_files[type]);
    }
}

void inline yama_fprintf(YAMA_FILE_TYPE type, const char *format, ...)
{
    if (yama_files[type]) {
        va_list args;
        va_start(args, format);
        vfprintf(yama_files[type], format, args);
        va_end(args);
    }
}

void serialize_table(void)
{
    if (!table) return;
    mach_vm_size_t table_size = 0;
    void *serialize_table = __mach_stack_logging_uniquing_table_serialize(table, &table_size);
    if (table_size) {
        yama_fwrite(YAMA_FILE_TYPE_SERIALIZE_TABLE, serialize_table, sizeof(char), table_size);
    }
}

void enumerator(mach_stack_logging_record_t record, void *context)
{
    if (!context) return;
        
#if ENABLE_DEBUG_LOG
    printf("[YAMA] [%s] %016llx %lld %016llx\n", readable_type_flags(record.type_flags), record.stack_identifier, record.argument, record.address);
#endif
    yama_fprintf(YAMA_FILE_TYPE_RECORDS, "%08d %016llx %lld %016llx\n", record.type_flags, record.stack_identifier, record.argument, record.address);
    
    uint32_t out_frames_count = STACK_LOGGING_MAX_STACK_SIZE;
    mach_vm_address_t *out_frames_buffer = (mach_vm_address_t *)malloc(sizeof(mach_vm_address_t) * out_frames_count);
    __mach_stack_logging_uniquing_table_read_stack((struct backtrace_uniquing_table *)context,
                                                   record.stack_identifier,
                                                   out_frames_buffer,
                                                   &out_frames_count,
                                                   out_frames_count);
    if (out_frames_count) {
        yama_fprintf(YAMA_FILE_TYPE_STACKS, "%016llx", record.stack_identifier);
        for (int i = 0; i < out_frames_count; i++) {
            mach_vm_address_t frame = out_frames_buffer[i];
            if (!frame) continue;
            yama_fprintf(YAMA_FILE_TYPE_STACKS, " %016llx", frame);
#if ENABLE_DEBUG_LOG
            printf("[YAMA] -> %llx\n", frame);
#endif
        }
        yama_fprintf(YAMA_FILE_TYPE_STACKS, "\n");
    } else {
#if ENABLE_DEBUG_LOG
        printf("[YAMA] what? could not find the frames for %lld\n", record.stack_identifier);
#endif
    }
    free(out_frames_buffer);
}

static int header_count = 0;
void add_image_callback(const struct mach_header *header, intptr_t slide)
{
    Dl_info header_info;
    dladdr(header, &header_info);
    if (header_info.dli_fname && strlen(header_info.dli_fname) > 0) {
#if ENABLE_DEBUG_LOG
        printf("[YAMA] [%03d] %016lx %s\n", header_count++, (unsigned long)header_info.dli_fbase, header_info.dli_fname);
#endif
        yama_fprintf(YAMA_FILE_TYPE_MACH_HEADERS, "%016lx %s\n", header_info.dli_fbase, header_info.dli_fname);
    } else {
#if ENABLE_DEBUG_LOG
        printf("[YAMA] what? could not find the name for address(%p)\n", (void *)header);
#endif
    }
}

void dump_mach_headers(void)
{
    _dyld_register_func_for_add_image(add_image_callback);
}

#pragma mark - Public

int yama_initialize(void)
{
    return turn_on_stack_logging(stack_logging_mode_all);
}

int yama_prepare_logging(yama_logging_context_t *context)
{
    logging_context = context;
    int ret = initialize_yama_files();
    return ret;
}

extern uint64_t __mach_stack_logging_shared_memory_address;
int yama_start_logging(void)
{
#if ENABLE_DEBUG_LOG
    printf("[MAYA] start logging\n");
#endif
    task_t task = current_task();
    boolean_t lite_mode;
    kern_return_t ret = KERN_SUCCESS;
    ret = __mach_stack_logging_start_reading(task, __mach_stack_logging_shared_memory_address, &lite_mode);
    checkRet(ret);
    table = __mach_stack_logging_copy_uniquing_table(task);
    if (!table) return YAMA_INIT_TABLE_ERROR;
    ret = __mach_stack_logging_enumerate_records(task, 0, enumerator, (void *)table);
    checkRet(ret);
    dump_mach_headers();
    serialize_table();
    return ret;
}

void yama_stop_logging(void)
{
    __mach_stack_logging_stop_reading(current_task());
    if (table) {
        __mach_stack_logging_uniquing_table_release(table);
        table = NULL;
    }
    uninitialize_yama_filse();
}

#pragma GCC diagnostic pop
