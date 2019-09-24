import sys
import os
class Word(object):
    def __init__(self):
        self.value = ""
        self.type = -1
        self.lineCount = -1
        self.startCharIndex = -1
        self.endCharIndex = -1

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
                    w.type = -3#源文件结束
                elif ch == '#':
                    w.value += ch
                    w.type = 0
                    self.hasEnd = True
                elif self.isDigit(ch):
                    w.value += ch
                    while True:
                        if self.isLineEnd():
                            w.type = 8
                            break
                        ch = self.readNextChar()
                        if self.isDigit(ch):
                            w.value += ch
                        elif self.isLetter(ch):
                            w.value += ch
                            w.type = -2
                            self.readPreChar()
                            break
                        else:
                            w.type = 8
                            self.readPreChar()
                            break
                elif ch == '=':
                    w.value += ch

                    # =
                    if self.isLineEnd():
                        w.type = 9

                    ch = self.readNextChar()
                    # ==
                    if ch == '=':
                        w.value += ch
                        w.type = 14
                    # =
                    else:
                        w.type = 9
                        self.readPreChar()
                elif ch == '+':
                    w.value += ch
                    w.type = 10
                elif ch == '-':
                    w.value += ch
                    w.type = 11
                elif ch == '/':
                    w.value += ch
                    tempPos = self.textCount

                    # /
                    if self.isLineEnd():
                        w.type = 13

                    ch = self.readNextChar()
                    # //
                    if ch == '/':
                        w.value += ch
                        w.type = 26
                        while not self.isLineEnd():
                            self.readNextChar()
                    # /*
                    elif ch == '*':
                        w.value += ch
                        w.type = 27
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
                        w.type = 13
                        self.readPreChar()
                elif ch == '*':
                    w.value += ch
                    tempPos = self.textCount

                    # *
                    if self.isLineEnd():
                        w.type = 12

                    ch = self.readNextChar()
                    if ch == '/':
                        w.value += ch
                        w.type = 28
                    else:
                        w.type = 12
                        self.readPreChar()
                elif ch == '!':
                    w.value += ch

                    if self.isLineEnd():
                        self.hasError = True
                        w.type = -3
                    
                    ch = self.readNextChar()
                    if ch == '=':
                        w.value += ch
                        w.type = 15
                    else:
                        self.hasError = True
                        w.type = -3
                        self.readPreChar()
                elif ch == '<':
                    w.value += ch

                    # <
                    if self.isLineEnd():
                        w.type = 16

                    ch = self.readNextChar()
                    # <=
                    if ch == '=':
                        w.value += ch
                        w.type = 17
                    # <
                    else:
                        w.type = 16
                        self.readPreChar()
                elif ch == '>':
                    w.value += ch

                    # >
                    if self.isLineEnd():
                        w.type = 18

                    ch = self.readNextChar()
                    # >=
                    if ch == '=':
                        w.value += ch
                        w.type = 19
                    # >
                    else:
                        w.type = 18
                        self.readPreChar()
                elif ch == ';':
                    w.value += ch
                    w.type = 20
                elif ch == ',':
                    w.value += ch
                    w.type = 21
                elif ch == '(':
                    w.value += ch
                    w.type = 22
                elif ch == ')':
                    w.value += ch
                    w.type = 23
                elif ch == '{':
                    w.value += ch
                    w.type = 24
                elif ch == '}':
                    w.value += ch
                    w.type = 25
                elif self.isLetter(ch):
                    w.value += ch
                    while True:
                        if self.isLineEnd():
                            w.type = 29
                            break

                        ch = self.readNextChar()
                        if self.isDigit(ch) or self.isLetter(ch):
                            w.value += ch
                        else:
                            w.type = 29
                            self.readPreChar()
                            break
                else:
                    w.value += ch
                    w.type = -5

                if w.type == 29:
                    if w.value == "int":
                        w.type = 1
                    elif w.value == "void":
                        w.type = 2
                    elif w.value == "if":
                        w.type = 3
                    elif w.value == "else":
                        w.type = 4
                    elif w.value == "while":
                        w.type = 5
                    elif w.value == "return":
                        w.type = 6
                    else:
                        w.type = 7
                
                if w.type >= 26 and w.type <= 28:
                    w.startCharIndex = tempPos
                    w.endCharIndex = tempPos
                else:
                    w.endCharIndex = self.textCount
                    w.startCharIndex = w.endCharIndex - len(w.value) + 1
        else:
            w.type = -6        
        return w

if __name__=="__main__":
    T = Tokenizer()
    if not T.openFile("code.cpp"):
        print("Failed to open the file")
        sys.exit()
    
    while True:
        w = T.readNextWord()
        if w.type < 0:
            break
        print(str(w.lineCount)+'\t'+str(w.startCharIndex)+'\t'+str(w.endCharIndex)+'\t'+str(w.type)+'\t'+w.value)

    T.closeFile()
    