CS6135 VLSI Physical Design Automation Homework 2

Title: k-way Min-Cut Partitioning using Fiduccia-Mattheyses Algorithm
Author: 劉期正(Liu Chi Cheng) 
Student ID: 114062615
Date: October 2025

1. Envorionment
Platform:Linux/ic51
Compiler:g++ (GCC) 9.3.0
C++ language version:C++11
Build tool:Makefile
Executable path: HW2/bin/hw2

2. How to compile

enter the src directory, and use makefile to compile

$ cd src
$ make

If you want to remove, use make clean

$ make clean

3. How to run
Run the program using the following command format:

$ ./hw2 <input_file> <output_file> <number_of_partitions>
< input_file >: path to input hypergraph file
< output_file >: path to output file
< number_of_partitions >: 2 or 4

example:

$ ./hw2 ../testcase/public1.txt ../output/public1.2way.out 2

If you want to run all the testcase in the testcase directory,
you can use "make run" command 

$ make run

4. Output format
CutSize <value>
GroupA <num_cells>
C1
C2
...
GroupB <num_cells>
...
[GroupC / GroupD if k=4]

5. Directory structure
HW2/
├── src/
│   ├── main.cpp
│   ├── fm.cpp
│   ├── fm.h
│   ├── graph.cpp
│   ├── graph.h
│   ├── Makefile
│   └── README
├── bin/
│   └── hw2
├── testcase/
│   ├── public1.txt
│   ├── public2.txt
│   └── ...
├── output/
│   ├── public1.2way.out
│   ├── public1.4way.out
│   └── ...
└── CS6135_HW2_114062615_report.pdf

6. Implementation Summary

(1)Utilized a bucket list data structure to store gain
(2)Extended FM to 4-way partitioning by modifying gain computation and balance checking.
(3)Improved the initial partitioning method by combining random shuffle and cell importance sorting for better starting points.




