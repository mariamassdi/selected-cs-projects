

from __future__ import annotations

import sys


class Cache:

    
    def __init__(
        self,
        line_size: int,
        data_size: int,
        num_ways: int,
    ) -> None:
    
        self.line_size = line_size
        self.data_size = data_size
        self.num_ways = num_ways
        
        # Calculate cache geometry
        self.num_lines = data_size // line_size
        self.num_sets = self.num_lines // num_ways
        
        # Initialize sets: each set is a list of block numbers (LRU to MRU)
        self._sets: list[list[int]] = [[] for _ in range(self.num_sets)]
    
    def get_set_index(self, block_number: int) -> int:
  
        return block_number % self.num_sets
    
    def contains(self, block_number: int) -> bool:
     
        set_idx = self.get_set_index(block_number)
        return block_number in self._sets[set_idx]
    
    def access(self, block_number: int) -> bool:
   
        set_idx = self.get_set_index(block_number)
        cache_set = self._sets[set_idx]
        
        if block_number not in cache_set:
            return False
        
        # Move to MRU position (end of list)
        cache_set.remove(block_number)
        cache_set.append(block_number)
        return True
    
    def insert(self, block_number: int) -> int | None:
    
        set_idx = self.get_set_index(block_number)
        cache_set = self._sets[set_idx]
        
        # If already present, just update LRU position
        if block_number in cache_set:
            cache_set.remove(block_number)
            cache_set.append(block_number)
            return None
        
        # Evict LRU if set is full
        evicted = None
        if len(cache_set) >= self.num_ways:
            evicted = cache_set.pop(0)  # Remove LRU
        
        # Insert new block at MRU position
        cache_set.append(block_number)
        return evicted
    
    def invalidate(self, block_number: int) -> bool:

        set_idx = self.get_set_index(block_number)
        cache_set = self._sets[set_idx]
        
        if block_number in cache_set:
            cache_set.remove(block_number)
            return True
        return False
    
    def get_set_contents(self, set_index: int) -> list[int]:

        return self._sets[set_index].copy()


class CacheSimulator:

    def __init__(
        self,
        line_size: int,
        inclusive: bool,
        l1_ways: int,
        l1_size: int,
        l2_ways: int,
        l2_size: int,
    ) -> None:

        self.line_size = line_size
        self.inclusive = inclusive
        
        # Create L1 and L2 caches
        self.l1 = Cache(line_size=line_size, data_size=l1_size, num_ways=l1_ways)
        self.l2 = Cache(line_size=line_size, data_size=l2_size, num_ways=l2_ways)
    
    def address_to_block(self, address: int) -> int:

        return address // self.line_size
    
    def process_read(self, block_number: int) -> str:
 
        # Case 1: L1 hit
        if self.l1.access(block_number):
            return "L1HIT"
        
        # Case 2: L1 miss, check L2
        if self.l2.access(block_number):
            # L2 hit - insert into L1
            self.l1.insert(block_number)
            return "L2HIT"
        
        # Case 3: Miss in both - fetch from memory
        # Insert into L2 first (for inclusive safety)
        evicted_from_l2 = self.l2.insert(block_number)
        
        # If inclusive mode and L2 evicted a block, invalidate it in L1
        if self.inclusive and evicted_from_l2 is not None:
            self.l1.invalidate(evicted_from_l2)
        
        # Insert into L1
        self.l1.insert(block_number)
        
        return "MEMACC"
    
    def process_write(self, block_number: int) -> str:

        # Case 1: L1 hit
        if self.l1.access(block_number):
            # L2 LRU is NOT updated on L1 hit (access does not reach L2)
            return "L1HIT"
        
        # Case 2: L1 miss, check L2
        if self.l2.access(block_number):
            # L2 hit - do NOT insert into L1 (no-write-allocate)
            return "L2HIT"
        
        # Case 3: Miss in both caches
        # Do not insert into either cache (no-write-allocate)
        return "MEMACC"

    def process_access(self, address: int, operation: str) -> str:

        block_number = self.address_to_block(address)
        if operation == "R":
            return self.process_read(block_number)
        else:
            return self.process_write(block_number)


def parse_config(config_text: str) -> dict:

    parts = [p.strip() for p in config_text.split(",")]

    if len(parts) != 6:
        raise ValueError(f"Expected 6 comma-separated values, got {len(parts)}")

    line_size = int(parts[0])
    inclusive_str = parts[1].upper()
    inclusive = inclusive_str == "TRUE"
    l1_ways = int(parts[2])
    l1_size = int(parts[3])
    l2_ways = int(parts[4])
    l2_size = int(parts[5])

    return {
        "line_size": line_size,
        "inclusive": inclusive,
        "l1_ways": l1_ways,
        "l1_size": l1_size,
        "l2_ways": l2_ways,
        "l2_size": l2_size,
    }


def parse_trace(trace_text: str) -> list[tuple[int, str]]:

    accesses = []
    for line in trace_text.splitlines():
        line = line.strip()
        if not line:
            continue
        parts = [p.strip() for p in line.split(",")]
        if len(parts) != 2:
            continue
        address = int(parts[0], 16)
        operation = parts[1]
        accesses.append((address, operation))
    return accesses


def main() -> None:

    if len(sys.argv) != 4:
        print("Usage: python3 cache_sim.py <config_file> <trace_file> <output_file>", file=sys.stderr)
        sys.exit(1)

    config_path = sys.argv[1]
    trace_path = sys.argv[2]
    output_path = sys.argv[3]

    # Read and parse configuration
    with open(config_path, "r") as f:
        config_text = f.read()
    config = parse_config(config_text)

    # Create simulator
    sim = CacheSimulator(
        line_size=config["line_size"],
        inclusive=config["inclusive"],
        l1_ways=config["l1_ways"],
        l1_size=config["l1_size"],
        l2_ways=config["l2_ways"],
        l2_size=config["l2_size"],
    )

    # Read and parse trace
    with open(trace_path, "r") as f:
        trace_text = f.read()
    accesses = parse_trace(trace_text)

    # Process each access and collect results
    results = []
    for address, operation in accesses:
        result = sim.process_access(address, operation)
        results.append(result)

    # Write results to output file
    with open(output_path, "w") as f:
        for result in results:
            f.write(result + "\n")


if __name__ == "__main__":
    main()
