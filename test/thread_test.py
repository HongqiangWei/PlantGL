from PyQt4.QtCore import *
from PyQt4.QtGui import *
from openalea.plantgl.all import *


class MyThread(QThread):
	def __init__(self):
		QThread.__init__(self)
		self.mutex = QMutex()
		self.waitcondition = QWaitCondition()
		self.endmutex = QMutex()
		self.endwaitcondition = QWaitCondition()
		
	def start(self):
		self.mutex.lock()
		QThread.start(self)
		self.waitcondition.wait(self.mutex)
		self.mutex.unlock()
	
	def waitEnd(self):
		self.endwaitcondition.wait(self.endmutex)
	
	def run(self):
		app = QApplication([])
		while not self.mutex.tryLock():
			pass
		self.mutex.unlock()
		self.waitcondition.wakeAll()
		Viewer.threaded = False
		Viewer.start()
		print "execute"
		QThread.exec_(self)
		self.endwaitcondition.wakeAll()

	def doit(self):
		self.start()
		self.endwaitcondition.wait(self.endmutex,2000)
		Viewer.update()
		self.waitEnd()
		while self.isRunning():
			pass

# m = MyThread()
# m.doit()
import code
import openalea.plantgl.all as pgl

class MyWidget(QWidget):
	def __init__(self):
		QWidget.__init__(self)
		self.mutex = QMutex()
		self.waitcondition = QWaitCondition()
	def customEvent(self,event):
		print "Receive Event"
		while not self.mutex.tryLock():
			pass
		self.mutex.unlock()
		self.waitcondition.wakeAll()		
	def sendAnEvent(self,event = QEvent(QEvent.User)):
		self.mutex.lock()
		QApplication.postEvent(self,event)
		self.waitcondition.wait(self.mutex)
		self.mutex.unlock()

class MyThread2(QThread):
	def __init__(self):
		QThread.__init__(self)
	
	def run(self):
		code.interact("Sub thread interpretation",local=globals())
		QApplication.quit()
	def doit(self):
		app = QApplication([])
		# self.widget = MyWidget()
		# self.widget.show()
		pgl.Viewer.start()
		self.start()
		app.exec_()

m = MyThread2()
m.doit()