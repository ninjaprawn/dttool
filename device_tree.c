#include <stdlib.h>
#include <string.h>
#include "device_tree.h"

DeviceTreeProperty* device_tree_first_property(DeviceTreeNode* child) {
    return child->properties;
}

DeviceTreeProperty* device_tree_next_property(DeviceTreeProperty* property) {
    return (DeviceTreeProperty*)((char*)property + sizeof(DeviceTreeProperty) + (((property->length & 0x7fffffff) + 3) & -4)); // Properties are 4 byte aligned
}

DeviceTreeNode* device_tree_first_child(DeviceTreeNode* node) {
    DeviceTreeProperty* property = device_tree_first_property(node);

    for (int i = 0; i < node->nProperties; i++) {
        property = device_tree_next_property(property);
    }

    return (DeviceTreeNode*)property;
}

DeviceTreeNode* device_tree_next_child(DeviceTreeNode* child) {
    DeviceTreeNode* _child = device_tree_first_child(child);

    for (int i = 0; i < child->nChildren; i++) {
        _child = device_tree_next_child(_child);
    }

    return _child;
}

DeviceTreeProperty* device_tree_get_property(DeviceTreeNode* node, char* property_name) {
    DeviceTreeProperty* property = node->properties;

    for (int i = 0; i < node->nProperties; i++) {
        char* name = property->name;
        if (!strcmp(name, property_name)) {
            return property;
        }

        property = device_tree_next_property(property);
    }

    return NULL;
}

DeviceTreeNode* device_tree_get_child(DeviceTreeNode* node, char* child_name) {
    DeviceTreeNode* child = device_tree_first_child(node);

    for (int i = 0; i < node->nChildren; i++) {
        char* name = device_tree_get_property(child, "name")->value;
        if (!strcmp(name, child_name)) {
            return child;
        }
        child = device_tree_next_child(child);
    }

    return NULL;
}

DeviceTreeNode* device_tree_lookup_entry(DeviceTreeNode* node, char* path) {
    if (path[0] == '/') {
        path++;
    }

    DeviceTreeNode* entry = node;
    char* temp_path = strdup(path);

    char* path_component = strtok(temp_path, "/");
    while (path_component) {
        entry = device_tree_get_child(entry, path_component);
        if (entry == NULL) {
            return NULL;
        }
        path_component = strtok(NULL, "/");
    }

    free(temp_path);

    return entry;
}
