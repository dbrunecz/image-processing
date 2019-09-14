#/bin/bash

# on ubuntu 18.04
#  sudo apt-get install imagemagick-6-common

#rm -f *.bmp

LS=$(ls *.jpeg)
for i in $LS; do
	convert $i $(basename -s .jpeg $i).bmp
done

LS=$(ls *.jpg)
for i in $LS; do
	convert $i $(basename -s .jpg $i).bmp
done
