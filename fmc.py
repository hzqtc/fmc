#! /usr/bin/env python2

import sys
import json
import socket
import getopt

# Default configuration, modify them to map your FMD setting.
default_addr = 'localhost'
default_port = 10098

class FMC(object):
	def __init__(self, addr = default_addr, port = default_port):
		self.addr = addr
		self.port = port

	def runcmd(self, cmd):
		conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try:
			conn.connect((socket.gethostbyname(self.addr), self.port))
		except:
			print 'Connect to FMD failed. Is FMD running?'
			return
		res = conn.recv(1024)	# receive welcome info

		conn.send(cmd)
		res = conn.recv(4096)
		conn.send('bye')
		conn.close()

		try:
			return json.loads(res)
		except:
			print res

def show(result):
	status = result['status']
	print 'Status: %s' % status
	if status == 'playing':
		artist = result['song']['artist'].encode('utf-8')
		title = result['song']['title'].encode('utf-8')
		like = result['song']['like']
		progress = result['progress']
		length = result['length']
		print '%s%s - %s: %d / %d' % ('[liked] ' if like else '', artist, title, progress, length)

if __name__ == '__main__':
	opts, cmd = getopt.getopt(sys.argv[1:], 'a:p:h')
	addr = default_addr
	port = default_port

	for k,v in opts:
		if k == '-h':
			print 'Usage: %s [-a addr] [-p port] [cmd]' % sys.argv[0]
			sys.exit(0)
		elif k == '-a':
			addr = v
		elif k == '-p':
			port = int(v)
	fmc = FMC(addr, port)

	if cmd:
		command = cmd[0]
	else:
		command = 'info'
	show(fmc.runcmd(command))
