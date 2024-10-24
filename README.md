# Kaze Operating System

Kaze is a minimal operating system (OS) designed with a simple, in-memory filesystem. The OS is written in a combination of Assembly and C, focusing on providing basic filesystem functionalities, like file creation, reading, writing, listing, and deletion.

<p align="center">
<img src="https://raw.githubusercontent.com/Jahirrrr/perpus21-page/refs/heads/main/jadicoy.png" alt="image"></a>
</p>

The project simulates a file system in memory, managing files and directories without reliance on external libraries like stdio.h or string.h. A custom kprint function is used for console output.

# Features
- Custom Filesystem: A custom in-memory filesystem that supports file creation, writing, reading, and deletion.
- No Dependency on Standard Libraries: Implemented without the use of stdio.h, string.h, or other C standard libraries.
- Basic Terminal Support: Text output using custom kprint and kprint_newline functions.
- Multiboot Compliant: The OS follows the multiboot specification for booting in an x86 environment.

# Installation and Compilation
## Prerequisites
- NASM: Assembler to compile the assembly code.
- GCC: Cross-compiler for building 32-bit binaries.
- QEMU: Emulator to test the OS in a virtualized environment.

### Compilation Steps
- Install the required tools:
```bash
sudo apt-get install nasm gcc qemu
```
- Clone this repository:
```bash
git clone https://github.com/Jahirrrr/kaze-os.git
cd kaze-os
```

- Compile the OS using the provided Makefile:

```bash
make
```
- Run the OS using QEMU:

```bash
make run
```

- Clean the build files:

```bash
make clean
```

# Usage
## Running in QEMU
After compiling, you can run the OS using the following command:

```bash
make run
```
The OS will boot into a simple console interface, where you can execute the shell commands.

# License
Copyright © 2024 Zahir Hadi Athallah. All Rights Reserved.

This software, including all its components and source code, is the exclusive property of Zahir Hadi Athallah. Redistribution, modification, or use of this software in any form, in whole or in part, is prohibited without explicit prior permission from the owner.

Terms:
1. Ownership: All intellectual property rights, including but not limited to code, design, and content, belong solely to Zahir Hadi Athallah.

2. Prohibition of Modification and Redistribution: You are not permitted to modify, distribute, or use any part of this software without prior written consent from Zahir Hadi Athallah. Any form of derivative work, whether for personal or commercial use, must first be approved.

3. Requesting Permission: For any modifications, usage beyond personal review, or redistribution, please contact the author at:
- Email: zahirrhadiathallah@gmail.com
- GitHub: Jahirrrr
- Other Contact Methods: @jahirishere.js [Instagram]

4. Liability Disclaimer: This software is provided "as is," without warranty of any kind. The owner is not liable for any damages arising from the use of this software.
