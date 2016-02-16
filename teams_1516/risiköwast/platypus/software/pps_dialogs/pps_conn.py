import sys, os, time, socket
from gi.repository import Gtk
import paramiko
import pps_io.pps_import as ppsimport

class pps_connect_dlg(Gtk.Window):

    def __init__(self):
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        self.connected = False

        Gtk.Window.__init__(self, title="Connect to Platypus")

        self.set_size_request(400, 100)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.add(vbox)

        hbox_ip = Gtk.Box(spacing=0)
        hbox_user = Gtk.Box(spacing=0)
        hbox_passwd = Gtk.Box(spacing=0)
        hbox_connect = Gtk.Box(spacing=0)
        vbox.pack_start(hbox_ip, True, True, 0)
        vbox.pack_start(hbox_user, True, True, 0)
        vbox.pack_start(hbox_passwd, True, True, 0)
        vbox.pack_start(hbox_connect, True, True, 0)

        label_ip = Gtk.Label("Platypus IP:")
        label_ip.set_justify(Gtk.Justification.RIGHT)
        hbox_ip.pack_start(label_ip, True, True, 10)
        self.entry_ip = Gtk.Entry()
        self.entry_ip.set_text("")
        self.entry_ip.set_activates_default(True)
        hbox_ip.pack_start(self.entry_ip, True, True, 10)

        label_user = Gtk.Label("Platypus User:")
        label_user.set_justify(Gtk.Justification.RIGHT)
        hbox_user.pack_start(label_user, True, True, 10)
        self.entry_user = Gtk.Entry()
        self.entry_user.set_text("root")
        self.entry_user.set_activates_default(True)
        hbox_user.pack_start(self.entry_user, True, True, 10)

        label_passwd = Gtk.Label("Platypus Password:")
        label_passwd.set_justify(Gtk.Justification.RIGHT)
        hbox_passwd.pack_start(label_passwd, True, True, 10)
        self.entry_passwd = Gtk.Entry()
        self.entry_passwd.set_text("")
        self.entry_passwd.set_visibility(False)
        self.entry_passwd.set_activates_default(True)
        hbox_passwd.pack_start(self.entry_passwd, True, True, 10)

        button_conn = Gtk.Button("Connect")
        button_conn.connect("clicked", self.on_connect_clicked)
        hbox_connect.pack_start(button_conn, True, True, 10)
        button_conn.set_can_default(True)
        button_conn.grab_default()

        self.connect("delete-event", Gtk.main_quit)
        self.show_all()

    def on_connect_clicked(self, button):
        ip = self.entry_ip.get_text()
        user = self.entry_user.get_text()
        passwd = self.entry_passwd.get_text()

        print('Trying to connect to ', user, '@', ip, sep='')

        try:
            self.ssh.connect(ip, username=user, password=passwd)
            print('Connected')
            #dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, "Connected")
            #dialog.run()
            #dialog.destroy()
            self.connected = True
            self.destroy()
            Gtk.main_quit()
        except paramiko.BadHostKeyException:
            print('Server host key could not be verified.')
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, "Server host key could not be verified")
            dialog.run()
            dialog.destroy()
        except paramiko.AuthenticationException:
            print('Authentication failed.')
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, "Authentication failed")
            dialog.run()
            dialog.destroy()
            self.entry_passwd.set_text("")
        except paramiko.SSHException as e:
            print('Error connecting or establishing SSH session:', e)
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, "Error connecting or establishing SSH session")
            dialog.run()
            dialog.destroy()
        except socket.error as e:
            print('Socket error occurred while connecting:', e)
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, "Socket error occurred while connecting")
            dialog.run()
            dialog.destroy()
        except socket.timeout as e:
            print('Socket timeout occurred while connecting.', e)
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, "Socket timeout occurred while connecting")
            dialog.run()
            dialog.destroy()


