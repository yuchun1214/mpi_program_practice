#!/bin/bash

if [ $# == 0 ]; then
	echo "Please give the file name."
	exit 1
fi

for i in "$@"
do
	scp -r $i E64061151@140.116.154.66:~/
done
