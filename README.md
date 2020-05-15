# Project 3: Memory Allocator

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-3.html 

```bash

Name: Porfirio Mohabir
University of San Francisco
Operating Systems CS 326
```

#ABOUT

```bash

Program Develops a custom memory allocator: 
	- Program uses system call mmap and allocate entire regions of memory at a time
	- Size of each region should be multiple of the system page size
	- Implements "First","Best", and "Worst" fit alogorthims. 
		- These alg. provides the ability to configure the active free
		  space
	- Manages Memory using Linked List Data Structure
	- Free Memory based on where the block is in Linked List
	- Reallocating Memory 
	- Ability to print Memory State Information
	- Named Blocks
	- File Logging
	- Visulaization 
		- Output from print_memory() can be passes into visualize.sh to dipslay the
		  memory regions
	- Scribbling 
```

To compile and use the allocator:

```bash
make
LD_PRELOAD=$(pwd)/allocator.so ls /
```

(in this example, the command `ls /` is run with the custom memory allocator instead of the default).

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
