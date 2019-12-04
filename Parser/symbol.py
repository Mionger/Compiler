import sys
import os

class Symbol(object):
    name = ""
    no = -1

    def __init__(self, s="\0", n=-1):
        self.name = s
        self.no = n

    def showSymbol(self):
        print(self.name+' '+str(self.no))

    def showSymbol_(self):
        print(self.name, end='')

    def setName(self, name):
        self.name = name

    def setNo(self, no):
        self.no = no

    def getName(self):
        return self.name
