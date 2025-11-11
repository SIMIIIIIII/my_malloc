# My Malloc

In this project, I create my own `malloc` and `free` functions. I use a list to simulate the heap. The `init()` function initializes the heap and prepares the space for any future allocation, so it must be called before using `malloc` or `free`.

## 1. Metadata (MD)

I allocated six bytes of MD for each block of free or allocated memory: four at the beginning and two at the end. At the beginning of the heap, two bytes of MD are allocated to store the index of the first free block in the heap,to start the linked list of empty blocks.

The MDs on the left contain two pieces of information:

- size: number of available bytes
- next:
  - index of the next free block if the space is free
  - 0 if the space is occupied
  - 65.535 (uint16_t max) if the block is the last free block

On the right, there is only the size information.

### Initial state (after initialization)

```text
+----------------------+------------------------------+----------------+
| BEGIN BLOCK          | 63,992 free bytes            | END BLOCK      |
| size : 63,992        | index : 2 + 4(MD)            | size : 63,992  |
| next : 65,535        |                              |                |
+----------------------+------------------------------+----------------+
| FIRST FREE           |
| index : 2            |
+----------------------+
```

## 2. Memory Allocation

When a suitable free block is found to accommodate user data, MD blocks are placed at the beginning and end of the newly occupied space, as well as in the remaining free space. If the size of the remaining block is less than 7 bytes, then the found block is fully allocated to avoid an unusable fragment. The `next` of the allocated block is set to 0, the `next` of the preceding and following empty blocks are updated, and the index of the first free block at the beginning of the heap is updated if necessary.

### Example (after three allocations)

```text
+-------------------+-------------------------------------+-------------------+
| BEGIN BLOCK       | 63,943 free bytes                   | END BLOCK         |
| size : 63.943     | index : 51 + 4(MD)                  | size : 63.943     |
| next : 65.535     |                                     |                   |
+-------------------+-------------------------------------+-------------------+
| BEGIN BLOCK       | 13 user data bytes                  | END BLOCK         |
| size : 13         | returned address : &MY_HEAP[36]     | size : 13         |
| next : 0          |                                     |                   |
+-------------------+-------------------------------------+-------------------+
| BEGIN BLOCK       | 17 user data bytes                  | END BLOCK         |
| size : 17         | returned address : &MY_HEAP[13]     | size : 17         |
| next : 0          |                                     |                   |
+-------------------+-------------------------------------+-------------------+
| BEGIN BLOCK       | 1 user data bytes                   | END BLOCK         |
| size : 1          | returned address : &MY_HEAP[6]      | size : 1          |
| next : 0          |                                     |                   |
+-------------------+-------------------------------------+-------------------+
| FIRST FREE        |
| index : 51        |
+-------------------+
```

## 3. Free Memory

Memory free consists of two operations:

1. Check left and right for free space and merge it immediately (the "eager" strategy).

2. Update the `next` statements of the surrounding empty space to insert the newly freed block into the linked list of empty blocks.

## 4. Placement Strategy

Objective: A compromise between memory optimization and speed. The search starts at the beginning of the array and jumps from empty block to empty block via the linked list.

Strategies used:

```text
+----------------------+---------------------------------------------------------------+
| Strategy             | Condition / Description                                       |
+----------------------+---------------------------------------------------------------+
| First perfect fit    | Exact size requested (size + 6 bytes MD)                      |
| Almost perfect fit   | Size within a margin of ~10%                                  |
| Insignificant fit    | This would leave a remainder >= 10 times the requested size   |
| Best fit             | If none of the above applies                                  |
+----------------------+---------------------------------------------------------------+
```

Process:
   - Sequential traversal of free blocks.
   - Immediate allocation if a "perfect," "almost," or "insignificant" fit is found.
   - Otherwise, the best free bloc address (best fit) is retained until the end.

### Measured results

```text
+-----------------------+-------+
| Indicator             | Score |
+-----------------------+-------+
| Memory optimization   | 77%   |
| Time optimization     | 75%   |
+-----------------------+-------+
```

## How to Run Tests

### Prerequisites

Packages to install:

   - gcc and make (Lunix)
   - CUnit library (headers and runtime)
   - lcov: optional for coverage

Fedora:

```bash
sudo dnf install -y gcc make CUnit CUnit-devel lcov
```

Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libcunit1 libcunit1-dev lcov
```

### Build and Run Tests

From the project root (`my_malloc`):

```bash
make test
```

This compiles `malloc.c` and `test.c`, links CUnit, and runs the tests.

### Run the performance/summary generator

```bash
make resume
```

Run `test_resume.c` which prints the allocation/free statistics.

### Clean the project directory

```bash
make clean
```

Deletes the binary files (`test`, `resume`, `malloc.o`) and the coverage files.

### Run with coverage locally (optional)

```bash
gcc -Wall -Werror --coverage -o malloc.o -c malloc.c
gcc -Wall -Werror --coverage -o test test.c malloc.o -lcunit -lm
./test
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors unused
lcov --list coverage.info
```