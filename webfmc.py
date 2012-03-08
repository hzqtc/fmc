#!/usr/bin/env python2

import web
from fmc import FMC

#TODO: move to configuration file
addr = 'localhost'
port = 10098

urls = (
	'/webfmc/static/(.*)', 'Static',
	'/webfmc/(.*)', 'WebUI',
	'(.*)', 'Error',
)
app = web.application(urls, globals())

class Error:
	def GET(self, path):
		return "Error: Unknown path {0}".format(path)

class Static:
    def GET(self, file):
        try:
            web.header('Cache-Control', 'max-age=2592000')
            f = open('static/'+file, 'r')
            return f.read()
        except:
            return '' # you can send an 404 error here if you want

class WebUI:
	def GET(self, cmd):
		if cmd == '':
			cmd = 'info'
		fmc = FMC(addr, port)
		result = fmc.runcmd(cmd)
		render = web.template.render('.')
		return render.index(result)

if __name__ == "__main__":
	app.run()
