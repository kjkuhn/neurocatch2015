# neurocatch2015




------- Sphero -------
LINK:

	https://github.com/slock83/sphero-linux-api

SETUP

1.	get the device address:
		hcitool scan

2.	make an entry in /etc/bluetooth/rfcomm.conf:

			rfcomm0 {
		#	# Automatically bind the device at startup
			bind no;
		#
		#	# Bluetooth address of the device
			device 00:06:66:44:64:F7;
		#
		#	# RFCOMM channel for the connection
			channel	1;
		#
		#	# Description of the connection
			comment "Sphero";
			}

3.	connect the device:
		sudo rfcomm connect rfcomm0


Remark:
	
	right-directed angle
