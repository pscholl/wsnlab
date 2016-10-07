#!/usr/bin/env python2.7

import sys, time, os
import numpy as np

path = os.path.dirname(os.path.realpath(__file__))
path = os.path.join(path, "pps_plot")
sys.path.append(path)
import pps_plot as pplt

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

## actual plotting here:
fig = pplt.PPS_raw_plot(10,8,80)
# IMU:
ax=fig.plot(1, 6, dta.t, (np.array((dta.ax,dta.ay,dta.az)).T) / 2048.0 * 9.807,'3D acceleration')
legend=ax.legend(["x","y","z"],loc='upper right',prop={'size':7})
legend.get_frame().set_alpha(.77)
ax=fig.plot(2, 6, dta.t, (np.array((dta.gx,dta.gy,dta.gz)).T) / 131.0,'3D gyroscope')
legend=ax.legend(["x","y","z"],loc='upper right',prop={'size':7})
legend.get_frame().set_alpha(.77)
ax=fig.plot(3, 6, dta.t, (np.array((dta.mx,dta.my,dta.mz)).T),'3D magnetometer')
legend=ax.legend(["x","y","z"],loc='upper right',prop={'size':7})
legend.get_frame().set_alpha(.77)
# light:
ax=fig.plot(4, 6)
ax.fill_between(dta.t,0, np.array(dta.l1).T, facecolor='yellow', alpha=0.5)
ax.fill_between(dta.t,0, np.array(dta.l2).T, facecolor='red', alpha=0.5)
legend=ax.legend(["ambient","IR"],loc='upper right',prop={'size':7})
legend.get_frame().set_alpha(.77)
ax.set_ylabel('ambient/IR light')
# env:
ax = fig.plot(5, 6)
p1,=ax.plot_date(dta.t, (np.array((dta.temp)).T) / 100.0, 'b-', lw=0.5)
p2,=ax.plot_date(dta.t, (np.array((dta.hum)).T) / 1024.0, 'g-', lw=0.5)
ax2 = ax.twinx()
p3,=ax2.plot_date(dta.t, (np.array((dta.press)).T) / 25600.0, 'r-', lw=0.5 )
ax2.set_xticklabels([]); ax2.set_yticklabels([]); 
ax.set_ylabel('environment')
legend=ax2.legend([p1,p2,p3],["temp. (C)","hum. (pct)","pres. (Pa)"],loc='upper right',prop={'size':7})
legend.get_frame().set_alpha(.77)
# battery:
ax=fig.plot(6, 6)
ax.fill_between(dta.t,0, (np.array((dta.bat)).T), facecolor='grey')
ax.set_ylabel('battery %')
fig.show()
