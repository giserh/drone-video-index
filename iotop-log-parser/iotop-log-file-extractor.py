#!/usr/bin/python

###############################################################################
# This is an extractor for the log file "iotop.log" output by the "iotop" command:
# "sudo iotop -botqqqk -d 10 > ./iotop.log"
#
# Input parameters:
#       argv[1]: the log input file name. [input]
#       argv[2]: the extracted output file name. [output]
#       argv[3]: the pid of the process to be monitored. [input]
#
# Each line in the output file is in form of 
#     "Timestamp	Reads(K/S)	Writes(K/S)	SWAPIN	IO"
#     They are separated with a tab.
#
# @author Ying Lu <ylu720@usc.edu>
# @date April 12th, 2017
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






