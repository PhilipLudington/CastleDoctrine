#!/bin/bash

for f in *.tga; do 
	echo "Processing $f";
	./makePSFromTGA.sh $f;
	
done