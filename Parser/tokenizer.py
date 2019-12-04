import sys
import os
from word import *

class Tokenizer(object):
    def __init__(self):
        self.lineCount = 0
        self.textCount = 0
        self.line = ""
        self.lineLength = 0
        self.hasError = False
        self.hasOpen = False
        self.hasEnd = False
        self.of = None

    def openFile(self, fileName):
        self.of = open(fileName, "r")
        if self.of is None:
            return False
        else:
            self.hasOpen = True
            return True

    def closeFile(self):
        self.of.close()
        self.hasOpen = False
        return True

    def isLetter(self, c):
        if (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z'):
            return True
        else:
            return False

    def isDigit(self, c):
        if '0' <= c and c <= '9':
            return True
        else:
            return False

    def isMeaningLess(self):
        result = True
        for i in range(self.lineLength):
            if self.line[i] == ' ' or self.line[i] == '\t':
                continue
            else:
                result = False
                break
        return result

    def isLineEnd(self):
        if self.textCount == self.lineLength:
            return True
        else:
            return False
        
    def readNextLine(self):
        self.line = self.of.readline() 
        if not self.line:
            self.hasEnd = True
            return False
        else:
            self.lineCount += 1
            self.textCount = 0
            self.line = self.line.replace('\n', '')
            self.lineLength = len(self.line)
            return True

    def readNextChar(self):
        nextChar = '\0'
        if self.isLineEnd():
            if(self.hasEnd):
                return '\0'
            while True:
                self.readNextLine()
                if(self.hasEnd):
                    return '\0'
                if not self.isMeaningLess():
                    break
        
        nextChar = self.line[self.textCount]
        self.textCount += 1
        return nextChar

    def readPreChar(self):
        if self.textCount > 0:
            self.textCount -= 1

    def readNextWord(self):
        w = Word()

        tempPos = -1

        if not self.hasEnd:
            if self.hasError:
                w.type = -4# 错误
            else:
                ch = ' '
                while ch == ' ' or ch == '\t':
                    ch = self.readNextChar()
                w.lineCount = self.lineCount
                if ch == '\0':
                    w.type = -2#源文件结束
                elif ch == '#':
                    w.value += ch
                    w.type = 0
                    self.hasEnd = True
                elif ch == '+':
                    w.value += ch
                    w.type = 12
                elif ch == '-':
                    w.value += ch
                    w.type = 13
                elif ch == ';':
                    w.value += ch
                    w.type = 22
                elif ch == ',':
                    w.value += ch
                    w.type = 23
                elif ch == '(':
                    w.value += ch
                    w.type = 24
                elif ch == ')':
                    w.value += ch
                    w.type = 25
                elif ch == '{':
                    w.value += ch
                    w.type = 26
                elif ch == '}':
                    w.value += ch
                    w.type = 27
                elif ch == '/':
                    w.value += ch
                    tempPos = self.textCount

                    # /
                    if self.isLineEnd():
                        w.type = 15

                    ch = self.readNextChar()
                    # //
                    if ch == '/':
                        w.value += ch
                        w.type = 28
                        while not self.isLineEnd():
                            self.readNextChar()
                    # /*
                    elif ch == '*':
                        w.value += ch
                        w.type = 29
                        while True:
                            ch = self.readNextChar()
                            if ch == '\0':
                                break
                            elif ch == '*':
                                if not self.isLineEnd():
                                    ch = self.readNextChar()
                                    if ch == '/':
                                        self.readPreChar()
                                        self.readPreChar()
                                        break
                    # /
                    else:
                        w.type = 15
                        self.readPreChar()
                elif ch == '*':
                    w.value += ch
                    tempPos = self.textCount

                    # *
                    if self.isLineEnd():
                        w.type = 14

                    ch = self.readNextChar()
                    if ch == '/':
                        w.value += ch
                        w.type = 28
                    else:
                        w.type = 14
                        self.readPreChar()
                elif ch == '=':
                    w.value += ch

                    # =
                    if self.isLineEnd():
                        w.type = 11

                    ch = self.readNextChar()
                    # ==
                    if ch == '=':
                        w.value += ch
                        w.type = 16
                    # =
                    else:
                        w.type = 11
                        self.readPreChar()
                elif ch == '<':
                    w.value += ch

                    # <
                    if self.isLineEnd():
                        w.type = 18

                    ch = self.readNextChar()
                    # <=
                    if ch == '=':
                        w.value += ch
                        w.type = 19
                    # <
                    else:
                        w.type = 18
                        self.readPreChar()
                elif ch == '>':
                    w.value += ch

                    # >
                    if self.isLineEnd():
                        w.type = 20

                    ch = self.readNextChar()
                    # >=
                    if ch == '=':
                        w.value += ch
                        w.type = 21
                    # >
                    else:
                        w.type = 20
                        self.readPreChar()
                elif ch == '!':
                    w.value += ch

                    if self.isLineEnd():
                        self.hasError = True
                        w.type = -3
                    
                    ch = self.readNextChar()
                    if ch == '=':
                        w.value += ch
                        w.type = 17
                    else:
                        self.hasError = True
                        w.type = -3
                        self.readPreChar()
                elif self.isDigit(ch) or ch == '.':
                    dot_count = 0
                    if ch == '.':
                        dot_count += 1

                    w.value += ch
                    if ch == '.' and self.isLineEnd():
                        self.hasError = True
                        w.type = -3
                        self.readPreChar()
                    else:
                        while True:
                            if self.isLineEnd():
                                if dot_count == 0:
                                    w.type = 9
                                else:
                                    w.type = 10
                                break

                            ch = self.readNextChar()
                            if self.isDigit(ch):
                                w.value += ch
                            elif ch == '.':
                                if dot_count > 0:
                                    self.hasError = True
                                    w.type = -3
                                    self.readPreChar()
                                    break
                                else:
                                    dot_count += 1
                                    w.value += ch
                            elif self.isLetter(ch):
                                w.value += ch
                                self.hasError = True
                                w.type = -3
                                self.readPreChar()
                                break
                            else:
                                if dot_count == 0:
                                    w.type = 9
                                else:
                                    w.type = 10
                                self.readPreChar()
                                break
                elif self.isLetter(ch) or ch == '_':
                    w.value += ch
                    while True:
                        if self.isLineEnd():
                            w.type = 31
                            break

                        ch = self.readNextChar()
                        if self.isDigit(ch) or self.isLetter(ch) or ch == '_':
                            w.value += ch
                        else:
                            w.type = 31
                            self.readPreChar()
                            break
                else:
                    w.value += ch
                    w.type = -5

                if w.type == 31:
                    if w.value == "int":
                        w.type = 1
                    elif w.value == "float":
                        w.type = 2
                    elif w.value == "void":
                        w.type = 3
                    elif w.value == "if":
                        w.type = 4
                    elif w.value == "else":
                        w.type = 5
                    elif w.value == "while":
                        w.type = 6
                    elif w.value == "return":
                        w.type = 7
                    else:
                        w.type = 8
                
                if w.type >= 28 and w.type <= 31:
                    w.startCharIndex = tempPos
                    w.endCharIndex = tempPos
                else:
                    w.endCharIndex = self.textCount
                    w.startCharIndex = w.endCharIndex - len(w.value) + 1
        else:
            w.type = -6        
        return w  
