#!/bin/bash

for file in `ls ./data/*.txt | sort`; do
  time ./hexagonal-walk < $file > /tmp/${file##*/} && cat /tmp/${file##*/} | wc -l
done
