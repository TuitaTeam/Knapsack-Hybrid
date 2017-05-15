#!/usr/bin/python

# Copyright 2017 Meritxell Jordana Gavieiro
# Copyright 2017 Albert Merino Pulido
# Copyright 2017 Marc Sanchez Fauste

import sys

if len(sys.argv) != 3:
    print "Usage:", sys.argv[0], "<results_speedups.csv> <threads.csv>"
    exit(1)

f = open(sys.argv[1], 'r')

results = {}

tf = open(sys.argv[2], 'r')
threads = tf.read().split()
tf.close()

for line in f:
   l = line.split()
   if len(l) == 7:
       results[(l[0], l[1], l[5])] = l[6]

f.close()
f = open(sys.argv[1], 'r')

sys.stdout.write("Width Items Speedup1Thread")
for thread in threads:
    sys.stdout.write(' Speedup' + str(thread) + 'Threads')
sys.stdout.write('\n')
for line in f:
   line = line.strip('\n')
   l = line.split()
   try:
       if len(l) == 7:
           if l[5] == '1':
               sys.stdout.write(str(l[0]) + ' ' + str(l[1]) + ' ' + str(l[6]))
               for thread in threads:
                   sys.stdout.write(' ' + str(results[(l[0], l[1], thread)]))
               sys.stdout.write('\n')
   except:
       pass

f.close()
