# Project Template

## Overview

### EVT-core
For an overview of EVT please see the following links:

[EVT-core Documentation](https://evt-core.readthedocs.io/)

[EVT-core GitHub](https://github.com/RIT-EVT/EVT-core/)

### Template Project

This project-template serves as the skeleton template that is used for
all EVT board repositories.  It contains the following capabilities:

- Includes EVT-core as a submodule and compiled as a library for the board
- Set up to contain code pertinent to the board to be built
- Supports an arbitrary number of executable targets to be built and deployed onto a microcontroller
  - These contain targets to be run on a specific EVT custom PCB
  - Also contain utilities for validation and debugging
- Provides a framework for auto-generated and built documentation using Sphinx and hosted on 
`readthedocs.io`

## Steps to Set Up a New Project

1) Create a new repo based on the project-template
   1) From [project-template](https://github.com/RIT-EVT/project-template) click `Use this template`
   2) Choose a repo name and make sure it's public for auto-documentation to work
   3) If the owner is not set to RIT-EVT then you need to update the .gitmodules file URL
2) Clone the new repo
   1) `git clone <URL>`
3) Update the EVT submodule
   1) `git submodule update --init --recusive && git pull`
   2) The above command can be used at any point to update the submodule from remote
4) Import the project into Read the Docs, following the steps on 
[this wiki page](https://wiki.rit.edu/display/EVT/Documentation+and+Organization+Standards)
5) Update all instances of BOARD_NAME to match your project name
   1) `CMakeLists.txt:28`
   2) Directory `./targets/BIKE_NAME-BOARD_NAME`
   3) `targets/BIKE_NAME-BOARD_NAME/CMakeLists.txt:3`
   4) `src/BOARD_NAME.cpp`
   5) `include/BOARD_NAME.hpp`
   6) `docs/Doxyfile:35`
   7) `docs/source/index.rst:6`
   8) `docs/source/api/index.rst:4,12,15`
   9) `README.md:56`
6) Sample files are included in `./src` and `./include`. Once proper functionality has been 
confirmed, these files should be deleted.  There are placeholders to demonstrate the board library 
building functionality.
7) Everything in this README from this line up should be deleted, leaving only the content below:

# BIKE_NAME-BOARD_NAME

## Introduction

*One-paragraph summary of the board and its purpose on the bike*