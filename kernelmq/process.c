#include "process.h"

#include "stdlib.h"

enum KernelMQ_Process_Error KernelMQ_Process_List_init(
    KernelMQ_Process_List *const process_list,
    const struct KernelMQ_Info *const kinfo
) {
    kmemset(process_list, 0, sizeof(*process_list));

    const enum KernelMQ_Process_Error create_kernel_process_result =
        KernelMQ_Process_create_from_kernel(&(*process_list)[0], kinfo);

    if (create_kernel_process_result != KERNELMQ_PROCESS_ERROR_OK) {
        kmemset(process_list, 0, sizeof(*process_list));
        return create_kernel_process_result;
    }

    const unsigned int modules_length = kinfo->modules_count;

    if (modules_length > KERNELMQ_PROCESS_LIST_LENGTH - 1) {
        kmemset(process_list, 0, sizeof(*process_list));
        return KERNELMQ_PROCESS_ERROR_MODULES_TOO_MANY;
    }

    for (unsigned int mod_index = 0; mod_index < modules_length; ++mod_index) {
        const enum KernelMQ_Process_Error create_mod_process_result =
            KernelMQ_Process_create_from_module(
                &(*process_list)[mod_index + 1],
                &kinfo->modules[mod_index]
            );

        if (create_mod_process_result != KERNELMQ_PROCESS_ERROR_OK) {
            kmemset(process_list, 0, sizeof(*process_list));
            return create_mod_process_result;
        }
    }

    return KERNELMQ_PROCESS_ERROR_OK;
}

enum KernelMQ_Process_Error KernelMQ_Process_create_from_kernel(
    struct KernelMQ_Process *const process,
    const struct KernelMQ_Info *const kinfo
) {
    process->is_present = 1;
    process->created_from = KERNELMQ_PROCESS_CREATED_FROM_KERNEL;

    const unsigned int cmdline_slen = kstrlen(kinfo->cmdline);

    if (cmdline_slen > KERNELMQ_PROCESS_CMDLINE_SLEN_MAX) {
        return KERNELMQ_PROCESS_ERROR_CMDLINE_TOO_LONG;
    }

    kstrncpy(process->cmdline, kinfo->cmdline, cmdline_slen);

    if (kinfo->areas_count > KERNELMQ_PROCESS_AREAS_LENGTH_MAX) {
        return KERNELMQ_PROCESS_ERROR_KERNEL_AREAS_LENGTH_TOO_LONG;
    }

    const unsigned int areas_length = kinfo->areas_count;

    process->areas_length = areas_length;

    for (unsigned int area_index = 0; area_index < areas_length; ++area_index) {
        const unsigned long long base  = kinfo->areas[area_index].base;
        const unsigned long long size  = kinfo->areas[area_index].size;
        const unsigned long long limit = kinfo->areas[area_index].limit;

        if (base > 0xFFFFFFFF || size > 0xFFFFFFFF || limit > 0xFFFFFFFF) {
            return KERNELMQ_PROCESS_ERROR_ADDR_TOO_BIG;
        }

        if (base + size - 1 != limit) {
            return KERNELMQ_PROCESS_ERROR_INVALID_AREA;
        }

        process->areas[area_index].base  = base;
        process->areas[area_index].size  = size;
        process->areas[area_index].limit = limit;
    }

    return KERNELMQ_PROCESS_ERROR_OK;
}

enum KernelMQ_Process_Error KernelMQ_Process_create_from_module(
    struct KernelMQ_Process *const process,
    const struct KernelMQ_Info_Module *const kinfo_module
) {
    process->is_present = 1;
    process->created_from = KERNELMQ_PROCESS_CREATED_FROM_MODULE;

    const unsigned int cmdline_slen = kstrlen(kinfo_module->cmdline);

    if (cmdline_slen > KERNELMQ_PROCESS_CMDLINE_SLEN_MAX) {
        return KERNELMQ_PROCESS_ERROR_CMDLINE_TOO_LONG;
    }

    kstrncpy(
        process->cmdline,
        kinfo_module->cmdline,
        cmdline_slen
    );

    process->areas_length = 1;

    const unsigned long long base  = kinfo_module->base;
    const unsigned long long size  = kinfo_module->size;
    const unsigned long long limit = kinfo_module->limit;

    if (base > 0xFFFFFFFF || size > 0xFFFFFFFF || limit > 0xFFFFFFFF) {
        return KERNELMQ_PROCESS_ERROR_ADDR_TOO_BIG;
    }

    if (base + size - 1 != limit) {
        return KERNELMQ_PROCESS_ERROR_INVALID_AREA;
    }

    process->areas[0].base  = base;
    process->areas[0].size  = size;
    process->areas[0].limit = limit;

    return KERNELMQ_PROCESS_ERROR_OK;
}
