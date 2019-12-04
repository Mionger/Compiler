import sys
import os
from symbol import *

class Production(object):
    left = None
    right = []

    def __init__(self, s):
        l = s.split(' ',2)
        self.left = Symbol(l[0])
        self.right = []
        temp = l[2].replace('\n','').replace('$','').strip().split()
        for i in temp:
            self.right.append(Symbol(i))

    def showProduction(self):
        print(self.left.getName()+ " ->",end='')
        for i in self.right:
            print(" "+i.name, end='')
        print("")

    def showProduction_(self):
        print(self.left.getName()+ " ->",end='')
        for i in self.right:
            print(" "+i.name, end='')
