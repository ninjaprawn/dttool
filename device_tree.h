// Based from xnu-4570.41.2/pexpert/pexpert/device_tree.h

#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

#include <stdint.h>

typedef struct DeviceTreeProperty {
    char        name[32];	// NULL terminated property name
    uint32_t    length;                 // Length of the value
    char        value[0];
} DeviceTreeProperty;

typedef struct DeviceTreeNode {
    uint32_t                nProperties;    // Number of props[] elements (0 => end)
    uint32_t                nChildren;      // Number of children[] elements
    DeviceTreeProperty      properties[0];  // array size == nProperties
//  DeviceTreeNode          children[];     // array size == nChildren
} DeviceTreeNode;



DeviceTreeProperty* device_tree_first_property(DeviceTreeNode* child);
DeviceTreeProperty* device_tree_next_property(DeviceTreeProperty* property);
DeviceTreeProperty* device_tree_get_property(DeviceTreeNode* node, char* property_name);
DeviceTreeNode* device_tree_first_child(DeviceTreeNode* node);
DeviceTreeNode* device_tree_next_child(DeviceTreeNode* child);
DeviceTreeNode* device_tree_get_child(DeviceTreeNode* node, char* child_name);
DeviceTreeNode* device_tree_lookup_entry(DeviceTreeNode* node, char* path);

#endif
