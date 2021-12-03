#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct backtrace_uniquing_table;
struct backtrace_uniquing_table *msl_uniquing_table_copy_from_serialized(void *buffer, size_t size);
int msl_uniquing_table_read_stack(struct backtrace_uniquing_table *uniquing_table,
							      uint64_t stackid,
                                  uint64_t *out_frames_buffer,
                                  uint32_t *out_frames_count,
                                  uint32_t max_frames);

int main(int argc, char const *argv[])
{
    const char *buffer_path = argv[1];
    printf("buffer path = %s\n", buffer_path);

    FILE *buffer_file = fopen(buffer_path, "r");
    if (!buffer_file) {
        printf("open buffer file fail\n");
    }

    uint64_t size = 4210688;
    void *buffer = malloc(size);
    fread(buffer, 1, size, buffer_file);

    struct backtrace_uniquing_table *table = msl_uniquing_table_copy_from_serialized(buffer, size);
    

    return 0;
}
