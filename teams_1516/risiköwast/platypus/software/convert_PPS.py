#!/usr/bin/python

import os, glob
import pps_io.pps_import as ppsimport


# look for the home directory of the system 
homedir = os.path.expanduser("~")

# create download dir if it doesn't exist
inputpath = os.path.join(homedir,'pps_logs/raw')
outputpath = os.path.join(homedir,'pps_logs')
if not os.path.exists(inputpath):
  print("No raw files on local system, please download logfiles from Platypus first!")
  quit()

inputlist = sorted(glob.glob(inputpath + '/datalog*.bin'))
outputlist = sorted(glob.glob(outputpath + '/datalog*.npz'))

print("\nFound", len(inputlist), "raw files to convert.\n")

i = 0
j = 1
skips = 0
while i < len(inputlist):
  inputfile = inputlist[i]
  (root, ext) = os.path.splitext(inputfile)
  (head, tail) = os.path.split(root)
  outputfile = os.path.join(outputpath, tail) + ".npz"

  if outputfile not in outputlist:
    print()
    print('{:02}'.format(j), "| Reading from file:", inputfile)
    data = ppsimport.pps_import_file(inputfile)
    print('{:02}'.format(j), "| Writing to file:", outputfile)
    ppsimport.pps_store(outputfile, data)
    j += 1
  else:
    skips += 1

  i += 1

if skips > 0:
  print("\nSkipped", skips, "files, converted files already exist!")

print("\nDone.")
