#include "multiboot.h"

#define MULTIBOOT_TAG_TYPE_END           0
#define MULTIBOOT_TAG_TYPE_CMDLINE       1
#define MULTIBOOT_TAG_TYPE_MODULE        3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO 4
#define MULTIBOOT_TAG_TYPE_MMAP          6

#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4

struct multiboot_mmap_entry
{
    unsigned long long addr;
    unsigned long long len;
    unsigned int type;
    unsigned int zero;
}
__attribute__((packed));

typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_tag
{
    unsigned int type;
    unsigned int size;
};

struct multiboot_tag_string
{
    unsigned int type;
    unsigned int size;
    char string[0];
};

struct multiboot_tag_module
{
    unsigned int type;
    unsigned int size;
    unsigned int mod_start;
    unsigned int mod_end;
    char cmdline[0];
};

struct multiboot_tag_basic_meminfo
{
    unsigned int type;
    unsigned int size;
    unsigned int mem_lower;
    unsigned int mem_upper;
};

struct multiboot_tag_mmap
{
    unsigned int type;
    unsigned int size;
    unsigned int entry_size;
    unsigned int entry_version;
    struct multiboot_mmap_entry entries[0];
};

static void print_multiboot_tag(struct KernelMQ_Info *kinfo, const struct multiboot_tag *tag);

static void print_multiboot_tag_cmdline      (struct KernelMQ_Info *kinfo, const struct multiboot_tag_string        *tag);
static void print_multiboot_tag_module       (struct KernelMQ_Info *kinfo, const struct multiboot_tag_module        *tag);
static void print_multiboot_tag_basic_meminfo(struct KernelMQ_Info *kinfo, const struct multiboot_tag_basic_meminfo *tag);
static void print_multiboot_tag_mmap         (struct KernelMQ_Info *kinfo, const struct multiboot_tag_mmap          *tag);

unsigned char multiboot_parse(struct KernelMQ_Info *kinfo, unsigned long addr)
{
    if (!kinfo) {
        return 0;
    }

    // Unaligned address
    if (addr & 7) {
        return 0;
    }

    for (
        struct multiboot_tag *tag = (struct multiboot_tag*)(addr + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (struct multiboot_tag*)((unsigned char*)tag + ((tag->size + 7) & ~7))
    ) {
        print_multiboot_tag(kinfo, tag);
    }

    return 1;
}

void print_multiboot_tag(struct KernelMQ_Info *kinfo, const struct multiboot_tag *const tag)
{
    switch (tag->type)
    {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            print_multiboot_tag_cmdline(kinfo, (struct multiboot_tag_string*)tag);
            break;

        case MULTIBOOT_TAG_TYPE_MODULE:
            print_multiboot_tag_module(kinfo, (struct multiboot_tag_module*)tag);
            break;

        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            print_multiboot_tag_basic_meminfo(kinfo, (struct multiboot_tag_basic_meminfo*)tag);
            break;

        case MULTIBOOT_TAG_TYPE_MMAP:
            print_multiboot_tag_mmap(kinfo, (struct multiboot_tag_mmap*)tag);
            break;
    }
}

void print_multiboot_tag_cmdline(struct KernelMQ_Info *kinfo, const struct multiboot_tag_string *const tag)
{
    kprintf("Kernel command line: %s\n", tag->string);
}

void print_multiboot_tag_module(struct KernelMQ_Info *kinfo, const struct multiboot_tag_module *const tag)
{
    kprintf("Module at 0x%x-0x%x, command line: %s\n", tag->mod_start, tag->mod_end, tag->cmdline);
}

void print_multiboot_tag_basic_meminfo(struct KernelMQ_Info *kinfo, const struct multiboot_tag_basic_meminfo *const tag)
{
    kprintf("mem_lower = %uKB, mem_upper = %uKB\n", tag->mem_lower, tag->mem_upper);
}

void print_multiboot_tag_mmap(struct KernelMQ_Info *kinfo, const struct multiboot_tag_mmap *const tag)
{
    kprintf("Memory map:\n");

    for (
        const multiboot_memory_map_t *mmap = tag->entries;
        (unsigned char*)mmap < (unsigned char*)tag + tag->size;
        mmap = (multiboot_memory_map_t*)((unsigned long) mmap + tag->entry_size)
    ) {
        kprintf(
            " base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
            (unsigned)(mmap->addr >> 32),
            (unsigned)(mmap->addr & 0xffffffff),
            (unsigned)(mmap->len >> 32),
            (unsigned)(mmap->len & 0xffffffff),
            (unsigned)mmap->type
        );
    }
}
