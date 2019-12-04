import sys
import os
import copy
from production import *
from symbol import *
from item import *
from action import *
from tokenizer import *
from snapshoot import * 

class Grammar(object):
    __of = None
    __productionFileName = None
    __production_counter = 0
    __productions = set()
    __productionsNo = {}
    __terminalFileName = None
    __terminal_counter = 0
    __terminals = set()
    __argumentFileName = None
    __argument_counter = 0
    __arguments = set()

    __items = set()

    __ERROR = Symbol()
    __startArgument = ""
    __endTerminal = Symbol()
    __itemDot = Symbol()
    __start = ""

    __closure = {}
    __go = {}

    __symbolFirst = {}
    __first = {}
    __follow = {}

    __action = {}

    __goto = {}

    __stateStack = []
    __symbolStack = []

    __tokenizer = None
    __lastWord = None

    def __init__(self):
        self.__ERROR.name = "\0"
        self.__ERROR.no = -1
        self.__itemDot.name = "."
        self.__itemDot.no = 30
        self.__tokenizer = Tokenizer()
        self.__tokenizer.openFile("code.cpp")

    def setProductionFileName(self, fileName):
        self.__productionFileName = fileName
    
    def setTerminalFileName(self, fileName):
        self.__terminalFileName = fileName

    def setArgumentFileName(self, fileName):
        self.__argumentFileName = fileName
    
    def __openFile(self, fileType):
        """
            type:   1 production
                    2 terminal
                    3 argument
        """
        if self.__of:
            return -1
        else:
            fileName = ""
            if fileType == 1:
                fileName = self.__productionFileName
            elif fileType == 2:
                fileName = self.__terminalFileName
            elif fileType == 3:
                fileName = self.__argumentFileName
            self.__of = open(fileName,encoding="utf-8")
            if self.__of is None:
                return -2
            else:
                return 0
    
    def __closeFile(self):
        self.__of.close()
        self.__of = None
        if self.__of:
            return -1
        else:
            return 0
    
    def showProductions(self):
        for i in self.__productions:
            i.showProduction()
    
    def showTerminals(self):
        for i in self.__terminals:
            i.showSymbol()
    
    def showArguments(self):
        for i in self.__arguments:
            i.showSymbol()

    def readProductions(self):
        if self.__openFile(1) != 0:
            return -1
        else:
            self.__production_counter = int(self.__of.readline().replace('\n','').replace(' ',''))

            for i in range(self.__production_counter):
                line = self.__of.readline()
                p = Production(line)
                self.__productions.add(p)

            if self.__closeFile() != 0:
                return -2
            else:
                return 0
        
    def readTerminals(self):
        if self.__openFile(2) != 0:
            return -1
        else:
            self.__terminal_counter = int(self.__of.readline().replace('\n','').replace(' ',''))

            for i in range(self.__terminal_counter):
                line = self.__of.readline()
                temp = line.split(' ')
                name = temp[0]
                no = int(temp[1].replace('\n','').replace(' ',''))
                s = Symbol(name, no)
                self.__terminals.add(s)

            if self.__closeFile() != 0:
                return -2
            else:
                return 0

    def readArguments(self):
        if self.__openFile(3) != 0:
            return -1
        else:
            self.__argument_counter = int(self.__of.readline().replace('\n','').replace(' ',''))

            for i in range(self.__argument_counter):
                line = self.__of.readline()
                temp = line.split(' ')
                name = temp[0]
                no = int(temp[1].replace('\n','').replace(' ',''))
                s = Symbol(name, no)
                self.__arguments.add(s)

            if self.__closeFile() != 0:
                return -2
            else:
                return 0

    def __isTerminal(self, s):
        for i in self.__terminals:
            if i.name == s:
                return True
        return False

    def __isArgument(self, s):
        for i in self.__arguments:
            if i.name == s:
                return True
        return False

    def __findTerminal(self, s):
        for i in self.__terminals:
            if i.name == s:
                return i
        return self.__ERROR

    def __findArgument(self, s):
        for i in self.__arguments:
            if i.name == s:
                return i
        return self.__ERROR

    def __findSymbol(self, s):
        if self.__findTerminal(s) != self.__ERROR:
            return self.__findTerminal(s)
        else:
            return self.__findArgument(s)
        
    def checkProductions(self):
        left = Symbol()
        right = Symbol()
        for i in self.__productions:
            if not self.__isArgument(i.left.name):
                return False
            else:
                left = self.__findArgument(i.left.name)
                i.left.setNo(left.no)
            
            for j in i.right:
                right = j
                if self.__isTerminal(right.name):
                    right = self.__findTerminal(right.name)
                    j.setNo(right.no)
                elif self.__isArgument(right.name):
                    right = self.__findArgument(right.name)
                    j.setNo(right.no)
                else:
                    return False
        return True

        epsilon_set = set()
        epsilon = self.__findTerminal("ε")
        if epsilon != self.__ERROR:
            epsilon_set.add(epsilon)

    def setStart(self, s):
        self.__start = s

    def setStartArgument(self,s):
        self.__startArgument = s

    def setEndTerminal(self, s, n):
        self.__endTerminal = Symbol(s, n)

    def setItemDot(self, s, n):
        self.__itemDot = Symbol(s, n)

    def getEpsilon(self):
        epsilon = self.__findTerminal("ε")
        return epsilon

    def getDot(self):
        dot = Symbol()
        dot.name = self.__itemDot.name
        dot.no = self.__itemDot.no
        return dot

    def getStartArgument(self):
        return self.__findArgument(self.__startArgument)

    def generateItems(self):
        epsilon = self.getEpsilon()
        dot = self.getDot()
        productions = self.__productions
        for p in productions:
            newItem = Item()
            newItem.left = p.left
            if p.right[0] == epsilon:
                newItem.right.append(dot)
                self.__items.add(newItem)
            
            size = len(p.right) + 1
            for i in range(size):
                i_s = 0
                s = p.right[i_s]
                for j in range(size):
                    if i == j:
                        newItem.right.append(dot)
                    else:
                        newItem.right.append(s)
                        i_s += 1
                        if i_s < size - 1:
                            s = p.right[i_s]
                # newItem.right.append(s)
                newItem_cpy = copy.copy(newItem)
                self.__items.add(newItem_cpy)
                newItem.right = []

    def showItems(self):
        for i in self.__items:
            i.showItem()

    def getItemStartWithDot(self, argument):
        s = set()

        for i in self.__items:
            if i.left.name != argument.name:
                continue
            if i.right[0].name == ".":
                s.add(i)
        
        return s

    def extendClosure(self, c):
        closure = c

        loopFlag = True
        while loopFlag:
            loopFlag = False
            newArguments = set()
            for i_item in closure:
                for i in range(len(i_item.right)):
                    if  (not (i_item.right[i].name == ".")) or (i + 1) == len(i_item.right):
                        continue

                    if not self.__isArgument(i_item.right[i + 1].name):
                        continue
                    
                    if i_item.right[i + 1] not in newArguments:
                        newArguments.add(self.__findArgument(i_item.right[i + 1].name))
            
            for i in newArguments:
                set_u = self.getItemStartWithDot(i) | closure
                if len(set_u) > len(closure):
                    closure = set_u
                    loopFlag = True
        
        return closure

    def generateFirstClosure(self):
        items = self.__items
        i_item = Item()
        firstClosure = set()
        for i_item in items:
            if i_item.left.name != self.getStartArgument().name:
                continue
            if i_item.right[0].name == "." and i_item.right[1].name == self.__start:
                break

        firstClosure.add(i_item)

        return self.extendClosure(firstClosure)

    def getItem(self, left, right):
        item = None
        for i in self.__items:
            if i.left.name != left:
                continue
            if len(i.right) != len(right):
                continue
            for j in range(len(right)):
                if right[j].name != i.right[j].name:
                    break
                if j == len(right) - 1:
                    item = i
                    return item

    def getNewClosure(self, i, s):
        J = set()
        closure = self.__closure[i]
        for i_c in closure:
            right = i_c.right
            for i in range(len(right)):
                if (not right[i].name ==".") or (i + 1) == len(right):
                    continue
                if right[i + 1].name != s.name:
                    continue
                
                new_right = copy.deepcopy(i_c.right)
                new_right.pop(i)
                new_right.insert(i+1, self.getDot())
                new_item = self.getItem(i_c.left.name, new_right)
                J.add(new_item)

        c = set()
        for i_c in J:
            c.add(i_c)
            right = i_c.right
            for i in range(len(right)):
                if (not right[i].name ==".") or (i + 1) == len(right):
                    continue
                if not self.__isArgument(right[i + 1].name):
                    continue

                temp = set()
                temp = self.getItemStartWithDot(right[i + 1])
                for j in temp:
                    c.add(j)

        return self.extendClosure(c)

    def isClosureEqual(self, closure1, closure2):
        c1 = set()
        for i in closure1:
            c1.add(self.getItem(i.left.name, i.right))

        c2 = set()
        for i in closure2:
            c2.add(self.getItem(i.left.name, i.right))

        if c1 == c2:
            return True
        else:
            return False

    def generateAllClosure(self):
        arguments = self.__arguments
        terminals = self.__terminals

        symbols = arguments | terminals

        firstClosure = self.generateFirstClosure();

        if 0 not in self.__closure:
            self.__closure[0] = set()
        self.__closure[0] = firstClosure

        last_size = 0
        loopFlag = True
        while loopFlag:
            loopFlag = False
            now_size = len(self.__closure)
            for i in range(last_size, now_size):
                for i_symbol in symbols:
                    if i not in self.__go:
                        self.__go[i] = {}
                    if i_symbol not in self.__go[i]:
                        self.__go[i][self.__findSymbol(i_symbol.name)] = -1
                    c = self.getNewClosure(i, i_symbol)
                    if len(c) == 0:
                        continue

                    hasShown = False
                    for j in range(now_size):
                        if self.isClosureEqual(self.__closure[j], c):
                        # if self.__closure[j] == c:
                        # if self.isClosuresEqu(self.__closure[j], c):
                            hasShown = True
                            self.__go[i][self.__findSymbol(i_symbol.name)] = j
                            break
                    
                    if not hasShown:
                        temp_size = len(self.__closure)
                        self.__closure[temp_size] = c

                        self.__go[i][self.__findSymbol(i_symbol.name)] = temp_size
                        loopFlag = True
            last_size = now_size

    def showClosures(self):
        for i in self.__closure:
            print(str(i) + " :")
            for j in self.__closure[i]:
                j.showItem()

    def showGo(self):
        for i in self.__go:
            i_sec = self.__go[i]
            for j in i_sec:
                print("GO("+str(i)+", ",end='')
                j.showSymbol_()
                print("):"+str(i_sec[j]))

    def generateSymbolFirst(self):
        epsilon = self.__findTerminal("ε")
        epsilonSet = set()
        if epsilon != self.__ERROR:
            epsilonSet.add(epsilon)

        for p in self.__productions:
            if self.__isTerminal(p.right[0].name):
                if p.left.name not in self.__symbolFirst:
                    self.__symbolFirst[p.left.name] = set()
                self.__symbolFirst[p.left.name].add(self.__findTerminal(p.right[0].name))

        loopFlag = True
        while loopFlag:
            loopFlag = False
            for p in self.__productions:
                if p.left.name not in self.__symbolFirst:
                    self.__symbolFirst[p.left.name] = set()
                rightLen = len(p.right)
                for i in range(rightLen):
                    if self.__isTerminal(p.right[i].name):
                        if self.__findTerminal(p.right[i].name) not in self.__symbolFirst[p.left.name]:
                            self.__symbolFirst[p.left.name].add(self.__findTerminal(p.right[i].name))
                            loopFlag = True
                        break

                    if p.right[i].name not in self.__symbolFirst:
                        self.__symbolFirst[p.right[i].name] = set()
                    d_temp = set()
                    d_temp = self.__symbolFirst[p.right[i].name] - epsilonSet
                    u_temp = set()
                    u_temp = self.__symbolFirst[p.left.name] | d_temp

                    if self.__symbolFirst[p.left.name] != u_temp:
                        self.__symbolFirst[p.left.name] = u_temp
                        loopFlag = True
                    
                    if epsilon not in self.__symbolFirst[p.right[i].name]:
                        break
                    else:
                        i += 1
                        if i == rightLen:
                            if epsilon not in self.__symbolFirst[p.left.name]:
                                self.__symbolFirst[p.left.name].add(epsilon)
                                loopFlag = True
                        else:
                            i -= 1

    def showSymbolFirst(self):
        for i in self.__symbolFirst:
            print(i +" : ")
            for j in self.__symbolFirst[i]:
                j.showSymbol_()
                print(" ",end='')
            print("")

    def generateFirst(self):
        epsilon = self.__findTerminal("ε")
        epsilonSet = set()
        if epsilon != self.__ERROR:
            epsilonSet.add(epsilon)

        for p in self.__productions:
            count = 0
            if p not in self.__first:
                self.__first[p] = set()
            for i in range(len(p.right)):
                if self.__isTerminal(p.right[i].name):
                    if p.right[i].name != epsilon.name:
                        if p not in self.__first:
                            self.__first[p] = set()
                        self.__first[p].add(self.__findTerminal(p.right[i].name))
                    break
                
                d_temp = set()
                d_temp = self.__symbolFirst[p.right[i].name] - epsilonSet
                u_temp = set()
                u_temp = self.__first[p] | d_temp
                self.__first[p] = u_temp

                if epsilon not in self.__symbolFirst[p.right[i].name]:
                    break

                count += 1
            
            if count == len(p.right):
                self.__first[p].add(epsilon)

    def showFirst(self):
        for i in self.__first:
            i.showProduction()
            for j in self.__first[i]:
                j.showSymbol_()
                print(" ",end='')
            print("")

    def generateFollow(self):
        epsilon = self.__findTerminal("ε")
        epsilonSet = set()
        if epsilon != self.__ERROR:
            epsilonSet.add(epsilon)

        startArgument = self.__findArgument(self.__startArgument)
        if startArgument.name not in self.__follow:
            self.__follow[startArgument.name] = set()
        self.__follow[startArgument.name].add(self.__endTerminal)

        for a in self.__arguments:
            if a.name not in self.__follow:
                self.__follow[a.name] = set()
            for p in self.__productions:
                for i in range(len(p.right)):
                    if p.right[i].name == a.name:
                        for j in range(i + 1,len(p.right)):
                            if self.__isTerminal(p.right[j].name):
                                if p.right[j].name != epsilon.name:
                                    self.__follow[a.name].add(self.__findTerminal(p.right[j].name))
                                break

                            d_temp = self.__symbolFirst[p.right[j].name] - epsilonSet
                            u_temp = self.__follow[a.name] | d_temp
                            self.__follow[a.name] = u_temp

                            if epsilon not in self.__symbolFirst[p.right[j].name]:
                                break
        
        loopFlag = True
        while loopFlag:
            loopFlag = False
            for p in self.__productions:
                for i in range(len(p.right)-1,-1,-1):
                    if self.__isArgument(p.right[i].name):
                        u_temp = self.__follow[p.right[i].name] | self.__follow[p.left.name]
                        
                        if self.__follow[p.right[i].name] != u_temp:
                            loopFlag = True
                            self.__follow[p.right[i].name] = u_temp

                        if epsilon not in self.__symbolFirst[p.right[i].name]:
                            break
                    elif p.right[i].name == epsilon.name:
                        continue
                    else:
                        break

    def showFollow(self):
        for i in self.__follow:
            print(i +" : ")
            for j in self.__follow[i]:
                j.showSymbol_()
                print(" ",end='')
            print("")

    def getAllConflictions(self, num):
        closure = self.__closure[num]
        all_conflicts = []
        afterDot = set()
        for i_item in closure:
            if i_item.right[-1] == '.':
                if self.__follow[i_item.left.name] not in all_conflicts:
                    all_conflicts.append(self.__follow[i_item.left.name])
                else:
                    pass
                continue

            for i in range(len(i_item.right)):
                if i_item.right[i].name != "." or i + 1 == len(i_item.right):
                    continue
                if self.__isTerminal(i_item.right[i + 1].name):
                    afterDot.add(i_item.right[i+1])
        
        all_conflicts.append(afterDot)
        return all_conflicts

    def isSLR1(self):
        all_conflicts = []
        inter = set()
        for k in range(len(self.__closure)):
            all_conflicts = []
            all_conflicts = self.getAllConflictions(k)

            for i in range(len(all_conflicts)):
                for j in range(len(all_conflicts)):
                    if i == j:
                        continue
                    inter.clear()
                    inter = all_conflicts[i] & all_conflicts[j]
                    if len(inter) > 0:
                        return False
        return True

    def encodeProduction(self):
        count = 0
        for p in self.__productions:
            if count not in self.__productionsNo:
                self.__productionsNo[count] = None
            self.__productionsNo[count] = p
            count += 1

    def showEncodeProduction(self):
        for i in range(len(self.__productionsNo)):
            print(str(i)+" : " , end='')
            self.__productionsNo[i].showProduction()
    
    def getItemProduction(self, item):
        count = 0
        for i in range(len(item.right)):
            count = i
            if item.right[i].name == ".":
                break

        temp_right = copy.copy(item.right)
        if count < len(temp_right):
            temp_right.pop(count)

        for i in range(len(self.__productionsNo)):
            if self.__productionsNo[i].left.name == item.left.name and len(self.__productionsNo[i].right) == len(temp_right):
                count = 0
                for j in range(len(self.__productionsNo[i].right)):
                    if self.__productionsNo[i].right[j].name == temp_right[j].name:
                        count += 1
                if count == len(self.__productionsNo[i].right):
                    return i
        return i
             
    def showGetProduction(self):
        for i in self.__items:
            i.showItem_()
            print(" : ",end='')
            self.__productionsNo[self.getItemProduction(i)].showProduction()

    def generateSLR1Action(self):
        self.encodeProduction()

        for i in self.__closure:
            if i not in self.__action:
                self.__action[i]={}
            for j in self.__closure[i]:
                if j.left.name == "S'"and len(j.right) == 2 and j.right[0].name == self.__start and j.right[1].name == '.' :
                    self.__action[i][self.__endTerminal.name] = Action("acc")
                    continue
                
                if j.right[-1].name == '.':
                    self.__action[i][self.__endTerminal.name] = Action("r",self.getItemProduction(j))
                    for k in self.__terminals:
                        self.__action[i][k.name] = Action("r",self.getItemProduction(j))
                    for k in self.__follow[j.left.name]:
                        self.__action[i][k.name] = Action("r",self.getItemProduction(j))
                    continue

                for l in range(len(j.right)):
                    if j.right[l].name == '.':
                        if self.__isTerminal(j.right[l + 1].name):
                            self.__action[i][j.right[l + 1].name] = Action("s",self.__go[i][self.__findSymbol(j.right[l + 1].name)])

    def showAction(self, state, s):
        for t in s:
            if t.name not in self.__action[state]:
                print("",end='\t')
            elif self.__action[state][t.name].first == "s" or self.__action[state][t.name].first == "r":
                print(self.__action[state][t.name].first+str(self.__action[state][t.name].second),end='\t')
            elif self.__action[state][t.name].first == "acc":
                print(self.__action[state][t.name].first,end='\t')
            else:
                print("",end='\t')

    def generateGOTO(self):
        for i in range(len(self.__closure)):
            if i not in self.__goto:
                self.__goto[i] = {}
            for a in self.__arguments:
                self.__goto[i][a.name] = self.__go[i][a]

    def showGOTO(self, state):
        for a in self.__arguments:
            print(a.name+" : "+str(self.__goto[state][a.name]),end='\t')

    def test(self):
        set_temp = set()
        set_temp.add(self.__endTerminal)
        set_temp = set_temp | self.__terminals
        print("state",end='\t')
        for t in set_temp:
            print(t.name,end='\t')
        print("goto",end='\t')
        print("")
        for i in range(len(self.__closure)):
            print(str(i),end='\t')
            self.showAction(i,set_temp)
            self.showGOTO(i)
            print("")

    def translate(self, token):
        s = Symbol()
        if token.type == 8:
            s.name = "ID"
        elif token.type == 9:
            s.name = "inum"
        elif token.type == 10:
            s.name = "fnum"
        elif token.type == -2:
            s.name = "#"
        else:
            s.name = token.value
        s.no = token.type
        if s.no == -2:
            s.no = 0
        return s
    
    def getNext(self):
        ss = Snapshoot()

        while self.__lastWord.type >= 28:
            self.__lastWord = self.__tokenizer.readNextWord()

        ss.token = self.__lastWord

        if self.__lastWord.type == -1 or self.__lastWord.type == -3:
            ss.error = -1
            return ss

        nowSymbol = self.translate(self.__lastWord)
        ss.symbol = nowSymbol

        stateTop = self.__stateStack[0]
        au = self.__action[stateTop][nowSymbol.name]
        ss.au = au

        if au.first == "":
            ss.error = -2
        elif au.first == "s":
            self.__symbolStack = [nowSymbol] + self.__symbolStack
            self.__stateStack = [au.second] + self.__stateStack
            ss.stateStack = self.__stateStack
            ss.symbolStack = self.__symbolStack
            self.__lastWord = self.__tokenizer.readNextWord()
            ss.error = 1
        elif au.first == "r":
            nowProduction = self.__productionsNo[au.second]

            length = len(nowProduction.right)
            if length == 1 and nowProduction.right[0] == "ε":
                length = 0
            
            for i in range(length):
                self.__symbolStack.pop(0)
                self.__stateStack.pop(0)
            
            self.__symbolStack = [nowProduction.left] + self.__symbolStack
            ss.production = nowProduction

            stateTop = self.__stateStack[0]
            self.__stateStack = [self.__goto[stateTop][nowProduction.left.name]] + self.__stateStack

            ss.error = 2
        elif au.first == "acc":
            ss.error = 3

        ss.stateStack = self.__stateStack
        ss.symbolStack = self.__symbolStack

        return ss

    def shiftReduce(self, inputList):
        pass
        self.__stateStack.append(0)
        self.__symbolStack = []

        action = self.__action[self.__stateStack[-1]][inputList[0].name]
        if action.first == "s":
            self.__stateStack.append(self.__goto[self.__stateStack[-1]][inputList[0].namr])
            self.__symbolStack.append(inputList[0])
            inputList.pop(0)
            pass
        elif action.first == "r":
            pass
        elif action.first == "acc":
            pass
        else:
            pass

    def initialize(self):
        self.setProductionFileName("./file/productions.txt")
        self.setTerminalFileName("./file/terminals.txt")
        self.setArgumentFileName("./file/arguments.txt")

        self.readProductions()
        self.readTerminals()
        self.readArguments()

        self.setStart("S")
        self.setStartArgument("S'")
        self.setEndTerminal("#", 0)

        self.__stateStack.append(0)
        self.__lastWord = self.__tokenizer.readNextWord()
