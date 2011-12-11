#! /usr/bin/env python2

import sys
import json
import socket
import getopt

class FMC(object):
	def __init__(self, addr = 'localhost', port = 10098):
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

		if cmd == 'info':
			obj = json.loads(res)
			status = obj['status']
			print 'Status: %s' % status
			if status == 'playing':
				artist = obj['song']['artist'].encode('utf-8')
				title = obj['song']['title'].encode('utf-8')
				progress = obj['progress']
				length = obj['song']['length']
				print '%s - %s: %d / %d' % (artist, title, progress, length)

if __name__ == '__main__':
	opts, cmd = getopt.getopt(sys.argv[1:], 'a:p:')
	if cmd:
		if 'a' not in opts and 'p' not in opts:
			fmc = FMC()
		elif 'p' not in opts:
			fmc = FMC(opts['a'])
		else:
			fmc = FMC(opts['a'], opts['p'])
		fmc.runcmd(cmd[0])
	else:
		print 'Usage: %s [-a address] [-p port] [command]' % sys.argv[0]
