//
//  yama.h
//  yama
//
//  Created by xurunkang on 2021/11/30.
//

#ifndef yama_h
#define yama_h

#include "_types.h"
#include "stdbool.h"

typedef struct yama_logging_context {
    uint64_t minimum_size;
    bool only_print_alive_only;
} yama_logging_context_t;

int yama_initialize(void);
void yama_prepare_logging(yama_logging_context_t *context);
int yama_start_logging(void);
int yama_stop_logging(void);

#endif /* yama_h */
