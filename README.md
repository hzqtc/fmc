# FMC (FMD Client)

Basic CLI client and web based client for [FMD](https://github.com/hzqtc/fmd). Just like MPC for MPD.

## Command Line

	python2 fmc.py [options] [command]

Options: -a for FMD address, -p for FMD port.

Command: one of "play", "stop", "skip", "ban", "rate", "unrate" and "info".

## Web UI

	python2 webfmc.py [port]
	
Run FMD web based client.

Web.py is required to run this web based client. You may also want to install it with lighttpd or Apache, please read [web.py install guide](http://webpy.org/install) for help.

