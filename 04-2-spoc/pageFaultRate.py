# encoding: UTF-8

pageNumList = [3, 3, 4, 2, 3, 5, 3, 5, 1, 4]
length = len(pageNumList)
memory = [1, 4, 5]
T = 2
tLast = 0

print "initial memory: ", memory
print "page visit list: ", pageNumList
print "T: ", T

for tCur in range(0, length):
    print "tLast: ", tLast, ", tCur: ", tCur,
    if pageNumList[tCur] not in memory:
        print ", [page fault: ", pageNumList[tCur], "]",
        if tCur - tLast > T:
            memory = []
            for i in range(tLast, tCur):
                if pageNumList[i] not in memory:
                    memory.append(pageNumList[i])
        memory.append(pageNumList[tCur])
        tLast = tCur
    else:
        print ", [page hit:   ", pageNumList[tCur], "]",
    print ", memory: ", memory