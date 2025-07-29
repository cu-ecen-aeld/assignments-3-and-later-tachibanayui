#!/bin/sh

# Shell script finder-app/finder.sh of Assignment 01. 
# https://www.coursera.org/learn/linux-system-programming-introduction-to-buildroot/supplement/bnixD/assignment-1-instructions
# Author: Nguyen Thu Giang

filesdir="$1"
searchstr="$2"

if [ $# -eq 2 ]; then
	if [ ! -d "$1" ]; then
		echo "Path does not exist"
		exit 1
	fi
else
	echo "Number of input arguments should be equal to 02"
	echo "the first argument is a path to a directory on the filesystem"
	echo "the second argument is a text string which will be searched within these files"
	exit 1
fi

number_of_files() {
	find $filesdir -type f | wc -l
}

number_of_matching_lines() {
	grep -R "$searchstr" "$filesdir" 2>/dev/null | wc -l
}

total_files=$(number_of_files)
matching_files=$(number_of_matching_lines)

echo "The number of files are $total_files and the number of matching lines are $matching_files "