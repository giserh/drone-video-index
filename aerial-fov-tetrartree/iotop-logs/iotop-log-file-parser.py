#!/usr/bin/python

###############################################################################
# This program is a parser for the quey result log files. 
#
# Input parameters:
#       argv[1]: the log file name. [input]
#       argv[2]: the parsed file name. [output]
#       argv[3]: x_axis_label_1 [input]
#       argv[4]: x_axis_label_2 [output]
#
#       X_axis_label_1 and X_axis_label_2 could be:
#       clustering_sample_ratio or 
#       sample_ratio_per_iter or 
#       total_cluster_num or 
#       max_iter or 
#       heat_diff_threshold or
#       topk_cell_dist_threshold
#
#       X_axis_label_2 could be 'null' if there is only one x-axis.
#       X_axis_label_1 or X_axis_label_2 are 'all', then use all the labels above.
#
# Each line in the output file is in form of 
#     "X_axis_label_1, X_axis_label_2	total_elapsed_time	accuracy"
#     total_elapsed_time and accuracy are separated with a tab.

# @author Ying Lu <ylu720@usc.edu>
# @date Sep 25th, 2016
###############################################################################


import numpy as np
import sys


def parse (inputLogFilename, outputFilename, pID):
#    lines = np.genfromtxt(inputLogFilename, dtype=None, unpack=True, delimiter='	')
    outFile = open(outputFilename, "w")
    linestring = open(inputLogFilename, 'r').read()
    lines = linestring.split('\n')
    for line in lines:
        line1 = line.split()
        if len(line1)>10 and line1[1]==pID:
            outFile.write("%s\t%s\t%s\t%s\t%s\n" \
                          % (line1[0], line1[4], line1[6], line1[8], line1[10]))
    return 


###
# main function
###
if (len(sys.argv)<2):
   print 'please type the input parameters correctly.'
else:
   intputLogFilename = sys.argv[1]
   outputFilename = sys.argv[2]
   pID = sys.argv[3]
   parse(intputLogFilename, outputFilename, pID)






