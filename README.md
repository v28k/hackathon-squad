# HACKATHON SQUAD

Maximum Weight Independent Set (MWIS) heuristic solver for selecting the strongest possible conflict-free hackathon team.

---

## Problem Statement

A university hackathon requires building the strongest possible team from a large pool of freshman coders.

Each coder has:
- A skill rating
- Possible rivalries/conflicts with other coders

The goal is to select a subset of coders such that:
- No two selected coders are in conflict
- The total skill rating is maximized

This is a classic Maximum Weight Independent Set (MWIS) problem in graph theory.

- Coders → Nodes
- Conflicts → Edges
- Valid team → Independent Set
- Objective → Maximum total node weight

Since MWIS is NP-Hard, exact solutions are infeasible for very large graphs.

This project implements a high-performance heuristic + optimization pipeline capable of handling graphs with up to:
- N = 200,000 nodes
- Dense conflict structures
- 5-minute optimization runtime

---

# Features

- Multi-strategy greedy initialization
- Local search optimization
- Simulated annealing
- Periodic random restarts
- Adaptive heat boosting
- O(N + M) memory complexity
- Fully conflict-free guaranteed output

---

# Algorithm Overview

The solution uses a 4-stage optimization pipeline.

## 1. Multi-Strategy Greedy Initialization

Four different greedy orderings are used:

### Density First
Prioritizes:
skill / degree

High-value low-conflict coders.

### Raw Skill First
Always chooses the highest skill coder available.

### Fewest Conflicts First
Keeps future options open by selecting low-degree nodes.

### Net Value First
Uses:
skill − neighbour penalty

to estimate long-term benefit.

The best greedy result is selected as the starting point.

---

## 2. Local Search Polish

After greedy construction:

### Fill Pass
Adds any coder with zero conflicts against the current team.

### Swap Pass
Attempts profitable replacements:
- remove weaker coders
- insert stronger available coders

Repeated until no improvement remains.

---

## 3. Simulated Annealing

The core optimization engine.

The algorithm performs millions of:
- ADD moves
- DROP moves
- SWAP moves

Temperature-based probabilistic acceptance allows escaping local optima.

### Move Types

#### ADD
Add a non-conflicting coder.

#### DROP
Temporarily remove a coder to escape local traps.

#### SWAP
Remove all conflicting selected coders and insert a stronger candidate.

---

## 4. Random Restart

Every 30 seconds:
- randomly remove ~10% of the current solution
- allow SA to rebuild differently

This helps explore new regions of the search space.

---

# Complexity

| Component | Complexity |
|---|---|
| Greedy Build | O(N + M) |
| Local Search | O(N + M) |
| SA Move | O(deg(u)) |
| Restart | O(N + M) |
| Space | O(N + M) |

---

# Project Structure

```text
hackathon-squad/
├── src/
│   └── hackathon_squad_final.cpp
├── report/
│   └── Hackathon_Squad_MWIS_Submission_Report.pdf
├── samples/
│   ├── input1.txt
│   └── output1.txt
├── README.md
└── .gitignore
```

---

# Input Format

```text
N M
S1 S2 S3 ... SN
u1 v1
u2 v2
...
uM vM
```

Where:
- N = number of coders
- M = number of conflict pairs
- Si = skill rating of coder i
- (u,v) = rivalry/conflict edge

---

# Output Format

```text
<maximum total skill>
<selected coder indices in ascending order>
```

---

# Example

## Input

```text
4 3
5 3 4 6
1 2
2 3
3 4
```

## Output

```text
9
1 3
```

---

# Compilation

## Linux / macOS

```bash
g++ -O3 -std=c++17 -o hackathon_squad src/hackathon_squad_final.cpp
```

## Windows (MinGW / MSYS2)

```bash
g++ -O3 -std=c++17 -o hackathon_squad.exe src/hackathon_squad_final.cpp
```

---

# Running

## Linux / macOS

```bash
./hackathon_squad < input.txt > output.txt
```

## Windows

```bash
hackathon_squad.exe < input.txt > output.txt
```

---

# Requirements

- C++17 compatible compiler
- g++ 7+ or clang++ 5+
- No external dependencies

---

# Correctness Guarantee

The algorithm always maintains a valid independent set.

At every stage:
- coders are added only if conflict count is zero
- swap moves remove all conflicting selected coders first
- conflict counters are updated atomically

Therefore:
- the final team is always conflict-free

---

# Notes

- The algorithm is optimized for very large graphs.
- Runtime is designed for a 5-minute competitive optimization setting.
- Longer runtime generally improves solution quality.

---

## Team Members

- Venu K
- NSDNV Manjunadh
- P Srinadh

---
