
# Computer Architecture

Selected projects developed as part of the Computer Architecture course at the Technion – Israel Institute of Technology.

These projects focus on processor behavior, branch prediction, memory hierarchy, and hardware-level mechanisms through software simulation in Python.

## Projects

### 1. Branch Predictor

[View Source Code](branch-predictor/predictor.py)

Implementation of the core prediction and update logic of a configurable two-level branch predictor.

**Technical highlights:**

- 2-bit saturating counters.
- Local and global branch history.
- Branch History Register (BHR).
- Global History Register (GHR).
- Pattern History Tables (PHT).
- XOR-based indexing (gshare and lshare).
- Address indexing, tag extraction, and bitwise operations.
- Configurable prediction mechanisms.

### 2. Cache Simulator

[View Source Code](cache-simulator/cache_sim.py)

Implementation of a configurable two-level cache simulator modeling memory access behavior and cache hierarchy.

**Technical highlights:**

- L1 and L2 cache hierarchy.
- Set-associative cache organization.
- Least Recently Used (LRU) replacement policy.
- Inclusive and non-inclusive cache configurations.
- Read and write access handling.
- No-write-allocate policy.
- Cache hit and miss classification.
- Configurable cache size, line size, and associativity.
- Memory trace processing.

## Technologies and Skills

- Python
- Computer Architecture
- Memory Hierarchy
- Branch Prediction
- Cache Management
- Bitwise Operations
- Hardware Behavior Simulation
- Algorithmic Problem-Solving

## Academic Context

Developed as part of the B.Sc. in Computer Science at the Technion.
 
## AI-Assisted Development

These projects were developed with assistance from OpenCode, an AI-powered development tool provided by the Technion as part of the course.
