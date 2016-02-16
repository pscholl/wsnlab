#!/usr/bin/python

import sys, time, os
import numpy as np
from struct import unpack
import glob, subprocess, shutil
from matplotlib.dates import num2date
import paramiko
from gi.repository import Gtk
#import pps_plot.pps_plot as ppsplot
import pps_io.pps_import as ppsimport
import pps_dialogs.pps_conn as ppsconn

# print file transfer progress
def progress(curr, tot):
	perc = curr / tot * 100
	print("    ", curr, " of ", tot, " Bytes, ", int(perc), "%", sep="", end="\r", flush=True)



# look for the home directory of the system 
homedir = os.path.expanduser("~")

# create download dir if it doesn't exist
localpath = os.path.join(homedir,'pps_logs')
if not os.path.exists(localpath):
	os.makedirs(localpath)

# remote log location on edison
remotepath = "/home/root/pps_logs/"


# connect to edison
conn_dlg = ppsconn.pps_connect_dlg()
Gtk.main()

ssh = conn_dlg.ssh

if conn_dlg.connected:
	ftp = ssh.open_sftp()

	try:
		ftp.chdir(remotepath)
	except IOError:
		print(remotepath, "doesn't exist on the Platypus device.")
		print("Nothing to download, exiting.")
		ftp.close()
		ssh.close()
		quit()

	filelist = sorted(ftp.listdir())

	print("\nDownloading", len(filelist), "files from the Platypus device.\n")

	idx = 1
	for filename in filelist:
		print('{:02}'.format(idx), "| Getting", filename, ":", flush=True)
		ftp.get(remotepath + filename, os.path.join(localpath, filename), progress)
		print("")
		idx += 1

	print("\nDone.")
	ftp.close()

ssh.close()


