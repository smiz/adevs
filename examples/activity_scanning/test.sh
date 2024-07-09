#!/usr/bin/env bash

./test1 | diff test1.ok -
./test2 | diff test2.ok -
