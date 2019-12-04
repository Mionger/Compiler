from grammar import * 

if __name__=="__main__":
    print("Start the parser...")

    print("Initializing ...")
    G = Grammar()
    G.initialize()
    print("Success to initialize")

    print("Check the productions...")
    print("The productions are ",end='')
    if G.checkProductions():
        print("valid")
    else:
        print("invalid")

    print("Generate the items...")
    G.generateItems()
    print("Success to generate the items")

    print("Generate the closures...")
    G.generateAllClosure()
    print("Success to generate the closures")

    print("Generate the first...")
    G.generateSymbolFirst()
    G.generateFirst()
    print("Success to generate the first")

    print("Generate the follow...")
    G.generateFollow()
    print("Success to generate the follow")

    print("Check if the grammar is SLR1...")
    print("The grammar ",end='')
    if G.isSLR1():
        print("is SLR1")
    else:
        print("isn`t SLR1")
        exit()

    print("Generate the SLR1 actions...")
    G.generateSLR1Action()
    print("Success to generate the SLR1 actions")

    print("Generate the goto...")
    G.generateGOTO()
    print("Success to generate the goto")

    print("Analyze begin:")

    while True:
        now_snapshoot = G.getNext()

        # information
        if now_snapshoot.au:
            print("Action        : s : " + str(now_snapshoot.au.first)+" n : "+str(now_snapshoot.au.second))
        if now_snapshoot.token and now_snapshoot.symbol:
            print("Word          : value : "+str(now_snapshoot.token.value)+" symbol name : "+str(now_snapshoot.symbol.name))
        print("Production    : ",end="")
        if now_snapshoot.production is None:
            print("Production No : None")
        else:
            print("Production No : ",end='')
            now_snapshoot.production.showProduction()
        print("Error Code    : "+str(now_snapshoot.error))
        print("Symbol Stack  : [",end="")
        for i in range(len(now_snapshoot.symbolStack)):
            if i > 0:
                print(",",end="")
            print(str(now_snapshoot.symbolStack[i].name),end="")
        print("]")
        print("State Stack   : [",end="")
        for i in range(len(now_snapshoot.stateStack)):
            if i > 0:
                print(",",end="")
            print(str(now_snapshoot.stateStack[i]),end="")
        print("]")
        print("")


        if now_snapshoot.error < 0:
            print("Some error occured!")
            break
        elif now_snapshoot.error == 3:
            print("Analyze over")
            break
