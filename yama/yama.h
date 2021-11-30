//
//  yama.h
//  yama
//
//  Created by xurunkang on 2021/11/30.
//

#ifndef yama_h
#define yama_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct yama_logging_context {
    uint64_t minimum_size;
    bool only_print_alive;
    bool print_stack_frames;
} yama_logging_context_t;

extern int yama_initialize(void);
extern void yama_prepare_logging(yama_logging_context_t *context);
extern int yama_start_logging(void);
extern int yama_stop_logging(void);

#ifdef __cplusplus
}
#endif

#endif /* yama_h */
