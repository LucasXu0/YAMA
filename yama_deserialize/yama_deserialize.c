#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mach/mach_init.h>
#include <stack_logging.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define ENABLE_DEBUG_LOG 0

void print_hello_world()
{
    printf("Hello world!\n");
}

int main(int argc, char const *argv[])
{   
    if (argc < 3) {
        printf("arguments is not OK!");
        return 0;
    }

    const char *buffer_path = argv[1];
    FILE *buffer_file = fopen(buffer_path, "r");
    if (!buffer_file) {
        printf("open buffer file fail\n");
    }
    
    // get the file size
    fseek(buffer_file, 0L, SEEK_END);
    uint64_t buffer_size = ftell(buffer_file);
    fseek(buffer_file, 0L, SEEK_SET);	

    // read the entire buffer from file
    void *buffer = malloc(buffer_size);
    fread(buffer, 1, buffer_size, buffer_file);

    // serialize the buffer to backtrace_uniquing_table
    struct backtrace_uniquing_table *table = __mach_stack_logging_uniquing_table_copy_from_serialized(buffer, buffer_size);
    if (!table) {
        printf("serialize the table fail\n");
    }
    
    // read the stack
    uint64_t stack_identifier = atoi(argv[2]);
    uint32_t out_frames_count = STACK_LOGGING_MAX_STACK_SIZE;
    mach_vm_address_t *out_frames_buffer = (mach_vm_address_t *)malloc(sizeof(mach_vm_address_t) * out_frames_count);
    __mach_stack_logging_uniquing_table_read_stack(table,
                                                   stack_identifier,
                                                   out_frames_buffer,
                                                   &out_frames_count,
                                                   out_frames_count);
    if (out_frames_count) {
        #if ENABLE_DEBUG_LOG
        printf("%016llx\n", stack_identifier);
        #endif
        for (int i = 0; i < out_frames_count; i++) {
            mach_vm_address_t frame = out_frames_buffer[i];
            if (!frame) continue;
            #if ENABLE_DEBUG_LOG
            printf("-> %016llx\n", frame);
            #endif
        }
    }

    free(out_frames_buffer);

    return 0;
}

#pragma GCC diagnostic pop