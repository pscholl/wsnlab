#!/usr/bin/python3

import sys, time, os
import numpy as np

import warnings
warnings.filterwarnings(action='ignore')

path = os.path.dirname(os.path.realpath(__file__))
path = os.path.join(path, "pps_plot")
sys.path.append(path)
#print sys.path
import pps_plot3 as pplt

import matplotlib.dates  as mld
import pygtk, gtk

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


try:
	filename = sys.argv[1]
	ext = filename[-3:]
	ext = ext.lower()
except:
	print('usage: viz_PPS.py <data.npy|data.npz>')
	quit()
	
if ext=='npy': # simple loading of numpy file:
	dta = np.load(filename)
elif ext=='npz':
	out = np.load(filename)
	dta = out['data']
else:
	print('file has wrong extension')
	quit()
	
dta = dta.view(desc_pps, np.recarray)

#print dta

## actual plotting here:
fig = pplt.PPS_load_plot(10,8,80)
#fig.plot(1, 9, dta.t, (np.array((dta.ax,dta.ay,dta.az)).T) / 8192.0 * 9.807,'3D acceleration')
#fig.plot(2, 9, dta.t, (np.array((dta.gx,dta.gy,dta.gz)).T) / 131.0,'3D gyroscope')
#fig.plot(3, 9, dta.t, (np.array((dta.mx,dta.my,dta.mz)).T),'3D magnetometer')
#fig.plot(4, 9, dta.t, np.array((dta.l1)).T, 'ambient light')
#fig.plot(5, 9, dta.t, np.array((dta.l2)).T, 'infrared light')
#fig.plot(6, 9, dta.t, (np.array((dta.temp)).T) / 100.0, 'temperature')
#fig.plot(7, 9, dta.t, (np.array((dta.press)).T) / 25600.0, 'pressure')
#fig.plot(8, 9, dta.t, (np.array((dta.hum)).T) / 1024.0, 'humidity')
#fig.plot(9, 9, dta.t, (np.array((dta.bat)).T), 'battery %')
#fig.show()

fig.plot(dta)

while(True):
	fig.update_plot(dta)
