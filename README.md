*This project has been created as part of the 42 curriculum by grishmel.*

# get_next_line

## Description

**`get_next_line`** is a foundational C programming project at 42 School whose goal is to implement a function (`char *get_next_line(int fd);`) that reads and returns a single line from a file descriptor (`fd`) on each invocation. 

Repeated calls to `get_next_line()` in a loop allow a program to read a text file (or standard input) sequentially, one line at a time, until the end of the file (EOF) is reached. Each returned string includes its terminating newline character (`\n`), except when EOF is reached and the file does not end with a trailing `\n`. If there is nothing left to read or if an error occurs, the function returns `NULL`.

### Core Learning Objectives
- **Static Variables:** Understanding persistent state across multiple function calls and how static memory lives in the Data/BSS segments rather than on the stack.
- **Dynamic Heap Memory Management:** Mastering `malloc()` and `free()` to construct dynamically sized strings without a single memory leak, dangling pointer, or double free.
- **File Descriptor Manipulation:** Interacting directly with the kernel via the `read()` system call using configurable buffer sizes (`BUFFER_SIZE`).
- **Strict 42 Norm Compliance:** Writing modular, defensive C code under strict constraints (maximum 25 lines per function, no `libft` allowed, no global variables, and `lseek()` strictly forbidden).

### Repository File Structure

| Category | File Name | Responsibility |
| :--- | :--- | :--- |
| **Mandatory** | `get_next_line.h` | Function prototypes, standard header includes (`unistd.h`, `stdlib.h`), and fallback `BUFFER_SIZE` macro definition. |
| **Mandatory** | `get_next_line.c` | Core pipeline implementation (`get_next_line`, `read_and_stash`, `extract_line`, `update_stash`). |
| **Mandatory** | `get_next_line_utils.c` | Custom string manipulation helpers (`ft_strlen`, `ft_strchr`, `ft_strjoin`, etc.). |
| **Bonus** | `get_next_line_bonus.h` | Bonus header defining multi-FD limits and prototypes. |
| **Bonus** | `get_next_line_bonus.c` | Multi-FD implementation managing simultaneous file descriptors with a single static variable. |
| **Bonus** | `get_next_line_utils_bonus.c` | Helper functions supporting the bonus implementation. |

---

## Algorithm & Technical Justification

### Core Pipeline (`read_and_stash`, `extract_line`, `update_stash`)

Because the number of bytes read per `read(fd, buf, BUFFER_SIZE)` call rarely aligns with the exact length of a line, our implementation relies on a persistent `stash` variable (`static char *stash`) that stores leftover characters between calls. Each call to `get_next_line(fd)` executes a three-stage pipeline:

1. **`read_and_stash(int fd, char *stash)`**
   - Allocates a temporary heap buffer `buf` of size `BUFFER_SIZE + 1` bytes.
   - Enters a `while` loop that invokes `read(fd, buf, BUFFER_SIZE)` **only** as long as no newline character (`\n`) is present in `stash` (`!ft_strchr(stash, '\n')`) and `read_bytes != 0`.
   - Null-terminates `buf` at `buf[read_bytes] = '\0'` after each read and appends it to `stash` using a custom `ft_strjoin`. To prevent memory leaks during iterative accumulation, `ft_strjoin` automatically calls `free()` on the previous `stash` allocation before returning the newly concatenated string.
   - If `read()` returns `-1` (read error), both `buf` and `stash` are immediately freed and `NULL` is returned.

2. **`extract_line(char *stash)`**
   - Inspects the populated `stash`. If `stash` is empty (`!stash[0]`), returns `NULL`.
   - Scans `stash` up to the first `\n` or `\0` to determine the exact byte length of the current line.
   - Allocates the precise byte count required (including `+ 1` for `\n` if present, and `+ 1` for the null terminator `\0`) and copies the characters into the newly allocated `line` string returned to the caller.

3. **`update_stash(char *stash)`**
   - Advances through `stash` to locate the character immediately following the first `\n`.
   - If no `\n` was found (meaning EOF was reached) or no characters remain after `\n` (`stash[i] == '\0'`), it frees `stash` and returns `NULL` so the static pointer is cleanly reset.
   - Otherwise, it allocates a new string strictly sized for the remaining suffix after `\n`, copies those leftover bytes into the new buffer, calls `free(stash)` on the old block, and returns the updated pointer to be stored in `stash` for the next call to `get_next_line()`.

### Technical Justification

- **Lazy Reading with a Dynamic String (`char *`) vs. Whole-File Reading or Linked Lists:**
  - *Why not read the whole file?* The 42 subject explicitly requires reading as little as possible on each call. Reading an entire gigabyte file or reading indefinitely from `stdin` before returning the first line violates the specification and wastes memory. By checking `!ft_strchr(stash, '\n')` before calling `read()`, our algorithm stops reading as soon as a complete line is available in `stash`. If `stash` already contains multiple `\n` characters from a previous large read, `read()` is not called at all until those buffered lines are consumed.
  - *Why a flat `char *` buffer over a linked list?* While a linked list of buffers avoids reallocating and copying strings on each chunk, it introduces significant pointer overhead, multiple small heap allocations per node, and complex cleanup logic that is difficult to fit cleanly within the 42 Norm's 25-line function limit. A dynamic `char *` buffer paired with an auto-freeing `ft_strjoin` keeps the code compact, readable, and resilient across extreme `BUFFER_SIZE` values (from `BUFFER_SIZE=1` to `BUFFER_SIZE=10000000`).

### Bonus Architecture (Multi-FD Support)

The bonus section requires managing multiple file descriptors simultaneously (e.g., interleaved reads across `fd 3`, `fd 4`, and `fd 5`) while using **only one static variable**.

- **Implementation:** We upgrade the single static pointer `static char *stash;` into a fixed-size static array of pointers:
  ```c
  static char *stash[4096];
  ```
- **Why Direct `fd` Indexing Works:** In UNIX/POSIX systems, file descriptors are small non-negative integers serving as indices into the process's file descriptor table (where standard `ulimit -n` limits typically default to `1024` or `4096`). By indexing our static array directly with `stash[fd]` (after validating `fd >= 0 && fd < 4096`), each open file descriptor maintains its own independent heap pointer inside a single static array declaration.
- **Complexity Advantage:** Direct array indexing `stash[fd]` guarantees $O(1)$ time complexity for looking up and updating the reading state of any file descriptor, avoiding the $O(N)$ traversal overhead and node-management complexity of a multi-FD linked list.

---

## Instructions

### Compilation

The project is designed to be compiled directly with your source files at the root of the repository using `cc` and the standard 42 flags (`-Wall -Wextra -Werror`).

#### 1. Mandatory Part
Compile with a custom `BUFFER_SIZE` (e.g., `42`):
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl
```

Compile without specifying `-D BUFFER_SIZE` (uses the fallback value defined in `get_next_line.h`):
```bash
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c -o gnl
```

#### 2. Bonus Part (Multi-FD)
Compile the `*_bonus.c` files to enable simultaneous file descriptor management:
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c -o gnl_bonus
```

### Usage Example (`main.c`)

Below is a complete, copy-pasteable `main.c` demonstrating both single-file reading and interleaved multi-FD reading, with proper memory deallocation (`free`) and file descriptor cleanup (`close`):

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "get_next_line_bonus.h" /* Or "get_next_line.h" for mandatory */

int	main(void)
{
	int		fd1;
	int		fd2;
	char	*line1;
	char	*line2;

	/* --- 1. Single File Reading Example --- */
	printf("=== Single FD Reading (test1.txt) ===\n");
	fd1 = open("test1.txt", O_RDONLY);
	if (fd1 < 0)
		return (perror("open test1.txt"), 1);
	while ((line1 = get_next_line(fd1)) != NULL)
	{
		printf("%s", line1);
		free(line1);
	}
	close(fd1);

	/* --- 2. Interleaved Multi-FD Reading Example (Bonus) --- */
	printf("\n\n=== Interleaved Multi-FD Reading (test1.txt & test2.txt) ===\n");
	fd1 = open("test1.txt", O_RDONLY);
	fd2 = open("test2.txt", O_RDONLY);
	if (fd1 < 0 || fd2 < 0)
		return (perror("open"), 1);
	while (1)
	{
		line1 = get_next_line(fd1);
		line2 = get_next_line(fd2);
		if (!line1 && !line2)
			break ;
		if (line1)
		{
			printf("[FD %d]: %s", fd1, line1);
			free(line1);
		}
		if (line2)
		{
			printf("[FD %d]: %s", fd2, line2);
			free(line2);
		}
	}
	close(fd1);
	close(fd2);
	return (0);
}
```

### Memory Leak Verification (Valgrind)

To verify that all heap-allocated memory is properly freed across normal execution and EOF conditions, run the compiled binary under Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./gnl_bonus
```

---

## Resources

### Technical References
- **Linux Programmer's Manual (`man` pages):**
  - `man 2 read` — System call specification, return values (`ssize_t`), EOF (`0`), and error states (`-1`).
  - `man 3 malloc` & `man 3 free` — Dynamic memory allocation and heap deallocation rules.
  - `man 2 open` & `man 2 close` — File descriptor lifecycle management.
- **C Memory Layout & Static Storage Duration:**
  - Documentation on C storage classes (`static` vs. `auto`) and process memory segments (Text, Data, BSS, Heap, Stack).
  - static vars: https://www.geeksforgeeks.org/c/g-fact-80/
  - read(2) linux manual: https://man7.org/linux/man-pages/man2/read.2.html
- **Community Testing Suites:**
  - [gnlTester by Tripouille](https://github.com/Tripouille/gnlTester) — Automated unit test suite covering edge cases, invalid file descriptors, and varying `BUFFER_SIZE` configurations.

### AI Usage Disclosure
In strict accordance with the 42 curriculum guidelines on Artificial Intelligence, AI was used solely as a **Socratic peer-learning and debugging assistant** rather than a code generator:
- **Edge-Case Analysis & Debugging:** Used to reason through subtle C memory pitfalls, including `size_t` unsigned integer underflow during length calculations, why `free()` must always receive the exact base pointer returned by `malloc()` rather than an incremented offset pointer, and properly handling files that reach EOF without a trailing `\n`.
- **42 Norm Refactoring:** Consulted for strategies on modularizing helper functions and simplifying control flow to stay strictly within the 25-line-per-function and 5-function-per-file 42 Norm limits.
- **Documentation Structuring:** Assisted in organizing and formatting this `README.md` to ensure clarity and full compliance with the 42 subject requirements.
- **Manual Implementation:** All source code, pointer arithmetic, and memory management logic were written, traced step-by-step, and tested manually.