#实现`soft link`机制，并设计测试用例说明你实现的正确性。

##设计思路
执行softlink(target, newfile)，相当于创建新文件newfile，newfile的内容是target的inode在数组inodes中的下标。

##实现

###在fs中增加函数：

```
def doSoftLink(self):
        dprint('doSoftLink')
        if len(self.files) == 0:
            return -1
        parent = self.dirs[int(random.random() * len(self.dirs))]
        nfile = self.makeName()

        # pick random target
        target = self.files[int(random.random() * len(self.files))]

        # get full name of newfile
        if parent == '/':
            fullName = parent + nfile
        else:
            fullName = parent + '/' + nfile

        dprint('try createSoftLink(%s %s %s)' % (target, nfile, parent))
        inum = self.createSoftLink(target, nfile, parent)
        if inum >= 0:
            self.files.append(fullName)
            self.nameToInum[fullName] = inum
            if printOps:
                print 'softLink("%s", "%s");' % (target, fullName)
            return 0
        return -1
```

###在block中增加函数：

```
def addSoftLink(self, tinum):
        assert(self.ftype == 'lf')
        self.data = tinum
```

###在程序的需要处增加文件类型'lf'：
例如，将

```
assert(ftype == 'd' or ftype == 'f' or ftype == 'free')
```
改为：

```
assert(ftype == 'd' or ftype == 'f' or ftype == 'free' or ftype == 'lf')
```

###在run函数中，将doLink改为doSoftLink

```
elif r < 0.7:
    rc = self.doSoftLink()
    dprint('doSoftLink rc:%d' % rc)
```

##运行结果说明
在创建u文件的软链接x时，创建文件x，x文件的内容是3，inodes[3]即对应着u文件的inode：

```
creat("/u");

inode bitmap  11110000
inodes        [d a:0 r:5] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1) (q,2) (u,3)] [(.,1) (..,0)] [] [] [] [] [] [] 

softLink("/u", "/x");

inode bitmap  11111000
inodes        [d a:0 r:6] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [lf a:2 r:1] [] [] [] 
data bitmap   11100000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,4)] [(.,1) (..,0)] [3] [] [] [] [] [] 
```

删除x文件时，将其当做普通文件直接删除：

```
creat("/g/c");

inode bitmap  11111110
inodes        [d a:0 r:7] [d a:1 r:3] [f a:-1 r:1] [f a:-1 r:1] [lf a:2 r:1] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11110000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,4) (t,5)] [(.,1) (..,0) (c,6)] [3] [(.,5) (..,0)] [] [] [] [] 

unlink("/x");

inode bitmap  11110110
inodes        [d a:0 r:6] [d a:1 r:3] [f a:-1 r:1] [f a:-1 r:1] [] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11010000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (t,5)] [(.,1) (..,0) (c,6)] [] [(.,5) (..,0)] [] [] [] []
```

##完整运行结果：

```
HYdeMacBook-Pro:q3 apple$ python sfs-homework.py 
ARG seed 0
ARG numInodes 8
ARG numData 8
ARG numRequests 10
ARG reverse False
ARG printFinal False

Initial state

inode bitmap  10000000
inodes        [d a:0 r:2] [] [] [] [] [] [] [] 
data bitmap   10000000
data          [(.,0) (..,0)] [] [] [] [] [] [] [] 

mkdir("/g");

inode bitmap  11000000
inodes        [d a:0 r:3] [d a:1 r:2] [] [] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1)] [(.,1) (..,0)] [] [] [] [] [] [] 

creat("/q");

inode bitmap  11100000
inodes        [d a:0 r:4] [d a:1 r:2] [f a:-1 r:1] [] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1) (q,2)] [(.,1) (..,0)] [] [] [] [] [] [] 

creat("/u");

inode bitmap  11110000
inodes        [d a:0 r:5] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1) (q,2) (u,3)] [(.,1) (..,0)] [] [] [] [] [] [] 

softLink("/u", "/x");

inode bitmap  11111000
inodes        [d a:0 r:6] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [lf a:2 r:1] [] [] [] 
data bitmap   11100000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,4)] [(.,1) (..,0)] [3] [] [] [] [] [] 

mkdir("/t");

inode bitmap  11111100
inodes        [d a:0 r:7] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [lf a:2 r:1] [d a:3 r:2] [] [] 
data bitmap   11110000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,4) (t,5)] [(.,1) (..,0)] [3] [(.,5) (..,0)] [] [] [] [] 

creat("/g/c");

inode bitmap  11111110
inodes        [d a:0 r:7] [d a:1 r:3] [f a:-1 r:1] [f a:-1 r:1] [lf a:2 r:1] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11110000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,4) (t,5)] [(.,1) (..,0) (c,6)] [3] [(.,5) (..,0)] [] [] [] [] 

unlink("/x");

inode bitmap  11110110
inodes        [d a:0 r:6] [d a:1 r:3] [f a:-1 r:1] [f a:-1 r:1] [] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11010000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (t,5)] [(.,1) (..,0) (c,6)] [] [(.,5) (..,0)] [] [] [] [] 

mkdir("/g/w");

inode bitmap  11111110
inodes        [d a:0 r:6] [d a:1 r:4] [f a:-1 r:1] [f a:-1 r:1] [d a:2 r:2] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11110000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (t,5)] [(.,1) (..,0) (c,6) (w,4)] [(.,4) (..,1)] [(.,5) (..,0)] [] [] [] [] 

fd=open("/g/c", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);

inode bitmap  11111110
inodes        [d a:0 r:6] [d a:1 r:4] [f a:-1 r:1] [f a:-1 r:1] [d a:2 r:2] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11111000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (t,5)] [(.,1) (..,0) (c,6) (w,4)] [(.,4) (..,1)] [(.,5) (..,0)] [o] [] [] [] 

fd=open("/g/c", O_WRONLY|O_APPEND); write(fd, buf, BLOCKSIZE); close(fd);

inode bitmap  11111110
inodes        [d a:0 r:6] [d a:1 r:4] [f a:-1 r:1] [f a:-1 r:1] [d a:2 r:2] [d a:3 r:2] [f a:-1 r:1] [] 
data bitmap   11111100
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (t,5)] [(.,1) (..,0) (c,6) (w,4)] [(.,4) (..,1)] [(.,5) (..,0)] [o] [k] [] [] 


```