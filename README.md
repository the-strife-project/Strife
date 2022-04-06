# jotaOS
## Introduction
jotaOS is an x86_64 microkernel operating system made in C++ from scratch. It doesn't aim to be a substitute for any existing one, but:

- It's an exercise for myself
- It's a way to show the feasibility of the different ideas I propose
- Not long ago, I decided it to be my TFG (final undergraduate project)

The whole project aims to be two things:

- Secure. Mainly due to compartmentalization (each process has its own local view of the system), but also making some already existing security measures as non-optional (RELRO, NX stack...)
- Beautiful. When possible and appropiate, jotaOS gives simple solutions to complex problems, even if they're not the absolute fastest. I've done my very best to write easy to read code. Note that the code is not a tutorial, do not take everything I do as the best solution, or even a good one. Have your own judgment.

This is a full operating system, not a kernel or a Linux distribution. It has a microkernel and a userspace, with its drivers and tools. The only parts that are not written by me are the bootloader (currently using [Limine](https://github.com/limine-bootloader), which I recommend), and the user-space allocator (Durand's `liballoc`, which I will probably change in the future).

This is fruit of many years of research and debugging. It's the work of my life. Do not undervalue the thousands of hours of effort put into this project.

This repo is the official jotaOS distribution. Since all projects have their own repository in [this GitHub organization](https://github.com/jotaOS), there has to be a way to glue them all together. This distribution is stable: it will never intentionally contain unfinished parts that break everything. It also has a simple Makefile which compiles all the programs in [the `projects` directory](https://github.com/jotaOS/jotaOS/tree/master/projects) and creates an ISO file. In the very near future, it will also have a CI set up so you can just download the latest ISO.