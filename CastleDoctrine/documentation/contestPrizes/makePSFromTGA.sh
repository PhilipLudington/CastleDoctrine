#!/bin/bash

filename="${1%.*}"

convert $1 $filename.ppm

./ppm2ps.pl $filename.ppm > $filename.ps