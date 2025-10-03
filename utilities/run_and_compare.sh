#!/usr/bin/env bash

# Executes the test and compares the output to a known good output. Since line
# order may change and whitespace does not matter, first sort all lines of the
# output before passing it to diff and also ignore whitespace.

case "$#" in
  2)
   echo "Running '$1' and comparing output to '$2'"
   eval $1 | sort | diff --ignore-space-change --ignore-blank-lines <(sort $2) -
   ;;
  3)
    cmd="$1 $2"
    echo "Running '$cmd' and comparing output to '$3'"
    eval $cmd | sort | diff --ignore-space-change --ignore-blank-lines <(sort $3) -
    ;;
  *)
    echo "Expect 2 or 3 arguments, got $#"
    exit 1
    ;;
esac
