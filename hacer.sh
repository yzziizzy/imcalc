#!/bin/bash

gcc -lutil build.c -o hacer -ggdb \
	&& ./hacer \
	&& build/imcalc
