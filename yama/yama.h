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

#define YAMA_SUCCESS                    0
#define YAMA_LOGGING_CONTEXT_IS_NULL    10001
#define YAMA_INIT_TABLE_ERROR           10002

typedef struct yama_logging_context {
    char *output_dir;
} yama_logging_context_t;

extern int yama_initialize(void);
extern int yama_prepare_logging(yama_logging_context_t *context);
extern int yama_start_logging(void);
extern void yama_stop_logging(void);

#ifdef __cplusplus
}
#endif

#endif /* yama_h */
