#!/usr/bin/env bash

# Executes the test and compares the output to a known good output. Since line
# order may change and whitespace does not matter, first sort all lines of the
# output before passing it to diff and also ignore whitespace.

echo "Running '$1' with input '$2' and output '$3' and comparing to '$4'"
$1 $2 $3; cat $3 | sort | diff --ignore-space-change --ignore-blank-lines <(sort $4) -
