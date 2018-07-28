# dttool
A tool for viewing and manipulating DeviceTree files.

To compile and run, run `make`, then move or execute `./dttool`.

The view implementation is not perfect, as some values may appear as strings, whereas other strings are cut off due to NULL bytes.
Use `--hex` to view the hex representation.

Feel free to submit issues or pull requests.

Tested on an extracted `DeviceTree.n56ap.im4p` from iOS 12 beta 1.

```
dttool - a tool for viewing and manipulating iOS DeviceTree files (https://github.com/ninjaprawn/dttool)
Created by @theninjaprawn. Based on xnu-4570.41.2/pexpert/gen/device_tree.c

Usage: dttool <operation> [modifiers] <file_name>

Operations (modifiers are prefixed by --):
	-view	Present the DeviceTree file in a readable format
	--hex	Force outputs all values as hex values
	--list-children	Only view a list of children of a node
	--no-children	Only view the properties of a node
	--path=<path>	Only view a specific node of the device tree

	-fix-sizes	Fixes the property size fields that have their upper bit set

	-qemu	Patches the device tree for QEMU emulation

Thanks to Jonathan Levin whose code was used briefly as a reference (http://www.newosxbook.com/src.jl?tree=listings&file=6-bonus.c)
```
