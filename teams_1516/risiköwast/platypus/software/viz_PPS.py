#!/usr/bin/env python2.7

import sys, time, os
import numpy as np

path = os.path.dirname(os.path.realpath(__file__))
path = os.path.join(path, "pps_plot")
sys.path.append(path)
print sys.path
import pps_plot as pplt

import matplotlib.dates  as mld
import pygtk, gtk

## data descriptor for the platypus default data:
desc_pps = {	'names':   ('t', 'ax', 'ay', 'az', 'gx', 'gy', 'gz', 'l1', 'l2', 'temp', 'press', 'hum'), 
					'formats': ('f8', 'h', 'h', 'h', 'h', 'h', 'h', 'H', 'H', 'i', 'I', 'I') }


try:
	filename = sys.argv[1]
	ext = filename[-3:]
	ext = ext.lower()
except:
	print 'usage: viz_PPS.py <data.npy|data.npz>'
	quit()
	
if ext=='npy': # simple loading of numpy file:
	dta = np.load(filename)
elif ext=='npz':
	out = np.load(filename)
	dta = out['data']
else:
	print 'file has wrong extension'
	quit()
	
dta = dta.view(desc_pps, np.recarray)

#print dta

## actual plotting here:
fig = pplt.PPS_raw_plot(10,8,80)
fig.plot(1, 7, dta.t, (np.array((dta.ax,dta.ay,dta.az)).T) / 8192.0 * 9.807,'3D acceleration')
fig.plot(2, 7, dta.t, (np.array((dta.gx,dta.gy,dta.gz)).T) / 131.0,'3D gyroscope')
fig.plot(3, 7, dta.t, np.array((dta.l1)).T, 'ambient light')
fig.plot(4, 7, dta.t, np.array((dta.l2)).T, 'infrared light')
fig.plot(5, 7, dta.t, (np.array((dta.temp)).T) / 100.0, 'temperature')
fig.plot(6, 7, dta.t, (np.array((dta.press)).T) / 25600.0, 'pressure')
fig.plot(7, 7, dta.t, (np.array((dta.hum)).T) / 1024.0, 'humidity')
fig.show()

