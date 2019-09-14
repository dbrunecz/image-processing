#!/bin/bash

if [ $# -ne "1" ]; then 
	echo "specify image number [1-7]"
else
	make clean && make

	rm sample.bmp /tmp/negative.bmp /tmp/grayscale.bmp /tmp/filter.bmp

	killall eog

	cp images/img$1.bmp sample.bmp
	./img-proc sample.bmp

	eog sample.bmp &
	eog /tmp/negative.bmp &
	eog /tmp/grayscale.bmp &
	eog /tmp/filter.bmp &
fi
