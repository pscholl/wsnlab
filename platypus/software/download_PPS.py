#!/usr/bin/python

import os, socket, readline
import paramiko

# print file transfer progress
def progress(curr, tot):
	perc = curr / tot * 100
	print("     ", curr, " of ", tot, " Bytes, ", int(perc), "%", sep="", end="\r", flush=True)

# input with prefill (TODO: may only work on linux systems!)
def rlinput(prompt, prefill=''):
  readline.set_startup_hook(lambda: readline.insert_text(prefill))
  try:
     return input(prompt)
  finally:
     readline.set_startup_hook()


# look for the home directory of the system 
homedir = os.path.expanduser("~")

# create download dir if it doesn't exist
localpath = os.path.join(homedir,'pps_logs/raw')
if not os.path.exists(localpath):
	os.makedirs(localpath)

# remote log location on edison
remotepath = "/home/root/pps_logs/"


# connect to platypus
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
connected = False

ip = ""
user = "root"
passwd = "platypus"

print("Enter Platypus connection details:")
ip = rlinput("IP: ", ip)
user = rlinput("User: ", user)
passwd = rlinput("Password: ", passwd)

try:
  ssh.connect(ip, username=user, password=passwd)
  print('Connected')
  connected = True
except paramiko.BadHostKeyException:
  print('Server host key could not be verified.')
  quit()
except paramiko.AuthenticationException:
  print('Authentication failed.')
  quit()
except paramiko.SSHException as e:
  print('Error connecting or establishing SSH session:', e)
  quit()
except socket.error as e:
  print('Socket error occurred while connecting:', e)
  quit()
except socket.timeout as e:
  print('Socket timeout occurred while connecting.', e)
  quit()


if connected:
	ftp = ssh.open_sftp()

	try:
		ftp.chdir(remotepath)
	except IOError:
		print(remotepath, "doesn't exist on the Platypus device.")
		print("Nothing to download, exiting.")
		ftp.close()
		ssh.close()
		quit()

	remotelist = sorted(ftp.listdir())
	locallist = sorted(os.listdir(localpath))

	print("\nFound", len(remotelist), "files on the Platypus device.\n")

	idx = 1
	skips = 0
	for filename in remotelist:
		
		if filename in locallist:
			skips += 1
		else:
			print('{:02}'.format(idx), "| Getting", filename, ":", flush=True)
			ftp.get(remotepath + filename, os.path.join(localpath, filename), progress)
			print("")
			idx += 1

	if skips > 0:
		print("\nSkipped", skips, "files, already exist locally!")

	print("\nDone.")
	ftp.close()

ssh.close()

conv = input("Convert files? [y/n]: ")
if conv == "y" or conv == "Y":
	path = os.path.dirname(os.path.realpath(__file__))
	convert = os.path.join(path, "convert_PPS.py")
	os.system(convert)