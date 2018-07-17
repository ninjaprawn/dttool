#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>

#include "device_tree.h"
typedef char bool;

void print_indents(int indent_level) {
    char* indent_buffer = calloc(4*indent_level + 5, sizeof(char));
    for (int i = 0; i < indent_level + 1; i++) {
        strcat(indent_buffer, "\t");
    }

    printf("%s", indent_buffer);

    free(indent_buffer);
}

bool is_string(char* mem) {
    int string_length = strlen(mem);

    if (mem[0] == 0) {
        return 0;
    }

    for (int i = 0; i < string_length; i++) {
        if (!isprint(mem[i]) || mem[i] == 0) {
            return 0;
        }
    }

    return 1;
}

void print_hex_view(uint32_t* mem, uint32_t size) {
    static uint32_t remainderMap[] = { 0xFF, 0xFFFF, 0xFFFFFF };

    int idx = 0;
    while (size > 3) {
        if (idx == 0) {
            printf("%08x", mem[idx]);
        } else {
            printf(" %08x", mem[idx]);
        }
        idx++;
        size -= 4;
    }

    if (size != 0) {
        printf(" %08x", mem[idx] & remainderMap[size - 1]);
    }
}

// Returns how much we have travelled
int device_tree_view(DeviceTreeNode* tree_node, bool force_hex, int tree_level) {
    if (tree_level == 0) {
        printf("root:\n");
    }

    DeviceTreeNodeProperty* property = (DeviceTreeNodeProperty*)((char*)tree_node + sizeof(tree_node));

    for (int i = 0; i < tree_node->nProperties; i++) {
        print_indents(tree_level);

        char* name = property->name;
        uint32_t size = property->length;
        if (size & 0x80000000) {
            printf("Found a bad size!\n");
            size = size & 0x7fffffff;
        }
        char* value = (void*)property + sizeof(*property);

        printf("(%d bytes) %s = ", size, name);
        if (force_hex || !is_string(value)) {
            print_hex_view((uint32_t*)value, size);
            printf("\n");
        } else {
            printf("%s\n", value);
        }

        property = (DeviceTreeNodeProperty*)((char*)property + ((size + 3) & -4) + sizeof(*property));
    }

    char* child = (char*)property;
    for (int i = 0; i < tree_node->nChildren; i++) {
        print_indents(tree_level);
        printf("child[%d]:\n", i);

        child += device_tree_view((DeviceTreeNode*)child, force_hex, tree_level + 1);
    }

    return child - (char*)tree_node;
}

int device_tree_fix_sizes(DeviceTreeNode* tree_node, uint32_t* props_fixed) {
    DeviceTreeNodeProperty* property = (DeviceTreeNodeProperty*)((char*)tree_node + sizeof(tree_node));

    for (int i = 0; i < tree_node->nProperties; i++) {
        if (property->length & 0x80000000) {
            property->length = property->length & 0x7fffffff;
            *props_fixed += 1;
        }
        property = (DeviceTreeNodeProperty*)((char*)property + ((property->length + 3) & -4) + sizeof(*property));
    }

    char* child = (char*)property;
    for (int i = 0; i < tree_node->nChildren; i++) {
        child += device_tree_fix_sizes((DeviceTreeNode*)child, props_fixed);
    }

    return child - (char*)tree_node;
}

bool file_exist(char *filename, int* file_size) {
    struct stat buffer;
    int ret = stat(filename, &buffer);

    if (ret != -1) {
        *file_size = buffer.st_size;
    }

    return ret != -1;
}


void print_usage() {
    printf("dttool - a tool for viewing and manipulating iOS DeviceTree files (https://github.com/ninjaprawn/dttool)\n");
    printf("Created by @theninjaprawn. Based on xnu-4570.41.2/pexpert/gen/device_tree.c\n");
    printf("\n");
    printf("Usage: dtool [operation] [modifiers] <file_name>\n");
    printf("\n");
    printf("Operations (modifiers are prefixed by --):\n");
    printf("\t-view\tOutputs the DeviceTree file in a readable format\n");
    printf("\t--hex\tForce outputs all values as hex values\n");
    printf("\n");
    printf("\t-fix-sizes\tFixes the property size fields that have their upper bit set\n");
    printf("\n");
    printf("Thanks to Jonathan Levin whose code was used briefly as a reference (http://www.newosxbook.com/src.jl?tree=listings&file=6-bonus.c)\n");
}

int main(int argc, char* argv[]) {
    int file_size = -1;

    if (argc == 1) {
        print_usage();
        return 0;
    }

    char* file_name = argv[argc - 1];
    if (!file_exist(file_name, &file_size)) {
        printf("File \"%s\" does not exist\n", file_name);
        return 1;
    }

    bool is_viewing = 0;
    bool view_hex = 0;

    bool is_fixing_sizes = 0;

    if (argc == 2) {
        printf("[warning] No options provided. Using -view\n");
        is_viewing = 1;
    } else {
        if (strncmp(argv[1], "--", 2) == 0 || strncmp(argv[1], "-", 1) != 0) {
            printf("First argument is not an operation! Run \"%s\" to view possible operations\n", argv[0]);
            return 1;
        }

        if (!strcmp(argv[1], "-view")) {
            is_viewing = 1;
            for (int i = 2; i < argc - 1; i++) {
                if (!strcmp(argv[i], "--hex")) {
                    view_hex = 1;
                } else {
                    printf("Unknown modifier \"%s\" for viewing. Run \"%s\" to view possible modifiers\n", argv[i], argv[0]);
                    return 1;
                }
            }
        } else if (!strcmp(argv[1], "-fix-sizes")) {
            is_fixing_sizes = 1;
        } else {
            printf("Unknown operation \"%s\". Run \"%s\" to view possible operations\n", argv[1], argv[0]);
            return 1;
        }
    }

    if (file_size == -1 || file_size == 0) {
        printf("File \"%s\" has an invalid file size (0x%08x)\n", file_name, file_size);
        return 3;
    }

    int fd = open(file_name, O_RDWR);
    if (fd == -1) {
        printf("Failed to open \"%s\" for read and write\n", file_name);
        return 4;
    }

    void* mapped_file = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    if (mapped_file == NULL) {
        printf("Failed to map the file into memory\n");
        return 5;
    }

    DeviceTreeNode* root_node = (DeviceTreeNode*)mapped_file;
    printf("Using a DeviceTree with %d properties and %d children\n", root_node->nProperties, root_node->nChildren);

    if (is_viewing) {
        device_tree_view(root_node, view_hex, 0);
    } else if (is_fixing_sizes) {
        uint32_t props_fixed = 0;
        device_tree_fix_sizes(root_node, &props_fixed);
        printf("Fixed %d length field%s\n", props_fixed, props_fixed == 1 ? "" : "s");
    }

    munmap(mapped_file, file_size);
    return 0;
}
