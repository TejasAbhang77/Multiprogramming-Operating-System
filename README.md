# Multiprogramming Operating System (MOS) Project

This repository contains the implementation of a **Multiprogramming Operating System (MOS)** as part of a college-level systems programming project. It is divided into two phases, each introducing key operating system concepts like job control, interrupt handling, paging, and virtual memory.

---

## 📂 Project Structure

```text
MOS-Project/
├── phase1.cpp          # Phase I implementation (basic OS simulation)
├── phaseII.cpp         # Phase II implementation (adds paging, errors, multiprogramming)
├── input1.txt          # Input for Phase I
├── output1.txt         # Output for Phase I
├── input2.txt          # Input for Phase II
├── output2.txt         # Output for Phase II
├── MOS_phaseI_doc.pdf  # Documentation for Phase I
├── MOS_phaseII_doc.pdf # Documentation for Phase II
├── README.md           # Project overview and usage
```

## 🚀 Phase I Overview

**Key Features:**
- Simulates a basic OS loader and executor.
- Handles simple instructions: `LR`, `SR`, `CR`, `BT`, `GD`, `PD`, `H`.
- Supports one job at a time.
- Uses `SI` interrupts for I/O and termination.
- Outputs are spaced by two blank lines for each job.

**Assumptions:**
- No multiprogramming.
- Programs are loaded starting from memory location 00.
- Job control using `$AMJ`, `$DTA`, and `$END`.

---

## 🔄 Phase II Enhancements

**New Features Added:**
- **Paging and Virtual Memory Management**
- **Interrupt Handling:**
  - `SI` for service requests
  - `TI` for time limits
  - `PI` for program errors (opcode, operand, page faults)
- **Random page allocation**
- **Page Table stored in memory**
- **Error messages and job termination messages**

**Errors Handled:**
- Out of data
- Line limit exceeded
- Time limit exceeded
- Operation code error
- Operand error
- Invalid page fault

---

## 🧪 How to Run

1. Compile the desired phase:
```bash
g++ phase1.cpp -o phase1
./phase1

g++ phaseII.cpp -o phaseII
./phaseII

```
## 👥 Contributors

- **Tejas Abhang** – [GitHub](https://github.com/TejasAbhang77)
- **Adnaan Momin** – [GitHub](https://github.com/Adnaan29)
