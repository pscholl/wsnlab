import matplotlib, numpy
matplotlib.use('GTK3Agg')
from matplotlib.pyplot import *
from matplotlib.pylab import *
import matplotlib.dates as mld


## plotting routine for vizualizing stuff while loading data from PPS:
class PPS_load_plot:
	def __init__(self, fig_x=10, fig_y=6, fdpi=80):
		self.fig = 0
		self.bw_lookup = [0.1, 5, 10, 25, 50, 100, 200, 400, 800, 1500]
		self.md_lookup = ['controller', 'sensor']			
		self.pw_lookup = ['normal', 'low-power', 'auto-sleep', 'low/auto']
		self.ax = None
		self.first_plot = 1
		try: # disable toolbar:
			rcParams['toolbar'] = 'None';		
			self.fig = figure(	num=None, figsize=(fig_x, fig_y), 
										dpi=fdpi, facecolor='w', edgecolor='k' )
			self.fig.show()
		except ValueError:
			return 0
	def fix_margins(self):
			subplots_adjust(	left  = 0.03, right = 0.98, # left, right, 
									bottom = 0.1,top = 1, # bottom and top
									wspace = 0.1, # width space betw. subplots
									hspace = 0.1  # height space btw. subplots
								)
	def plot(self, dta, fn='',cnf=''):
		## plot data and clean up the axes: ##############################
		self.fix_margins()
		self.ax = self.fig.add_subplot(2,1,2, axisbg='#FFFFFF')
		self.linesx, = self.ax.plot_date(dta.t, dta.ax, '-r', lw=0.5)
		self.linesy, = self.ax.plot_date(dta.t, dta.ay, '-g', lw=0.5)
		self.linesz, = self.ax.plot_date(dta.t, dta.az, '-b', lw=0.5)
		self.axe = self.fig.add_subplot(4,1,2, axisbg='#777777')
		self.ambfill = self.axe.fill_between(dta.t, dta.l1>>8, 
			facecolor='yellow', lw=0.1, alpha=.6, label='ambient light')
		self.ax.grid(color='k', linestyle=':', linewidth=0.5)
		self.ax.xaxis.set_major_formatter(mld.DateFormatter('%H'))
		setp(self.ax.get_xticklabels(), visible=True, fontsize=10)
		setp(self.ax.get_yticklabels(), visible=False)
		self.ax.axes.set_ylim(0, 256)
		self.ax.axes.set_xlim(int(dta.t[-1]), int(dta.t[-1])+1)
		self.axe.grid(color='w', linestyle=':', linewidth=0.5)
		self.axe.axes.set_xlim(int(dta.t[-1]), int(dta.t[-1])+1)
		setp(self.axe.get_xticklabels(), visible=False)
		self.axe.axes.set_ylim(0, 128)
		setp(self.axe.get_yticklabels(), visible=False)
		self.axe.xaxis.axis_date()
		self.ax.legend([self.linesx, self.linesy, self.linesz],
			["x acceleration", "y acceleration", "z acceleration"], 
			loc=2, prop={'size':10})
		self.rect = Rectangle((0,1),1,1,fc='y')
		self.axe.legend([self.rect], ['ambient light'],
			loc=2, prop={'size':10})
		## plot infos: ###################################################
		self.fig.text( 0.04, 0.96, 'Sensor unit settings', 
			ha='left', va='top', family='monospace',fontsize=11,
			bbox=dict(boxstyle='round',facecolor='grey',alpha=.4))			
		if len(cnf)>72:
			self.fig.text( 0.04, 0.927,
			'HedgeHog_ID: ' + cnf[:4] + '\nfirmware:    ' + cnf[35:42]
			+' \nlogging end: 20' +str(ord(cnf[71])) +'-'
			+str(ord(cnf[74])).zfill(2) +'-'+ str(ord(cnf[73])).zfill(2),
			ha='left', va='top', family='monospace', fontsize=11,
			bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
		## display accelerometer settings: ###############################
			g_range = pow(2,1+ord(cnf[12])-48)
			self.fig.text( 0.34, 0.96, 'Accelerometer settings', 
				ha='left', va='top', family='monospace',fontsize=11,
				bbox=dict(boxstyle='round',facecolor='grey',alpha=.4))
			self.fig.text( 0.34, 0.927, 
				'acc. range: '+ u"\u00B1" + str(g_range) +'g'
				+ '\nsampled at: ' + str(self.bw_lookup[ord(cnf[13])-48])
				+ 'Hz (' + str(self.md_lookup[ord(cnf[14])-48])
				+ ')\npower mode: ' + str(self.pw_lookup[ord(cnf[15])-48])
				+ '\nRLE delta : ' + str(cnf[20]), ha='left', va='top', 
				family='monospace',fontsize=11, linespacing=1.25,
				bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
		## display log file settings: ####################################
		self.fig.text( 0.64, 0.96, 'Log information', 
			ha='left', va='top', family='monospace',fontsize=11,
			bbox=dict(boxstyle='round',facecolor='grey',alpha=.4))
		self.t_tme = self.fig.text( 0.64, 0.927, 'log started at:  ' 
			+ str(mld.num2date(dta.t[0]))[:19]  + '\nlog stopped at:  ', 
			ha='left', va='top', family='monospace', fontsize=11, 
			bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
		self.b = self.fig.text( 0.02, 0.03, 'statistics loading...', 
			ha='left',va='top',family='monospace',fontsize=10)
		## set window title: #############################################
		self.fig.canvas.set_window_title(
			'loading from '+fn+' on HHG#'+cnf[:4])
		ion()
		draw()
		self.fig.show()
	def update_plot(self, dta, s=''):
		## update the timeseries plots: ##################################
		self.linesx.set_xdata(dta.t)
		self.linesy.set_xdata(dta.t)
		self.linesz.set_xdata(dta.t)
		self.linesx.set_ydata(dta.ax)
		self.linesy.set_ydata(dta.ay)
		self.linesz.set_ydata(dta.az)
		## clear and replot ambient data: ################################
		self.ambfill.remove()
		self.ambfill = self.axe.fill_between(dta.t, dta.l1>>8, 
			facecolor='yellow', lw=0, alpha=.6)
		## make sure to update x axes to current day: ####################
		self.ax.axes.set_xlim(int(dta.t[-1]), int(dta.t[-1])+1)
		self.axe.axes.set_xlim(int(dta.t[-1]), int(dta.t[-1])+1)
		## update the log info ###########################################
		if len(s)>54:
			self.t_tme.set_text( self.t_tme.get_text()[:54]  
			+ str(mld.num2date(dta.t[-1]))[:19] + '\nraw 3D samples:  ' 
			+ str(s[-10:]) + '\nRunLEn samples: ' + str(s[-22:-11]))
		## update the stats info #########################################
		self.b.set_text(s)
		draw()
		##################################################################
	def save_plot(self, fn):
		self.fig.savefig(fn+'.pdf', format='pdf', dpi=100, 
			bbox_inches='tight')
		self.fig.savefig(fn+'.png', format='png', dpi=17, 
			bbox_inches='tight')
