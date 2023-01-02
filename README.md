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
   2) Set the owner of the repository to RIT-EVT
   3) Name the repo with the new board's acronym
   4) Set the privacy to Public
   5) Don't include all branches
2) Clone the new repo and create a setup branch
   1) `git clone <URL>`
   2) `git checkout -b feature/<github-username>/inital-setup`
3) Set the project template up as an upstream repository
   1) `git remote add upstream https://github.com/RIT-EVT/project-template`
   2) `git remote set-url --push upstream no-push`
      1) Confirm that this worked by running `git remote -v`
      2) This should produce output similar to this:
            ```
            origin  https://github.com/RIT-EVT/ABC.git (fetch)
            origin  https://github.com/RIT-EVT/ABC.git (push)
            upstream        https://github.com/RIT-EVT/project-template (fetch)
            upstream        no-push (push)
            ```
   3) `git fetch upstream`
   4) `git merge upstream/main --allow-unrelated-histories`

4) Update the EVT submodule
   1) `git submodule update --init --recursive && git pull`
   2) `cd ./libs/EVT-core`
   3) `git merge origin/main`
   4) `cd ../..`
5) Import the project into Read the Docs, following the steps on 
[the wiki](https://wiki.rit.edu/display/EVT/Documentation+and+Organization+Standards)
6) Update all instances of BOARD_NAME to match your project name
   1) `CMakeLists.txt:28`
   2) Directory `./targets/BIKE_NAME-BOARD_NAME`
   3) `targets/BIKE_NAME-BOARD_NAME/CMakeLists.txt:3`
   4) `src/BOARD_NAME.cpp`
   5) `include/BOARD_NAME.hpp`
   6) `docs/Doxyfile:35`
   7) `docs/source/index.rst:6`
   8) `docs/source/api/index.rst:4,12,15`
   9) `README.md:56`
7) Sample files are included in `./src` and `./include`. Once proper functionality has been 
confirmed, these files should be deleted.  There are placeholders to demonstrate the board library 
building functionality.
8) Everything in this README from this final step up should be deleted, leaving only the content 
below. When finished, all the changes should be committed and pushed to the setup branch, and a PR
should be created to merge into main.

# BIKE_NAME-BOARD_NAME

## Introduction

*One-paragraph summary of the board and its purpose on the bike*