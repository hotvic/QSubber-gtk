#!/bin/env python3

import os, sys

def convert(fileName):
  with open(fileName, 'r') as f:
    f.readline()

    print('[')
    for line in f:
      lang = line.split('\t')
      print('{{ "display": "{}", "code": "{}" }},'.format(lang[2], lang[0]))
    print(']')

if __name__ == '__main__':
  if (len(sys.argv) <= 1):
    print('Missing file name')
    exit(-2)

  convert(sys.argv[1])