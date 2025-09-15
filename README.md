# COMMON-C *(commc)*

## Overview

*COMMON-C* is a collection of Post-FOSS C libraries providing foundational low-level tools for software and game development. The project maintains strict **C89** compliance for maximum compatibility.

*COMMON-C* is a collection of open source functions - both ported from existing projects and newly developed - reorganized into a unified framework. While the code is aggregated in one accessible location for convenience, the project maintains no central authority over its use, modification, or redistribution."

## Project Scope

COMMON-C encompasses (at present time):

### Core Systems



- **Error handling** and **memory management**
  - **Error handling**: detect and represent abnormal conditions using return codes, error values, or status flags; propagate errors to the caller; provide *deterministic cleanup* and clear error reporting.
  - **Memory management**: allocate and free heap memory (*e.g.*, `malloc`/`free`), track ownership and lifetimes, avoid leaks and double-free, and manage fragmentation. Also covers stack vs heap allocation and strategies like *pooling* or *reference counting*.

  - **String manipulation** and **time utilities**
    - **String manipulation**: functions operating on sequences of bytes terminated by a NUL character (`char *` in C). Includes length, copy, concatenation, compare, search, split/tokenize, and formatted conversion. Key concerns: buffer sizing, encoding assumptions, and correct NUL termination.
    - **Time utilities**: represent time as numeric timestamps (integer or floating point), obtain monotonic and wall-clock time, compute durations (timestamp differences), and format/parse human-readable date/time strings.

  - **Platform abstraction layers**
    - Provide a stable API layer that maps higher-level calls (file I/O, threading, timers, etc.) to platform-specific implementations. Abstract differences in system calls, error codes, and path/permission semantics so the same higher-level code compiles on multiple OSes.

### Data Structures



- **Dynamic arrays**, **linked lists**, **queues**, **stacks**
  - **Dynamic array**: contiguous memory for elements with **O(1)** indexed access; resizing copies elements to a larger block when capacity is exceeded (amortized **O(1)** append).
  - **Linked list**: chain of nodes with pointers to next (and optionally previous) nodes; **O(1)** insertion/removal given a node, **O(n)** random access.
  - **Queue**: **FIFO** structure supporting enqueue and dequeue operations; common **O(1)** implementations use circular buffers or linked lists.
  - **Stack**: **LIFO** structure supporting push/pop operations; **O(1)** operations using arrays or linked lists.

  - **Hash tables** and **tree implementations**
  - **Hash table**: map keys to values using a hash function `h(key)` producing bucket indices; collision resolution via chaining or open addressing. Average-case **O(1)** for insert/lookup/delete; performance depends on load factor and hash quality.
  - **Tree implementations**: hierarchical node structures. **Binary search trees** provide ordered storage with **O(h)** lookup where `h` is tree height; balanced trees (**AVL**, **Red-Black**) maintain `h = O(log n)` for guaranteed logarithmic operations. **B-trees** and variants are used for disk-optimized storage.

- **Graph structures** and **specialized containers**
  - **Graph**: set of vertices and edges (directed or undirected). Representations: adjacency list (per-vertex neighbor lists) or adjacency matrix (2D matrix of weights/flags). Core algorithms: **BFS**/**DFS** traversal, shortest path (**Dijkstra**), connectivity and cycle detection. Complexity depends on representation and algorithm.
  - **Specialized containers**: data structures optimized for specific constraints (*e.g.*, priority queue with **O(log n)** extract-min, bloom filter for probabilistic membership, sparse/dense sets for different memory/access tradeoffs).

### Mathematics

- **Vector** and **matrix operations**
  - **Vector**: ordered tuple of numeric components, *e.g.* `v = [x, y, z]`. Operations: addition `v + w` (component-wise), scalar multiplication `a * v`, dot product `dot(v, w) = sum(v_i * w_i)` producing a scalar, and cross product (3D) producing a vector orthogonal to inputs.
    - **Matrix**: 2D array of numbers. Matrix multiplication `A * B` composes linear transformations. Common operations: multiply, transpose, inverse (if invertible), and application to vectors for linear transforms (rotation, scaling). Use homogeneous coordinates for translations in affine transforms.
- **Trigonometric functions**
    - **Sine**, **cosine**, **tangent**: functions of an angle (radians) mapping to ratios or unit-circle coordinates. Properties: periodicity, derivatives, and algebraic identities (*e.g.*, `sin^2 x + cos^2 x = 1`). Used for rotations and oscillatory computations.

- **Random number generation**
    - **Pseudo-random number generators (PRNGs)**: deterministic stateful algorithms producing sequences approximating statistical randomness. Use a seed for reproducibility. Provide uniform and non-uniform sampling (*e.g.*, Gaussian), shuffling, and sampling with/without replacement.

### File & Network I/O

- Binary file operations
- Directory management
- Cross-platform TCP/UDP socket abstractions

  - Binary file operations
    - Read and write raw byte sequences to files. Concerns: **endianness** (byte order), fixed-size records, seeking (file offsets), alignment, and structured serialization/deserialization.

  - Directory management
    - Enumerate directory entries, create/remove directories, manipulate and normalize file paths, and read file metadata (size, timestamps, permissions). Account for platform-specific path separators and permission semantics.

  - Cross-platform TCP/UDP socket abstractions
    - **TCP**: connection-oriented byte-stream transport providing reliable, ordered delivery; APIs include connect/listen/accept/read/write/close.
    - **UDP**: connectionless datagram transport providing message-oriented send/receive without delivery or ordering guarantees.
    - Abstractions normalize differences between socket implementations (**POSIX** vs **Winsock**), initialization, error codes, and socket options.

### Development Tools

- Command-line argument parsing
- Configuration file handling
- Logging and debugging utilities

  - Command-line argument parsing
    - Parse `argc/argv` into typed options: boolean flags, options with values (*e.g.*, `--file <path>`), and positional arguments. Validate values, provide defaults, and produce clear error messages for malformed inputs.

  - Configuration file handling
    - Parse structured configuration formats (**INI**, **JSON**, or custom key-value), validate types and ranges, merge with defaults, and expose typed configuration to the program.

  - Logging and debugging utilities
    - **Logging**: emit timestamped messages with severity levels (debug/info/warn/error), configurable outputs (console, file), and optional rotation/retention. Useful for post-mortem analysis.
    - **Debugging utilities**: assertions, diagnostic hooks, and instrumentation for inspecting invariants and internal state during development builds.

### Media Abstractions

- 2D graphics primitives for bitmaps and sprites
- PCM audio playback and mixing concepts
- Input handling for keyboard and mouse

  - 2D graphics primitives for bitmaps and sprites
    - **Bitmap**: rectangular array of pixels with width, height, and a pixel format (*e.g.*, 24-bit RGB, 32-bit RGBA). Operations: pixel read/write, blit (copy), scaling, and alpha blending.
    - **Sprite**: image or sub-image with position and optional transform parameters (translation, rotation, scale); rendering composes sprites onto a framebuffer using blend modes.

  - PCM audio playback and mixing concepts
    - **PCM (Pulse-Code Modulation)**: discrete audio samples per channel at a fixed sample rate (*e.g.*, 44100 Hz). Each sample is a numeric amplitude. Playback involves buffering frames to an audio device and handling latency.
    - **Mixing**: combine multiple PCM streams by summing samples with scaling to prevent clipping; may require resampling or channel mapping when sources differ.

  - Input handling for keyboard and mouse
    - **Keyboard**: capture key codes and state transitions (down/up) and optionally text input with Unicode composition. Include timestamps and repeat handling.
    - **Mouse**: capture pointer coordinates, button states, wheel deltas, and relative motion. Provide event-driven and polling interfaces, and map raw device coordinates to window/screen coordinates.

## Distribution

This project is released under multiple licenses. Users may choose to operate under any or all of the following:

- Anti-Capitalist Software License (v1.4)
- Fuck Around and Find Out License (v0.1)  
- Hippocratic License (v3.0)

Full license texts are available in LICENSE.txt.

## Development

This particular branch of COMMON-C is maintained by members of **Elastic Softworks**.

All libraries adhere to:

- C89/ANSI C standard compliance
- C-FORM code formatting rules
- Zero external dependencies
- Cross-platform compatibility focus

## Usage

Libraries can be integrated individually or as a complete suite. Each component maintains its own documentation and examples within its respective directory.

The modular design allows developers to include only needed functionality without carrying unnecessary overhead.

## Contributing

Contributions follow horizontal collaboration principles. Submit patches, improvements, or new libraries through available federation channels. All contributions must maintain C89 compliance and follow project formatting standards.

## Resources

Additional documentation, tutorials, and examples can be found archived at [LibMephisto](http://71.131.94.115/mediawiki/).
