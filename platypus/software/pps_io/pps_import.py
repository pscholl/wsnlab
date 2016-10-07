import os
import numpy as np
import csv
from matplotlib.dates import date2num, seconds
from datetime import datetime
from struct import unpack


PAGESIZE = 7200 # size in bytes of one page of data (e.g. 600 samples)
HEADERSIZE = 32 # size in bytes of data header
SAMPLESIZE = 12 # size in bytes of one sample of data
SAMPLERATE = 25 # samplerate in Hz

## data descriptor for the platypus default data:
desc_pps = {	'names':   ('t',												# timestamp
													'ax', 'ay', 'az',						# accelerometer x,y,z
													'gx', 'gy', 'gz',						# gyroscope x,y,z
													'mx', 'my', 'mz',						# magnetometer x,y,z
													'l1', 'l2',									# light vis,ir
													'temp', 'press', 'hum',			# BME
													'bat'), 										# battery percentage
					'formats': ('f8',
					            'h', 'h', 'h',
					            'h', 'h', 'h',
					            'h', 'h', 'h',
					            'H', 'H',
					            'i', 'I', 'I',
					            'B') }
## https://docs.python.org/2/library/array.html


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


# TODO: This needs some commenting!!!
def pps_import_file(filename, rate):
	SAMPLERATE = rate
	fsize = os.path.getsize(filename) # size of the file in bytes
	numheaders = int(fsize / (PAGESIZE + HEADERSIZE)) + 1 # number of headers in the data (+1 for first header that is always present)
	rawsize = fsize - (numheaders * HEADERSIZE) # size of sample data without headers in bytes
	samples = int(rawsize / SAMPLESIZE) # number of samples in the sample data

	data = np.recarray((samples,), dtype=desc_pps)
	f = open(filename, "rb")

	i = 0 # overall sample index
	j = 0 # page sample index
	k = 0 # header index
	headernext = True # header indicator
	while i < samples:
		if headernext:
			bs = f.read(HEADERSIZE)
			bs = unpack("%sB"%len(bs), bs)
			tme = pps_convtime(bs[0], bs[1], bs[2], bs[3])
			mx = (bs[4]<<8)+(bs[5])
			my = (bs[6]<<8)+(bs[7])
			mz = (bs[8]<<8)+(bs[9])
			l1 = (bs[10]<<8)+(bs[11])
			l2 = (bs[12]<<8)+(bs[13])
			temp = (bs[14]<<24)+(bs[15]<<16)+(bs[16]<<8)+(bs[17])
			press = (bs[18]<<24)+(bs[19]<<16)+(bs[20]<<8)+(bs[21])
			hum = (bs[22]<<24)+(bs[23]<<16)+(bs[24]<<8)+(bs[25])
			bat = bs[26]
			headernext = False
			k += 1
		elif k < numheaders:
			bs = f.read(PAGESIZE)
			bs = unpack("%sB"%len(bs), bs)
			for x in range(int(PAGESIZE/SAMPLESIZE)):
				data[i] = np.array((tme + seconds(x * (1 / SAMPLERATE)), (bs[0+(x*SAMPLESIZE)]<<8)+(bs[1+(x*SAMPLESIZE)]), (bs[2+(x*SAMPLESIZE)]<<8)+(bs[3+(x*SAMPLESIZE)]), (bs[4+(x*SAMPLESIZE)]<<8)+(bs[5+(x*SAMPLESIZE)]), (bs[6+(x*SAMPLESIZE)]<<8)+(bs[7+(x*SAMPLESIZE)]), (bs[8+(x*SAMPLESIZE)]<<8)+(bs[9+(x*SAMPLESIZE)]), (bs[10+(x*SAMPLESIZE)]<<8)+(bs[11+(x*SAMPLESIZE)]), mx, my, mz, l1, l2, temp, press, hum, bat), dtype=desc_pps)
				i += 1
			headernext = True
		else:
			bs = f.read(SAMPLESIZE)
			bs = unpack("%sB"%len(bs), bs)
			data[i] = np.array((tme + seconds(j * (1 / SAMPLERATE)), (bs[0]<<8)+(bs[1]), (bs[2]<<8)+(bs[3]), (bs[4]<<8)+(bs[5]), (bs[6]<<8)+(bs[7]), (bs[8]<<8)+(bs[9]), (bs[10]<<8)+(bs[11]), mx, my, mz, l1, l2, temp, press, hum, bat), dtype=desc_pps)
			i += 1
			j += 1
		progress(i, samples)

	f.close()
	if i > 0:
		return data
	else:
		return []


## store the data in an npz file
def pps_store(filename, data, format):
	if format == "npz":
		try:
			np.savez(filename, data=data)
			return filename
		except:
			return ''
	elif format == "csv":
		try:
			with open(filename, 'w', newline='') as f:
				writer = csv.writer(f)
				i = 1
				writer.writerow(desc_pps.get('names')) # header
				for d in data:
					writer.writerow(d)
					progress(i, data.size)
					i += 1
			print("                                         ", sep="", end="\r", flush=True)
			return filename
		except:
			return ''
	return ''
