#!/bin/bash

cc cwl.c pc_main.c -lraylib -opc_main -ggdb -fsanitize=address -Wall