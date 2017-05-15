#!/usr/bin/python

# Copyright 2017 Meritxell Jordana Gavieiro
# Copyright 2017 Albert Merino Pulido
# Copyright 2017 Marc Sanchez Fauste

import pylab as pl
import sys

times = {}

def readData(resultsFile, threadsFile):
    f = open(resultsFile, 'r')
    for line in f:
        try:
            l = line.split()
            t = []
            times[(l[0], l[1])] = [float(x) for x in l[2:]]
        except:
            pass
    f.close()
    tf = open(threadsFile, 'r')
    global threads
    threads = [1] + [int(x) for x in tf.read().split()]
    tf.close()

def plot(filename):
    global threads
    pl.figure(figsize=(15,10), dpi=80, facecolor='white', edgecolor='k')
    for key, value in times.items():
        pl.plot(threads, value, linewidth=2, label='Width: ' + key[0] + ' Items: ' + key[1])
    pl.legend()
    pl.xlabel('Number of cores')
    pl.ylabel('Time (s)')
    pl.xticks(threads)
    pl.grid()
    pl.savefig(filename)
    pl.show()

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print "Usage:", sys.argv[0], "<data_file.csv> <threads.csv> <plot_file.svg>"
        exit(1)
    readData(sys.argv[1], sys.argv[2])
    plot(sys.argv[3])
