#!/bin/bash

cc cwl.c pc_main.c -lraylib -lm -opc_main -ggdb -fsanitize=address -Wall