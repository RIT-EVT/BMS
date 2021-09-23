# Project Template

## Overview

### EVT-core
For an overview of EVT please see the following links:

[EVT-core Documentation](https://evt-core.readthedocs.io/)

[EVT-core Github](https://github.com/RIT-EVT/EVT-core/)

### Template Project

This project-template serves as the skeleton template that is used for
all EVT board repositories.  It contains the following capabilities:

- EVT-core is included as a submodule and compiled as a library for
the board
- Template is set up for a library containing code pertinent to the board 
to be built.
- The template supports an arbitrary number of executable targets to be built
and deployed onto a microcontroller
  - These contain targets to be run on a specific EVT custom boards
  - Also contain utilities for debugging and debugging
- Framework for auto-generated and built documentation using Sphinx and
hosted on `readthedocs.io`

## Steps to Setup a New Project

1) Create a new repo based on the project-template
   1) From [project-template](https://github.com/RIT-EVT/project-template) click `Use this template`
   2) Choose a repo name and make sure it's public for auto-documentation to work
   3) If the owner is not set to RIT-EVT then you need to update the .gitmodules file URL
2) Clone the new repo
   1) `git clone <URL>`
3) Update the EVT submodule
   1) `git submodule update --init --recusive && git pull`
   2) The above command can be used at any point to update the submodule from remote
4) Import the project into Read the Docs
   1) Consult with current firmware team lead for help setting up hosting.
5) Update all instances of BOARD_NAME to match your project name
   1) `CMakeLists.txt:15,21`
   2) Directory `./include/BOARD_NAME`
   3) Directory `./source/BOARD_NAME`
   4) Directory `./targets/BOARD_NAME`; also update `targets/CMakeLists.txt`
   5) `docs/Doxyfile:35`
   6) `docs/source/index.rst:6`
   7) `docs/source/api/index.rst`
6) Sample files are included in `./src` and `./include`.  These files can likely be removed and 
updates made accordingly.  There are placeholders to demonstrate the board library building
functionality.

