#!/usr/bin/env bash

# Executes the test and compares the output to a known good output. Since line
# order may change and whitespace does not matter, first sort all lines of the
# output before passing it to diff and also ignore whitespace.

echo "Comparing '$1' output to '$2'"
$1 | sort | diff --ignore-space-change --ignore-blank-lines <(sort $2) -
