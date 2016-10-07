import matplotlib, numpy, gtk
matplotlib.use('GTKAgg')
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
			subplots_adjust(	left  = 0.02, right = 1, # left, right, 
									bottom = 0.1,top = 0.72, # bottom and top
									wspace = 0, # width space betw. subplots
									hspace = 0  # height space btw. subplots
								)
	def plot(self, dta, fn='',cnf=''):
		## plot data and clean up the axes: ##############################
		self.fix_margins()
		self.ax = self.fig.add_subplot(2,1,2, axisbg='#FFFFFF')
		self.linesx, = self.ax.plot_date(dta.t, dta.ax, '-r', lw=0.5)
		self.linesy, = self.ax.plot_date(dta.t, dta.ay, '-g', lw=0.5)
		self.linesz, = self.ax.plot_date(dta.t, dta.az, '-b', lw=0.5)
		self.axe = self.fig.add_subplot(4,1,2, axisbg='#777777')
		self.ambfill = self.axe.fill_between(dta.t, dta.e1>>8, 
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
		self.ambfill = self.axe.fill_between(dta.t, dta.e1>>8, 
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





# main plotting routine for generic purposes
class PPS_plot:
	def __init__(self, fig_x=10, fig_y=6, fdpi=80):
		self.fig = 0
		self.save_dta_file = ''
		self.labels = None
		self.a_ax = None
		self.dayborders = []
		try:
			self.fig = figure(	num=None, figsize=(fig_x, fig_y), 
										dpi=fdpi, facecolor='w', edgecolor='k' )
		except ValueError:
			return 0
	def fix_margins(self):
			subplots_adjust(	left  = .02, right = .99, # left, right, 
								bottom = .04,top = .96, # bottom and top
								wspace = .00, # width space betw. subplots
								hspace = .02  # height space btw. subplots
							)
	def draw_top_text(self, strs):
		# evenly draw the strings at the top of the plotting window
		total_strlength = sum([ len(i) for i in strs])
		# draw the first string aligned to the left, last to the right:
		t = self.fig.text( 0.02, 0.95, strs[0], ha='left')
		setp(t, va='baseline', family='monospace', size='medium', 
				bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
		if len(strs)>1:	
			t = self.fig.text( 0.98, 0.95, strs[-1], ha='right')
			setp(t, va='baseline', family='monospace', size='medium', 
					bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
			# the middle ones should be spread across the plot  <-------->
			for i in range(1,len(strs)-1):
				t =self.fig.text(.2+float((i)*(.9/(len(strs)))),.95,strs[i])
				setp(t, ha='center', va='baseline', family='monospace',
					size='medium', 
					bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))	
	# obtain the filename to which the original data should be saved
	def save_data(self, tdata):
		self.save_dta_file,self.save_dta_type = fsave_dlg.hhg_fsave()
	def mark_day(self,tb_data):
		res = ginput(1,timeout=-1)
		self.dayborders.append(res[0][0])
		print self.dayborders
	# read user clicks on the plot for annotation
	def mark_label(self, tb_data):
		res = ginput(2,timeout=-1) # wait for the user to mark two points
		self.labels = [res[0][0], res[1][0]]
		self.add_labels(self.lblcombo.get_active_text())
	def add_labels(self, label_str):
		arrow_props = dict(arrowstyle="<|-|>", connectionstyle="arc3")
		self.ann_a = self.a_ax.annotate("", xy=(self.labels[0], 230), 
							xycoords='data', xytext=(self.labels[1], 230), 
							textcoords='data', arrowprops=arrow_props)
		bbox_props = dict(boxstyle="round,pad=0.3",fc="cyan",ec="b",lw=1)
		middle_co = self.labels[0] + (self.labels[1]-self.labels[0])/2
		self.ann_t = self.a_ax.text(middle_co, 240, 
							label_str, ha="center", va="bottom", 
							rotation=0, size=10, bbox=bbox_props)
		self.fig.canvas.draw()
	def add_extra_tools(self):
		# create annotation combo box on tool item
		self.lblcombo = gtk.combo_box_new_text(); 
		self.lblcombo.show()
		self.lblcombo.append_text("Sleep"); # should be loaded from file
		self.lblcombo.append_text("Breakfast")
		self.lblcombo.append_text("Lunch")
		self.lblcombo.append_text("Dinner")
		self.lblcombo.append_text("Leisure")
		self.lblcombo.set_active(0);
		# add annotation and quit buttons to the toolbar:
		toolbar = get_current_fig_manager().toolbar
		savedta_tb = gtk.ToolButton(gtk.STOCK_SAVE_AS); savedta_tb.show()
		savedta_tb.connect("clicked", self.save_data)
		sep1_tb = gtk.SeparatorToolItem(); sep1_tb.show()
		labelstr_tb = gtk.ToolItem(); labelstr_tb.show()
		labelstr_tb.add(self.lblcombo)
		label_tb = gtk.ToolButton(gtk.STOCK_INDEX); label_tb.show()
		label_tb.connect("clicked", self.mark_label)
		sep2_tb = gtk.SeparatorToolItem(); sep2_tb.show()
		day_tb = gtk.ToolButton(gtk.STOCK_CUT);  day_tb.show()
		day_tb.connect("clicked",self.mark_day)
		sep3_tb = gtk.SeparatorToolItem(); sep3_tb.show()
		exit_tb = gtk.ToolButton(gtk.STOCK_QUIT); exit_tb.show()
		exit_tb.connect("clicked", gtk.main_quit)
		try:
			savedta_tb.set_tooltip_text( 'Save data')
			labelstr_tb.set_tooltip_text( 'Select annotation')
			label_tb.set_tooltip_text( 'Annotate')
			exit_tb.set_tooltip_text( 'Close window')
			day_tb.set_tooltip_text( 'Mark days')
		except:
			print 'Tooltips not available. Hm.'
		toolbar.insert(sep1_tb,  	8)
		toolbar.insert(labelstr_tb,9)
		toolbar.insert(label_tb, 	10)
		toolbar.insert(sep2_tb,  	11)
		toolbar.insert(day_tb,  	12)
		save_fig = toolbar.get_nth_item(13) # the save figure toolitem
		save_fig.set_icon_name(gtk.STOCK_SAVE_AS) # changes the icon 
		toolbar.insert(savedta_tb, 14)
		toolbar.insert(sep3_tb,  	15)
		toolbar.insert(exit_tb,  	16)
	def show(self):
		self.fix_margins()
		self.fig.show(); show()



# plots raw representation of datasets
class PPS_raw_plot(PPS_plot):
	def plot(self, i, n, ts=[], dta=[], plot_label = '', uid='anon0001'):
		if i==1:
			self.a_ax = self.fig.add_subplot(n,1,i); ax = self.a_ax
			self.draw_top_text((('dates_span: '+str(mld.num2date(ts[0]))[:10]+
							 ' -> '+str(mld.num2date(ts[-1]))[:10] ),
							('entries_#: ' + str(len(ts)) ),
							('subject_id: ' + uid) ) )
		else:
			ax = self.fig.add_subplot(n,1,i, sharex=self.a_ax)
		if ts!=[]:
			ax.plot_date(ts, dta, '-', lw=0.5)
		ax.yaxis.set_label_text(plot_label, fontsize=10)
		setp(ax.get_xticklabels(), visible=False)
		setp(ax.get_yticklabels(), visible=False)
		ax.grid(color='k', linestyle=':', linewidth=0.5)
		if n==i: # last plot?
			ax.xaxis.set_major_formatter(mld.DateFormatter('%Y-%m-%d\n %H:%M:%S'))
			setp(ax.get_xticklabels(), visible=True, fontsize=10)
		return ax
	def draw_top_text(self, strs):
		# evenly draw the strings at the top of the plotting window
		total_strlength = sum([ len(i) for i in strs])
		# draw the first string aligned to the left, last to the right:
		t = self.fig.text( .02, .977, strs[0], ha='left')
		setp(t, va='baseline', family='monospace', size='medium', 
				bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
		if len(strs)>1:	
			t = self.fig.text( .98, .977, strs[-1], ha='right')
			setp(t, va='baseline', family='monospace', size='medium', 
					bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))
			# the middle ones should be spread across the plot  <-------->
			for i in range(1,len(strs)-1):
				t =self.fig.text(.2+float((i)*(.9/(len(strs)))),.977,strs[i])
				setp(t, ha='center', va='baseline', family='monospace',
					size='medium', 
					bbox=dict(boxstyle='round',facecolor='yellow',alpha=.4))	
	def equidist_plot(self, i, n, ts, dta, plot_label = ''):
		ax = self.fig.add_subplot(n,1,i)
		ax.plot(ts, dta, '-', lw=0.5)
		ax.yaxis.set_label_text(plot_label, fontsize=10)
		setp(ax.get_xticklabels(), visible=True, fontsize=10)
		setp(ax.get_yticklabels(), visible=False)
		setp(ax.get_yticklabels(), fontsize=8)
		ax.grid(color='k', linestyle=':', linewidth=0.5)
		ax.axes.set_xlim(0, len(dta))
	def show(self):
		self.fix_margins()
		self.add_extra_tools()
		self.fig.show(); show()
		
		

