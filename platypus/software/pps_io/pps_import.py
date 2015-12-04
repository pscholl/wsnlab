import os
import numpy as np
from matplotlib.dates import date2num, seconds
from datetime import datetime
from struct import unpack


PAGESIZE = 7200 # size in bytes of one page of data (e.g. 600 samples)
HEADERSIZE = 20 # size in bytes of data header
SAMPLESIZE = 12 # size in bytes of one sample of data
SAMPLERATE = 25 # samplerate in Hz

## data descriptor for the platypus default data:
desc_pps = {	'names':   ('t', 'ax', 'ay', 'az', 'gx', 'gy', 'gz', 'l1', 'l2', 'temp', 'press', 'hum'), 
					'formats': ('f8', 'h', 'h', 'h', 'h', 'h', 'h', 'H', 'H', 'i', 'I', 'I') }


# print file transfer progress
def progress(curr, tot):
	perc = curr / tot * 100
	print("     ", curr, " of ", tot, " Samples, ", int(perc), "%", sep="", end="\r", flush=True)


## converts 4 bytes into a matplotlib timestamp
def pps_convtime(b1,b2,b3,b4):
	try:
		dt	= datetime(	2000+((b4 & 0xFC) >> 2),					# year
						((b4 & 0x03)<<2) + ((b3 & 0xC0)>>6),		# month
						(b3 & 0x3E)>>1,									# day
						((b3 & 0x01)<<4) + ((b2 & 0xF0)>>4),		# hour
						((b2 & 0x0F)<<2) + ((b1 & 0xC0)>>6),		# minute
						(b1 & 0x3F) ) 										# seconds
		return date2num(dt)
	except ValueError:
		return False


def pps_import_file(filename):
	fsize = os.path.getsize(filename)
	numheaders = int(fsize / (PAGESIZE + HEADERSIZE)) + 1
	rawsize = fsize - (numheaders * HEADERSIZE)
	samples = int(rawsize / SAMPLESIZE)

	data = np.recarray((samples,), dtype=desc_pps)
	f = open(filename, "rb")

	i = 0 # overall sample index
	j = 0 # page sample index
	headernext = True # header indicator
	while i < samples:
		if headernext:
			bs = f.read(HEADERSIZE)
			bs = unpack("%sB"%len(bs), bs)
			tme = pps_convtime(bs[0], bs[1], bs[2], bs[3])
			l1 = (bs[4]<<8)+(bs[5])
			l2 = (bs[6]<<8)+(bs[7])
			temp = (bs[8]<<24)+(bs[9]<<16)+(bs[10]<<8)+(bs[11])
			press = (bs[12]<<24)+(bs[13]<<16)+(bs[14]<<8)+(bs[15])
			hum = (bs[16]<<24)+(bs[17]<<16)+(bs[18]<<8)+(bs[19])
			headernext = False
			j = 0
		else:
			bs = f.read(SAMPLESIZE)
			bs = unpack("%sB"%len(bs), bs)
			data[i] = np.array((tme + seconds(j * (1 / SAMPLERATE)), (bs[0]<<8)+(bs[1]), (bs[2]<<8)+(bs[3]), (bs[4]<<8)+(bs[5]), (bs[6]<<8)+(bs[7]), (bs[8]<<8)+(bs[9]), (bs[10]<<8)+(bs[11]), l1, l2, temp, press, hum), dtype=desc_pps)
			i += 1
			j += 1
			if j == PAGESIZE / SAMPLESIZE:
				headernext = True
			progress(i, samples)

	f.close()
	if i > 0:
		return data
	else:
		return []


## store the data in an npz file
def pps_store(filename, data):
	try:
		np.savez(filename, data=data)
		return filename
	except:
		return ''

