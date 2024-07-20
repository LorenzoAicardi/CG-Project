#!/bin/bash

for file in *; do
	if [[ -f $file ]]; then
		if [[ $file == *.frag ]]; then
			glslc $file -o "${file:0:-5}Frag.spv" 
		elif [[ $file == *.vert ]]; then
			glslc $file -o "${file:0:-5}Vert.spv"
		fi
	fi
done
