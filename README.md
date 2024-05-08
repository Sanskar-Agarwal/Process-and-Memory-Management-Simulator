

<h1>Description</h1>

This project involves creating a simulation of a process manager designed to manage process scheduling and memory allocation in a system environment where all processes are fully CPU-bound. The simulation handles process scheduling using a round-robin algorithm and supports three types of memory management strategies: contiguous, paged, and virtual.

<h1>Features</h1>

<b>Process Scheduling:</b> Implements a round-robin scheduling algorithm to manage processes in a queue.<br>
<b>Memory Management:</b> Includes three memory management strategies:<br>
<b>Contiguous Memory Allocation:</b> Uses the First Fit algorithm for memory allocation.<br>
<b>Paged Memory Allocation:</b> Implements a system with swapping, where processes are allocated memory in pages.<br>
<b>Virtual Memory Allocation:</b> Simulates a virtual memory system, allowing partial allocation of pages.<br>
<b>Performance Metrics:</b> Calculates and displays key performance metrics such as average turnaround time, time overhead, and makespan.<br>


<h1>How to run</h1>

Compile using provided Makefile<br>

./allocate -f cases/task1/spec.txt -q 1 -m infinite | diff - cases/task1/spec-q1.out<br>
./allocate -f cases/task1/two-processes.txt -q 1 -m infinite | diff - cases/task1/two-processes-q1.out<br>
./allocate -f cases/task1/two-processes.txt -q 3 -m infinite | diff - cases/task1/two-processes-q3.out<br><br>

./allocate -f cases/task2/fill.txt -q 3 -m first-fit | diff - cases/task2/fill-q3.out<br>
./allocate -f cases/task2/non-fit.txt -q 1 -m first-fit | diff - cases/task2/non-fit-q1.out<br>
./allocate -f cases/task2/retake-left.txt -q 3 -m first-fit | diff - cases/task2/retake-left-q3.out<br>
./allocate -f cases/task2/consecutive-running.txt -q 3 -m first-fit | diff - cases/task2/consecutive-running-q3.out<br><br>

./allocate -f cases/task3/simple-alloc.txt -q 3 -m paged | diff - cases/task3/simple-alloc-q3.out<br>
./allocate -f cases/task3/simple-evict.txt -q 1 -m paged | diff - cases/task3/simple-evict-q1.out<br>
./allocate -f cases/task3/internal-frag.txt -q 1 -m paged | diff - cases/task3/internal-frag-q1.out<br><br>

./allocate -f cases/task4/no-evict.txt -q 3 -m virtual | diff - cases/task4/no-evict-q3.out<br>
./allocate -f cases/task4/virtual-evict.txt -q 1 -m virtual | diff - cases/task4/virtual-evict-q1.out<br>
./allocate -f cases/task4/virtual-evict-alt.txt -q 1 -m virtual | diff - cases/task4/virtual-evict-alt-q1.out<br>
./allocate -f cases/task4/to-evict.txt -q 3 -m virtual | diff - cases/task4/to-evict-q3.out<br>

./allocate -f cases/task1/spec.txt -q 1 -m infinite | diff - cases/task1/spec-q1.out<br>
./allocate -f cases/task2/non-fit.txt -q 3 -m first-fit | diff - cases/task2/non-fit-q3.out<br>
./allocate -f cases/task3/simple-alloc.txt -q 3 -m paged | diff - cases/task3/simple-alloc-q3.out<br>
./allocate -f cases/task4/to-evict.txt -q 3 -m virtual | diff - cases/task4/to-evict-q3.out<br><br>

<h3>Technologies</h3>
C programming language
Unix-based operating systems for execution

<h3>Authors</h3>
Sanskar Agarwal
