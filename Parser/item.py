import sys
import os
from symbol import *

class Item(object):
    left = None
    right = []

    def __init__(self):
        self.left = None
        self.right = []

    def showItem(self):
        print(self.left.getName()+ " ->",end='')
        for i in self.right:
            print(" "+i.name, end='')
        print("")

    def showItem_(self):
        print(self.left.getName()+ " ->",end='')
        for i in self.right:
            print(" "+i.name, end='')
    
    def getLeft(self):
        return self.left.name

    def getRight(self):
        return self.right 
