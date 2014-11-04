Linux Kernel Hack Challenge
===========================
Linux Kernel Hack Challenge is a contest to encourage Linux professionals to solve an execution time measurement problem. The winner solution will be awarded with a flight in a SAAB Gripen simulator including travel to Linköping, and accommodation.
The task is to provide a Linux kernel modification that gives a more accurate worst-case execution time measurement taking into account cache effects. The winning solution may be used to verify execution time aspects of airborne software.

### Challenging tasks 
* Execute small applications uninterrupted by Linux in a small time-slot.
* Force a cache invalidation/flush at a specific point in time.

### Requirements
* Linux kernel shall be 2.6.32, PowerPC e500v2.
* Deadline for your solution is 2015-02-28.

### Prices
A flight in a SAAB Gripen simulator including a trip to Linköping, and accommodation. 

### Benchmarking
Benchmarking is done using acceptance test found in [full specification] (https://github.com/christoffer-nylen/linux-kernel-hack-challenge/raw/master/doc/KernelHackSpecification-1.0.0.docx).

### The rules
Prize is limited to European residents.
All submitted solutions must comply with GPL v2 license.
The winner takes it all!

### How do I proceed?
Create a fork of this repo, clone it and initilize the linux submodule (which points at torvalds/linux 2.6.32).
```
$ git clone https://github.com/<your repo>
$ git submodule sync
$ git submodule update --init linux
```


We accept solutions as pull requests to this repo from 2015-02-25 to 2015-02-28.



### ./README.md
* guides to other material
* FAQ for the linux kernel hack challenge

### ./FAILED_profiling_lib
Contains a FAILED attempt of a profiling library that contains the required APIs
according to header file libprf.h

### ./FAILED_Linux_kernel_patch_attempt
Contains a FAILED attempt to change the Linux kernel scheduling - can be an
inspiration?
Uses a kernel module that must be installed to set up the interrupt handling.
L1 invalidation cache for powerPC used here.
Note: This is vital and must be used (powerPC e500v2) to be used by interrupt routine to 
invalidate L1 cache. 

### ./L1_invalidate_Kernel_module
Example of how to Invalidate L1 cache on a powerPC e500v2 architecture.
Note: This is vital and must be used (powerPC) to be used by interrupt routine 
to invalidate L1 cache. 

### ./testcase
Test case that may prove the correctness of achieved work. Corresponds to Full 
specification, Acceptance test 6. The principal characteristics of measured values 
for this test when working is found in Powerpoint picture in ./doc/

### ./linux-2.6.32
config file for the linux kernel 2.6.32.

### ./linux
Submodule pointing at https://github.com/torvalds/linux/tree/v2.6.32

### ./doc
* Short version of Linux kernel hack challenge.
* Longer technical specification of task, including Acceptance tests that define 
  behaviour of profiling when working.
* PowerPoint picture of expected time usage behaviour for the test case as L1 cache
is invalidated at different points in time from execution start.

### FAQ
* Q: Who arranges this challenge?

Saab Aeronautics development department in Linköping, Sweden. 
Saab Aeronautics offers advanced airborne systems, related subsystems, 
Unmanned Aerial Systems (UAS), aerostructures and services to defense customers 
and commercial aerospace industries worldwide. Aeronautics is also responsible for 
development, production, marketing, selling and supporting of the Gripen fighter.

* Q: Why did you launch this contest?

The goal of this contest is to solve a real-time problem. 
The question at hand is whether Linux can be used to measure worst case execution time.

* Q: What happens when I win?

The best proposed solution, will be published on github and the winner will be announced. 
Saab contacts the winner with all details for the prize, travel, and accommodation.