# cpp09-ford-johnson-sort
C++98 implementation of Ford-Johnson (Merge-Insertion) sort using std::vector and std::deque, with a Bitcoin exchange rate lookup tool and a Reverse Polish Notation calculator — CPP Module 09.
# CPP Module 09 — Ford-Johnson Sort & STL Algorithms

> C++98 implementations of three classic algorithms using STL containers:
> a Bitcoin exchange rate tool, a Reverse Polish Notation calculator, and the Ford-Johnson (Merge-Insertion) sort.

---

## Table of Contents

- [Overview](#overview)
- [ex00 — Bitcoin Exchange](#ex00--bitcoin-exchange)
- [ex01 — Reverse Polish Notation](#ex01--reverse-polish-notation)
- [ex02 — PmergeMe (Ford-Johnson Sort)](#ex02--pmergeme-ford-johnson-sort)
- [Build](#build)
- [Concepts Covered](#concepts-covered)

---

## Overview

This module is the final project of the C++ piscine at 42 School. Each exercise forces you to pick the right STL container for a real problem, write defensive input validation, and understand performance trade-offs between data structures.

| Exercise | Algorithm / Container | Key Concept |
|---|---|---|
| ex00 | `std::map` | Ordered date lookup, closest-previous key |
| ex01 | `std::stack` | Postfix expression evaluation |
| ex02 | `std::vector` + `std::deque` | Ford-Johnson / Merge-Insertion sort |

All exercises are compiled in **C++98** with strict flags:
```
-Wall -Wextra -Werror -std=c++98
```

---

## ex00 — Bitcoin Exchange

### What it does

Loads a CSV database of historical Bitcoin exchange rates (`data.csv`), then reads an input file. For each line it finds the correct exchange rate — using the exact date if available, otherwise the closest earlier date — and prints the computed value.

### Usage

```bash
./btc input.txt
```

### Input file format

```
date | value
2011-01-03 | 3
2011-01-05 | 1.5
2012-01-11 | 0.25
```

### Expected output

```
2011-01-03 => 3 = 0.9
2011-01-05 => 1.5 = 0.6
2012-01-11 => 0.25 = 2.43
```

### Error handling

```
Error: could not open file
Error: bad input => 2011-99-99 | 3
Error: date is in the future.
Error: not a positive number.
Error: too large a number.
Error: no earlier date in database
```

### Core container: `std::map<std::string, double>`

`std::map` keeps keys sorted in lexicographical order. Because dates are formatted as `YYYY-MM-DD`, lexicographical order matches chronological order — no custom comparator needed.

```
"2009-12-31" < "2010-01-01" < "2011-05-20"  ✓
```

**Exact lookup** — `map::find()`:
```cpp
it = _database.find(date);
if (it != _database.end())
    return it->second;  // O(log N)
```

**Closest previous date** — `map::upper_bound()`:
```cpp
it = _database.upper_bound(date);  // first date strictly after
if (it == _database.begin())
    throw std::runtime_error("Error: no earlier date in database");
--it;                              // step back to closest earlier
return it->second;
```

### Algorithm pipeline

```
data.csv ──► std::map<date, rate>
                        │
input.txt ──► validate  ├──► find() exact match
              date/value │
                        └──► upper_bound() − 1 → closest earlier
                                        │
                                  value × rate ──► print
```

---

## ex01 — Reverse Polish Notation

### What it does

Evaluates a postfix arithmetic expression passed as a single command-line argument using a `std::stack<int>`.

### Usage

```bash
./RPN "8 9 * 9 - 9 - 9 - 4 - 1 +"
# Output: 42
```

```bash
./RPN "3 4 +"
# Output: 7
```

### Rules

- Operators supported: `+` `-` `*` `/`
- Numbers are single digits only (< 10)
- No brackets, no floating-point
- Prints `Error` for any invalid input

### Core container: `std::stack<int>`

A stack provides **LIFO** (Last In, First Out) behavior — exactly what postfix evaluation needs.

| Token type | Action |
|---|---|
| Digit | `stack.push(digit)` |
| Operator | `pop` two operands → compute → `push` result |
| End of input | Stack must contain exactly **one** value |

**Stack evolution for `8 9 * 9 - 4 - 1 +`:**

| Token | Stack (bottom → top) |
|---|---|
| `8` | [8] |
| `9` | [8, 9] |
| `*` | [72] |
| `9` | [72, 9] |
| `-` | [63] |
| `4` | [63, 4] |
| `-` | [59] |
| `1` | [59, 1] |
| `+` | [60] |

**Result: 60**

### Error cases

| Case | Triggered when |
|---|---|
| Wrong argument count | `argc != 2` |
| Invalid token | Not a single digit and not an operator |
| Not enough operands | Operator with fewer than 2 items on stack |
| Division by zero | Right operand is `0` |
| Leftover values | Stack has more than 1 item at end |

---

## ex02 — PmergeMe (Ford-Johnson Sort)

### What it does

Sorts a sequence of positive integers from the command line using the **Ford-Johnson algorithm** (also known as Merge-Insertion Sort), run independently on both `std::vector` and `std::deque`. Prints the sequence before and after sorting, plus the time taken by each container.

### Usage

```bash
./PmergeMe 3 5 9 7 4 1 8 2 6
```

### Expected output

```
Before: 3 5 9 7 4 1 8 2 6
After:  1 2 3 4 5 6 7 8 9
Time to process a range of 9 elements with std::vector : 0.005 us
Time to process a range of 9 elements with std::deque  : 0.007 us
```

### Error handling

```bash
./PmergeMe 3 -1 foo
# Error
```

Rejects: non-integer tokens, negative numbers, integer overflow.

---

### The Ford-Johnson Algorithm

Ford-Johnson is designed to **minimize the number of comparisons** when sorting. It combines merge sort and insertion sort in a carefully engineered way.

#### The three phases

**Phase 1 — Pair and compare**

Elements are grouped into pairs. Each pair costs exactly **one comparison** to order:

```
Input:  7  4  9  3  8  1  6
Pairs: (7,4) (9,3) (8,1)   extra: 6
After:  (4,7) (3,9) (1,8)
```

The larger element of each pair is called **big** (main chain).
The smaller is called **small** (pend chain).

**Phase 2 — Recursively sort the main chain**

Only the big elements are sorted recursively:

```
main = [7, 9, 8]  →  sorted main = [7, 8, 9]
```

This halves the problem size at each recursion level.

**Phase 3 — Insert the pend chain (Jacobsthal order)**

The small elements are inserted into the sorted main chain using **binary search with bounded ranges**:

```
sorted main: [7, 8, 9]
insert 4 (bound = 7) → [4, 7, 8, 9]
insert 3 (bound = 9) → [3, 4, 7, 8, 9]
insert 1 (bound = 8) → [1, 3, 4, 7, 8, 9]
insert 6 (no bound)  → [1, 3, 4, 6, 7, 8, 9]
```

The **bound** is key: since `small ≤ big` in each pair, the small element must appear before its partner big in the final array. This shrinks each binary search range.

---

#### The Jacobsthal Sequence

The **order** in which pending elements are inserted matters for minimizing comparisons. The Jacobsthal sequence determines this order.

The sequence is defined by:

```
J(0) = 0
J(1) = 1
J(n) = J(n-1) + 2 * J(n-2)

Result: 0, 1, 1, 3, 5, 11, 21, 43, 85 ...
```

The algorithm uses Jacobsthal numbers to batch insertions in a way that keeps binary search ranges structured and short — a result proven by Knuth to be comparison-optimal (or near-optimal) for small to medium inputs.

---

#### Recursion tree

```
sort(n elements)
  └─ sort(~n/2 big elements)
        └─ sort(~n/4 big elements)
              └─ sort(~n/8 big elements)
                    └─ base case (size ≤ 1)
```

Recursion depth: `O(log n)`
Total time complexity: `O(n log n)` — but with fewer comparisons than standard mergesort.

---

#### Why two containers?

The subject requires running the **same algorithm** on both `std::vector` and `std::deque` to observe and compare real performance:

| Container | Memory layout | Access speed |
|---|---|---|
| `std::vector` | Contiguous array | Faster (cache-friendly) |
| `std::deque` | Multiple fixed-size chunks | Slightly slower (extra indirection) |

The timing difference is typically small for short sequences but grows noticeable at thousands of elements.

---

## Build

Each exercise has its own directory and `Makefile`:

```bash
# Build all
cd ex00 && make && cd ..
cd ex01 && make && cd ..
cd ex02 && make && cd ..

# Clean
make fclean
```

Binaries produced: `btc`, `RPN`, `PmergeMe`

---

## Concepts Covered

| Concept | Where |
|---|---|
| `std::map` — sorted associative container | ex00 |
| `map::find()` — O(log N) exact lookup | ex00 |
| `map::upper_bound()` — closest-previous key | ex00 |
| `std::ifstream` — safe file reading | ex00 |
| `std::stringstream` — line splitting by delimiter | ex00, ex01 |
| `std::stack` — LIFO operand management | ex01 |
| Postfix (RPN) expression evaluation | ex01 |
| Ford-Johnson / Merge-Insertion sort theory | ex02 |
| Jacobsthal sequence for insertion ordering | ex02 |
| Bounded binary insertion | ex02 |
| `std::vector` vs `std::deque` performance | ex02 |
| `clock()` microsecond timing | ex02 |

---

## References

- D.E. Knuth, *The Art of Computer Programming*, Vol. 3 — Sorting and Searching, §5.3.1
- Ford-Johnson algorithm overview: https://medium.com/@mohammad.ali.ibrahim.525/ford-johnson-algorithm-merge-insertion-4b024f0c3d42
- 42 School CPP Module 09 subject
