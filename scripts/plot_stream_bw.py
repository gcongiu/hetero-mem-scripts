#!/usr/local/bin/python
import matplotlib.pyplot as plt
import numpy as np
import sys  # for file I/O

labels = ['64K', '128K', '256K', '512K', '1M', '2M', '4M']

x = np.arange(len(labels))

# Main
if __name__ == "__main__":
    # get input csv file name
    csv = sys.argv[1]
    filename = csv.split(".")[0]

    # open csv file
    fd = open(csv, "r")

    # first row is: 'T=1,T=2,T=4,...,T=64\n'
    line = fd.readline().split(",")
    line = np.char.replace(line, '\n', '')
    confs = line[1:]     # memory configurations
    nconfs = len(confs)  # number of configurations

    # temp list
    tmp = []

    # get data values from file
    for line in fd:
        tmp.append(line.split(",")[1:])

    # close csv file
    fd.close()

    # remove end of line from values
    tmp = np.char.replace(tmp, '\n', '')

    # convert tmp list into a matrix
    values = np.array(tmp)

    # convert characters into floats
    values = values.astype(float)

    # transpose matrix
    values = values.transpose()

    # Now plot data
    fig, ax = plt.subplots()

    # print all curves
    for i in np.arange(len(confs)):
        ax.plot(x, values[i], '.-')

    # rotate ticks
    plt.xticks(x, labels)

    # set axis labels, grid and legend
    ax.set(xlabel='Elements/Thread (doubles)', ylabel='Bandwidth (MB/s)')
    ax.grid()
    ax.legend(confs)

    # adjust figure's margin
    fig.tight_layout()

    # add annotation to figure
#    ax.annotate('Switch to Rendezvous', xy=(16, 40000), xytext=(12.5, 10000),
#            arrowprops=dict(facecolor='black', shrink=0.05))

    # save figure and plot it
    fig.savefig("%s.pdf" % filename, format='pdf')
    plt.show()
