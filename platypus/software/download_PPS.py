#!/usr/bin/python3

import os, socket, readline
import argparse
import paramiko

parser = argparse.ArgumentParser(description="download raw binary data from a platypus device", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
parser.add_argument("-c", "--convert", help="convert files after downloading", action="store_true")

group_io = parser.add_argument_group('I/O arguments')
group_io.add_argument("-r", "--remote-dir", help="specify remote directory (on platypus), absolute", metavar="DIR", default="~/pps_logs")
group_io.add_argument("-l", "--local-dir", help="specify local directory, absolute", metavar="DIR", default="~/pps_logs/raw")

group_conn = parser.add_argument_group('connection arguments')
group_conn.add_argument("--ip", help="remote ip", default="")
group_conn.add_argument("--port", help="remote port", default="22")
group_conn.add_argument("--user", help="remote user", default="root")
group_conn.add_argument("--password", help="remote password", default="platypus")

args = parser.parse_args()

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

# parse local path
localpath = os.path.expanduser(args.local_dir)
if not os.path.isdir(localpath):
  print("Output directory", localpath, "does not exist, creating...")
  os.makedirs(localpath)

# remote log location on edison
remotepath = args.remote_dir.replace("~","/home/root")

# connect to platypus
ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
connected = False

# get connection config
ip = args.ip
port = args.port
user = args.user
passwd = args.password

if args.ip is "":
	#print("Enter Platypus connection details:")
	ip = rlinput("IP/Name: ", ip)
	#port = rlinput("Port: ", port)
	#user = rlinput("User: ", user)
	#passwd = rlinput("Password: ", passwd)

if args.verbose:
	print("remote:", remotepath)
	print("local:", localpath)
	print("connection:")
	print("  ip:", ip)
	print("  port:", port)
	print("  user:", user)
	print("  password:", passwd)

print()

# try to connect to platypus
try:
  ssh.connect(ip, int(port), username=user, password=passwd)
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
	ftp = ssh.open_sftp() # if shh connected, open ftp connection

	try:
		ftp.chdir(remotepath) # change ftp to remote dir
	except IOError:
		print(remotepath, "doesn't exist on the Platypus device.")
		print("Nothing to download, exiting.")
		ftp.close()
		ssh.close()
		quit()

	# get file lists
	remotelist = sorted(ftp.listdir())
	locallist = sorted(os.listdir(localpath))

	print("\nFound", len(remotelist), "files on the Platypus device.\n")

	idx = 1
	skips = 0
	for filename in remotelist:

		if filename in locallist: # skip, already exists
			skips += 1
		else:
			# copy file from remote to local path
			print('{:02}'.format(idx), "| Getting", filename, ":", flush=True)
			ftp.get(remotepath + "/" + filename, os.path.join(localpath, filename), progress)
			print("")
			idx += 1

	if skips > 0:
		print("\nSkipped", skips, "files, already exist locally!")

	print("\nDone.")
	ftp.close()

ssh.close()

# start convert if needed
if args.convert:
	path = os.path.dirname(os.path.realpath(__file__))
	convert = os.path.join(path, "convert_PPS.py")
	os.system(convert + (" -v" if args.verbose else "") + " --input-dir " + localpath)
