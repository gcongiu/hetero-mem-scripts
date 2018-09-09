#!/usr/local/bin/python

import re
import sys
import numpy as np

# memory placement configurations
configuration = \
    re.compile("([sS]/[rR]/[fF]/[cC]/[cCbB]{2}|[oO]/[tT]) \(procs \= ([0-9]+)\)")

if __name__ == '__main__':
    # read file name from command line
    csv = sys.argv[1]
    filename = csv.split('.')[0]

    # open csv file
    fd = open(csv, 'r')

    # initial number of procs
    procs = 0

    # list of sizes for tests
    confs = []
    sizes = []
    avg = []
    data = {}

    # read lines and compute avg and stdev
    for line in fd:
        # get mem conf and procs
        res = re.match(configuration, line)
        if res is not None:
            if res.group(1) not in confs:     # configuration not yet discovered
                confs.append(res.group(1))    # add it to found configurations
            if procs == 0:                    # initially procs = 0
                procs = int(res.group(2))     # initialize number of procs
            elif procs != int(res.group(2)):  # new num of procs, wr prev procs
                tmp_fd = open(filename+"_%s_procs.csv" % str(procs), 'w')
                tmp_fd.write('procs = %s,' % procs+','.join(confs)+'\n')
                for i in np.arange(len(sizes)):
                    tmp_fd.write(sizes[i])    # write size
                    tmp_str = data[sizes[i]]  # get data for size
                    for elem in tmp_str:      # write each data value to file
                        tmp_fd.write(','+str(elem))
                    tmp_fd.write('\n')        # size data with newline
                sizes = []                    # reset sizes for next procs num
                avg = []                      # reset average
                data = {}                     # reset data map
                tmp_fd.close()
                procs = int(res.group(2))     # update number of procs
        else:                                 # get data and compute avg
            size = line.split(',')[0]         # get size from first column
            if size not in sizes:
                sizes.append(size)            # update size list
                data[size] = []               # add size to data map
            tmp_data = line.split(',')[1:]    # get data for size
            tmp_data = np.array(tmp_data)     # convert list into array
            tmp_data = tmp_data.astype(float) # convert array elem into float
            avg = np.average(tmp_data)        # compute average for size
            data[size].append(avg)            # add average to size in map

    # finally write last number of processes configurations
    tmp_fd = open(filename+"_%s_procs.csv" % str(procs), 'w')
    tmp_fd.write('procs = %s,' % procs+','.join(confs)+'\n')
    for i in np.arange(len(sizes)):
        tmp_fd.write(sizes[i])
        tmp_str = data[sizes[i]]
        for elem in tmp_str:
            tmp_fd.write(','+str(elem))
        tmp_fd.write('\n')
    tmp_fd.close()

    # close csv file
    fd.close()
