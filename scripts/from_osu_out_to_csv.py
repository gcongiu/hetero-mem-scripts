#!/usr/bin/python

import re
import sys

test_regex = re.compile('([sS]/[rR]/[fF]/[cC]/[cCbB]{2}|[oO]/[tT])')
data_regex = re.compile('([0-9]+)\s+([0-9]+\.[0-9]+)')
proc_regex = re.compile('Number of procs = ([0-9]+)')

if __name__ == "__main__":

    # get number of message sizes tested
    num_sizes = int(sys.argv[1])

    # get file names from cmd line
    iname = sys.argv[2]
    outname = sys.argv[3]

    # open files
    fdin = open(iname, "r")
    fdout = open(outname, "w")

    # define arrays
    size = ["" for i in range(num_sizes)]
    data = ["" for i in range(num_sizes)]
    count = 0

    # process file
    for line in fdin:
        # get memory configuration
        res = re.match(test_regex, line)
        if res is not None:
            if count > 0:
                for i in range(num_sizes):
                    fdout.write(size[i]+data[i]+'\n')
                    data[i] = ""
                count = 0
            label = res.group(0)
        else:
            # get experiment data
            res = re.match(data_regex, line)
            if res is not None:
                if size[count % num_sizes] == "":
                    size[count % num_sizes] = res.group(1)
                data[count % num_sizes] += ','+res.group(2)
                count += 1
            else:
                res = re.match(proc_regex, line)
                if res is not None:
                    fdout.write(label+' (procs = '+res.group(1)+')'+'\n')

    # print remaining data to out file:
    for i in range(num_sizes):
        fdout.write(size[i]+data[i]+'\n')

    # close files
    fdin.close()
    fdout.close()
