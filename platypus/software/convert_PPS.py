#!/usr/bin/python3

import os, glob
import argparse
import pps_io.pps_import as ppsimport

parser = argparse.ArgumentParser(description="convert raw binary data from platypus to a given data fromat", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
parser.add_argument("-f", "--format", help="format of the output", choices=["npz","csv"], default="npz")
parser.add_argument("-r", "--rate", help="sample rate of raw data [Hz]", type=int, default="25")

group_io = parser.add_argument_group('I/O arguments')
group_io.add_argument("-id", "--input-dir", help="specify input directory, absolute", metavar="DIR", default="~/pps_logs/raw")
group_io.add_argument("-od", "--output-dir", help="specify output directory, absolute", metavar="DIR", default="~/pps_logs")
group_io.add_argument("-if", "--input-file", help="specify single input file, absolute", metavar="FILE")
group_io.add_argument("-of", "--output-file", help="specify single output file, absolute", metavar="FILE")

args = parser.parse_args()

# do some basic path/file checks

if args.input_file is not None:
  args.input_dir = args.input_file
  inputisfile = True
else:
  inputisfile = False

if args.output_file is not None:
  args.output_dir = args.output_file
  outputisfile = True
else:
  outputisfile = False

if not inputisfile and outputisfile:
  print("specified input is a directory but output is a file!")
  quit()

inputpath = os.path.expanduser(args.input_dir)
inputfile = None
inputext = None
outputpath = os.path.expanduser(args.output_dir)
outputfile = None
outputext = None

if inputisfile and not os.path.isfile(inputpath):
  print("Input file", inputpath, "does not exist!")
  quit()
if not inputisfile and not os.path.isdir(inputpath):
  print("Input directory", inputpath, "does not exist!")
  quit()

if inputisfile:
  inputpath, inputext = os.path.splitext(inputpath)
  inputpath, inputfile = os.path.split(inputpath)
  if inputext != ".bin":
    print("Input must be a .bin file!")
    quit()

if outputisfile:
  outputpath, outputext = os.path.splitext(outputpath)
  outputpath, outputfile = os.path.split(outputpath)
  if "." + args.format != outputext:
    print("Warning: specified output file extension", outputext, "does not correspond with given format", args.format)

if not os.path.isdir(outputpath):
  print("Output directory", outputpath, "does not exist, creating...")
  os.makedirs(outputpath)


# get the input and output file lists
if not inputisfile:
  inputlist = sorted(glob.glob(inputpath + '/*.bin'))
else:
  inputlist = sorted(glob.glob(inputpath + '/' + inputfile + inputext))

if not outputisfile:
  outputlist = sorted(glob.glob(outputpath + '/*.' + args.format))
else:
  outputlist = sorted(glob.glob(outputpath + '/' + outputfile + outputext))

if args.verbose:
  print("format:", args.format)
  print("input:")
  print("  isfile:", inputisfile)
  print("  path:", inputpath)
  print("  file:", inputfile)
  print("  extension:", inputext)
  print("  glob:", inputlist)
  print("output:")
  print("  isfile:", outputisfile)
  print("  path:", outputpath)
  print("  file:", outputfile)
  print("  extension:", outputext)
  print("  glob:", outputlist)

print("\nFound", len(inputlist), "raw files to convert.\n")

i = 0
j = 1
skips = 0
while i < len(inputlist):
  c_inputfile = inputlist[i]
  (root, ext) = os.path.splitext(c_inputfile)
  (head, tail) = os.path.split(root)
  if not outputisfile:
    c_outputfile = os.path.join(outputpath, tail) + "." + args.format
  else:
    c_outputfile = os.path.join(outputpath, outputfile) + outputext

  if c_outputfile not in outputlist:
    print()
    print('{:02}'.format(j), "| Reading from file:", c_inputfile)
    data = ppsimport.pps_import_file(c_inputfile, args.rate)
    print('{:02}'.format(j), "| Writing to file:", c_outputfile)
    ppsimport.pps_store(c_outputfile, data, args.format)
    j += 1
  else:
    skips += 1

  i += 1

if skips > 0:
  print("\nSkipped", skips, "files, converted files already exist!")

print("\nDone.")
