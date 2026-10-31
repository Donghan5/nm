# 42 Unix school project
This project is part of the 42 school curriculum and focuses on Unix programming.

The goal is implementing a nm command that displays the symbols of object files in a human-readable format.

## Project Structure
The project is organized into the following directories:
- `src/`: Contains the source code files for the project.
- `include/`: Contains the header files for the project.
- `tests/`: Contains test cases and scripts to validate the functionality of the nm command.
- Readme.md: This file provides an overview of the project and its structure.

## Project Architecture
```text
1. Take the input file as a command-line argument.
2. Open the file and read its contents.
3. Identify the file format (We are using ELF in this project) and determine the appropriate parsing method.
4. Parse the symbol table and collect relevant information (e.g., symbol names, addresses, types).
5. Sort the symbols based on the specified criteria (e.g., name, address).
6. Print the sorted symbols in a human-readable format.
```

## Getting Started
To get started with the project, follow these steps:
1. Clone the repository to your local machine.
2. Navigate to the project directory.
3. Compile the source code using the provided Makefile or build script.

## Bonus feature (-r and -p are implemented)
1. `-a` --> print debugger symbol which defaultly hind
2. `-g` --> print external symbol
3. `-u` --> print undefined symbol
4. `-r` --> reverse sorting
5. `-p` --> not sorting
