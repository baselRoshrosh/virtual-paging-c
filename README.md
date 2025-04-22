# Virtual Memory Paging Simulation (C Project)

This is a university project that simulates virtual memory management using paging in C.

## 🧠 Overview

The program simulates a simple **Memory Management Unit (MMU)** with a virtual 32-bit address space and a 16-bit physical address space. Pages are swapped in and out of RAM and disk (simulated arrays), using a FIFO page replacement strategy.

## 📂 Project Files

- `main.c`: The full source code with all logic.
- `Aufgabenstellung.pdf`: Original assignment description (in German).

## 🔧 Features

- Translation of virtual to physical address
- Page Table with Present and Dirty bits
- FIFO Page Replacement Strategy
- Simulated `ra_mem` (RAM) and `hd_mem` (hard disk)
- `get_data()` and `set_data()` API
- Error checking and test cases in `main()`

## ⚙️ How to Compile

```bash
gcc main.c -o hu_soft_mmu -fno-stack-protector -g -fsanitize=address -std=c11
