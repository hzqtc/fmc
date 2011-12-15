#! /usr/bin/env python2

import sys
import json
import socket
import getopt

class FMC(object):
	def __init__(self, addr, port):
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
			obj = json.loads(res)
			status = obj['status']
			print 'Status: %s' % status
			if status == 'playing':
				artist = obj['song']['artist'].encode('utf-8')
				title = obj['song']['title'].encode('utf-8')
				like = obj['song']['like']
				progress = obj['progress']
				length = obj['length']
				print '%s%s - %s: %d / %d' % ('[liked] ' if like else '', artist, title, progress, length)
		except:
			print res

if __name__ == '__main__':
	opts, cmd = getopt.getopt(sys.argv[1:], 'a:p:h')
	addr = 'localhost'
	port = 10098

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
		fmc.runcmd(cmd[0])
	else:
		fmc.runcmd('info')
