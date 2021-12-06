//
//  system_info.c
//  yama
//
//  Created by 徐润康 on 2021/12/6.
//

#include "yama_system_info.h"
#include "yama.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <mach/machine.h>

const char *yama_get_device_arch(void)
{
    cpu_type_t type;
    size_t size = sizeof(type);
    sysctlbyname("hw.cputype", &type, &size, NULL, 0);
    
    cpu_subtype_t subtype;
    size = sizeof(subtype);
    sysctlbyname("hw.cpusubtype", &subtype, &size, NULL, 0);

    const char *arch = "";
    
    if (type == CPU_TYPE_ARM64) {
        if (subtype == CPU_SUBTYPE_ARM64_V8) {
            arch = "arm64";
        } else if (subtype == CPU_SUBTYPE_ARM64E) {
            arch = "arm64e";
        }
    } else if (type == CPU_TYPE_X86_64) {
        arch = "x86_64";
    } else if (type == CPU_TYPE_X86) {
        arch = "x86";
    }
    
#if YAMA_ENABLE_DEBUG_LOG
    printf("[YAMA_SYSTEM_INFO] type = %d, subtype = %d, and arch = %s\n", type, subtype, arch);
#endif
    
    return arch;
}
