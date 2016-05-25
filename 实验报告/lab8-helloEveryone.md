#黄予	2013011363	计34
#练习1: 完成读文件操作的实现
##【1.1】打开文件的处理流程
从sh.c进行函数跟踪：

###文件系统访问接口层：

reopen->open->sys_open

sys_open为系统调用的接口，自此进入内核态文件系统抽象层。

###文件系统抽象层	

->sysfile_open->file_open->vfs_open->(vfs_lookup->vop_lookup)、vop_open
									
vfs_open调用vfs_lookup，根据路径找到文件对应的inode结构体，进而由获得的inode结构体调用vop_open打开文件。

###SFS文件系统层
vop_open->sfs_opendir/sfs_openfile
在sfs_inode.c中:

```
// The sfs specific DIR operations correspond to the abstract operations on a inode.
static const struct inode_ops sfs_node_dirops = {
    .vop_magic                      = VOP_MAGIC,
    .vop_open                       = sfs_opendir,
    .vop_close                      = sfs_close,
    .vop_fstat                      = sfs_fstat,
    .vop_fsync                      = sfs_fsync,
    .vop_namefile                   = sfs_namefile,
    .vop_getdirentry                = sfs_getdirentry,
    .vop_reclaim                    = sfs_reclaim,
    .vop_gettype                    = sfs_gettype,
    .vop_lookup                     = sfs_lookup,
};
/// The sfs specific FILE operations correspond to the abstract operations on a inode.
static const struct inode_ops sfs_node_fileops = {
    .vop_magic                      = VOP_MAGIC,
    .vop_open                       = sfs_openfile,
    .vop_close                      = sfs_close,
    .vop_read                       = sfs_read,
    .vop_write                      = sfs_write,
    .vop_fstat                      = sfs_fstat,
    .vop_fsync                      = sfs_fsync,
    .vop_reclaim                    = sfs_reclaim,
    .vop_gettype                    = sfs_gettype,
    .vop_tryseek                    = sfs_tryseek,
    .vop_truncate                   = sfs_truncfile,
};
```
可见调用vop_open调用sfs_opendir或sfs_openfile，之后为sfs文件系统的最终实现，打开文件改层即结束，之后经重重返回，直至返回用户态。顺带一提，若操作为文件读写，不妨以写为例，则将调用sfs_buf_op->sfs_wbuf->sfs_rwblock_nolock等进入下一层：文件系统I/O设备接口。

###文件系统I/O设备接口
调用dop_io进行文件读写


##【1.2】编写在sfs_inode.c中sfs_io_nolock读文件中数据的实现代码。

代码以及思路注释如下：

```
static int
sfs_io_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, void *buf, off_t offset, size_t *alenp, bool write) {
    struct sfs_disk_inode *din = sin->din;
    assert(din->type != SFS_TYPE_DIR);
    off_t endpos = offset + *alenp, blkoff;
    *alenp = 0;
	// calculate the Rd/Wr end position
    if (offset < 0 || offset >= SFS_MAX_FILE_SIZE || offset > endpos) {
        return -E_INVAL;
    }
    if (offset == endpos) {
        return 0;
    }
    if (endpos > SFS_MAX_FILE_SIZE) {
        endpos = SFS_MAX_FILE_SIZE;
    }
    if (!write) {
        if (offset >= din->size) {
            return 0;
        }
        if (endpos > din->size) {
            endpos = din->size;
        }
    }

    int (*sfs_buf_op)(struct sfs_fs *sfs, void *buf, size_t len, uint32_t blkno, off_t offset);
    int (*sfs_block_op)(struct sfs_fs *sfs, void *buf, uint32_t blkno, uint32_t nblks);
    if (write) {
        sfs_buf_op = sfs_wbuf, sfs_block_op = sfs_wblock;
    }
    else {
        sfs_buf_op = sfs_rbuf, sfs_block_op = sfs_rblock;
    }

    int ret = 0;
    size_t size, alen = 0;
    uint32_t ino;
    uint32_t blkno = offset / SFS_BLKSIZE;          // The NO. of Rd/Wr begin block
    uint32_t nblks = endpos / SFS_BLKSIZE - blkno;  // The size of Rd/Wr blocks

  //LAB8:EXERCISE1 2013011363 HINT: call sfs_bmap_load_nolock, sfs_rbuf, sfs_rblock,etc. read different kind of blocks in file
	/*
	 * (1) If offset isn't aligned with the first block, Rd/Wr some content from offset to the end of the first block
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op
	 *               Rd/Wr size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset)
	 * (2) Rd/Wr aligned blocks 
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_block_op
     * (3) If end position isn't aligned with the last block, Rd/Wr some content from begin to the (endpos % SFS_BLKSIZE) of the last block
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op	
	*/

    //读/写第一块，读/写大小<SFS_BLKSIZE
    if ((blkoff = offset % SFS_BLKSIZE) != 0) {
        size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset);//根据offset与endpos是否在同一块内，计算第一块读/写的size
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) {//得到blkno对应的inode编号
            goto out;
        }
        if ((ret = sfs_buf_op(sfs, buf, size, ino, blkoff)) != 0) {//读/写size字节
            goto out;
        }
        alen += size;//已经读/写的字节数
        if (nblks == 0) {
            goto out;
        }
        buf += size, blkno ++, nblks --;
    }

    //读/写中间部分，读写大小 = SFS_BLKSIZE
    size = SFS_BLKSIZE;
    while (nblks != 0) {
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfs_block_op(sfs, buf, ino, 1)) != 0) {
            goto out;
        }
        alen += size, buf += size, blkno ++, nblks --;
    }

    //读/写最后一块，读写大小 < SFS_BLKSIZE
    if ((size = endpos % SFS_BLKSIZE) != 0) {
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfs_buf_op(sfs, buf, size, ino, 0)) != 0) {
            goto out;
        }
        alen += size;
    }

out:
    *alenp = alen;//实际读/写的字节数
    if (offset + alen > sin->din->size) {
        sin->din->size = offset + alen;
        sin->dirty = 1;
    }
    return ret;
}
```


##【1.3】在ucore中实现”UNIX的PIPE机制“的设计方案
设管道所连接的两个进程分别为A、B，A写B读。创建管道，即创建一个临时文件F作为缓冲区，A、B的inode均指向该临时文件F，A通过文件接口往F中写数据，B从F中读数据，A、B通过信号量或者管程互相通知，实现同步互斥。A、B结束后将临时文件删去。


#练习2: 完成基于文件系统的执行程序机制的实现
##【2.1】改写proc.c中的load_icode函数和其他相关函数

1.	alloc_proc函数中增加```proc->filesp = NULL;```，初始化files_struct结构
2.	do_fork函数中增加

	```
if (copy_files(clone_flags, proc) != 0) {
        goto bad_fork_cleanup_fs;
    }
```
以复制进程的打开文件列表。
3.	完成load_icode函数，将可执行文件从硬盘加载到内存中

```
static int
load_icode(int fd, int argc, char **kargv) {
    /* LAB8:EXERCISE2 2013011363  HINT:how to load the file with handler fd  in to process's memory? how to setup argc/argv?
     * MACROs or Functions:
     *  mm_create        - create a mm
     *  setup_pgdir      - setup pgdir in mm
     *  load_icode_read  - read raw data content of program file
     *  mm_map           - build new vma
     *  pgdir_alloc_page - allocate new memory for  TEXT/DATA/BSS/stack parts
     *  lcr3             - update Page Directory Addr Register -- CR3
     */
	/* (1) create a new mm for current process
     * (2) create a new PDT, and mm->pgdir= kernel virtual addr of PDT
     * (3) copy TEXT/DATA/BSS parts in binary to memory space of process
     *    (3.1) read raw data content in file and resolve elfhdr
     *    (3.2) read raw data content in file and resolve proghdr based on info in elfhdr
     *    (3.3) call mm_map to build vma related to TEXT/DATA
     *    (3.4) callpgdir_alloc_page to allocate page for TEXT/DATA, read contents in file
     *          and copy them into the new allocated pages
     *    (3.5) call pgdir_alloc_page to allocate pages for BSS, memset zero in these pages
     * (4) call mm_map to setup user stack, and put parameters into user stack
     * (5) setup current process's mm, cr3, reset pgidr (using lcr3 MARCO)
     * (6) setup uargc and uargv in user stacks
     * (7) setup trapframe for user environment
     * (8) if up steps failed, you should cleanup the env.
     */
     assert(argc >= 0 && argc <= EXEC_MAX_ARG_NUM);

    //新建mm
    if (current->mm != NULL) {
        panic("load_icode: current->mm must be empty.\n");
    }

    int ret = -E_NO_MEM;
    struct mm_struct *mm;
    if ((mm = mm_create()) == NULL) {
        goto bad_mm;
    }

    //建立页目录表
    if (setup_pgdir(mm) != 0) {
        goto bad_pgdir_cleanup_mm;
    }

    struct Page *page;

    //将文件从硬盘加载到内存
    struct elfhdr __elf, *elf = &__elf;

    //读elf文件头
    if ((ret = load_icode_read(fd, elf, sizeof(struct elfhdr), 0)) != 0) {
        goto bad_elf_cleanup_pgdir;
    }

    if (elf->e_magic != ELF_MAGIC) {
        ret = -E_INVAL_ELF;
        goto bad_elf_cleanup_pgdir;
    }

    //读elf文件头中的所有程序头
    struct proghdr __ph, *ph = &__ph;
    uint32_t vm_flags, perm, phnum;
    for (phnum = 0; phnum < elf->e_phnum; phnum ++) {
        off_t phoff = elf->e_phoff + sizeof(struct proghdr) * phnum;//当前读到的程序头在文件中的偏移量

        if ((ret = load_icode_read(fd, ph, sizeof(struct proghdr), phoff)) != 0) {//读程序头
            goto bad_cleanup_mmap;
        }
        if (ph->p_type != ELF_PT_LOAD) {//判断该程序头描述的段是否可装载
            continue ;
        }
        if (ph->p_filesz > ph->p_memsz) {
            ret = -E_INVAL_ELF;
            goto bad_cleanup_mmap;
        }
        if (ph->p_filesz == 0) {//若该程序头描述的段的大小为0，继续读下一个程序头
            continue ;
        }

        //建立程序头的地址映射
        vm_flags = 0, perm = PTE_U;
        //设置标志位
        if (ph->p_flags & ELF_PF_X) vm_flags |= VM_EXEC;
        if (ph->p_flags & ELF_PF_W) vm_flags |= VM_WRITE;
        if (ph->p_flags & ELF_PF_R) vm_flags |= VM_READ;
        if (vm_flags & VM_WRITE) perm |= PTE_W;
        if ((ret = mm_map(mm, ph->p_va, ph->p_memsz, vm_flags, NULL)) != 0) {
            goto bad_cleanup_mmap;
        }
        off_t offset = ph->p_offset;
        size_t off, size;
        uintptr_t start = ph->p_va, end, la = ROUNDDOWN(start, PGSIZE);//la为start的PGSIZE对齐处

        //复制程序头对应的段
        ret = -E_NO_MEM;

        //复制p_filesz（text/data）
        end = ph->p_va + ph->p_filesz;
        while (start < end) {
            if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if (end < la) {
                size -= la - end;
            }
            if ((ret = load_icode_read(fd, page2kva(page) + off, size, offset)) != 0) {
                goto bad_cleanup_mmap;
            }
            start += size, offset += size;
        }

        //p_memsz（bss）开始时未对齐部分初始化
        end = ph->p_va + ph->p_memsz;
        if (start < la) {
            /* ph->p_memsz == ph->p_filesz */
            if (start == end) {
                continue ;
            }
            off = start + PGSIZE - la, size = PGSIZE - off;
            if (end < la) {
                size -= la - end;
            }
            memset(page2kva(page) + off, 0, size);
            start += size;
            assert((end < la && start == end) || (end >= la && start == la));
        }

        //p_memsz（bss）对齐部分初始化
        while (start < end) {
            if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if (end < la) {
                size -= la - end;
            }
            memset(page2kva(page) + off, 0, size);
            start += size;
        }
    }
    sysfile_close(fd);//关闭文件


    //建立用户栈的内存映射
    vm_flags = VM_READ | VM_WRITE | VM_STACK;
    if ((ret = mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags, NULL)) != 0) {
        goto bad_cleanup_mmap;
    }
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-2*PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-3*PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-4*PGSIZE , PTE_USER) != NULL);
    
    //设置当前进程的mm、cr3（页目录表基址）
    mm_count_inc(mm);
    current->mm = mm;
    current->cr3 = PADDR(mm->pgdir);
    lcr3(PADDR(mm->pgdir));

    //将kargv的内容依次放入用户栈
    uint32_t argv_size=0, i;
    for (i = 0; i < argc; i ++) {//计算所有参数的长度和
        argv_size += strnlen(kargv[i],EXEC_MAX_ARG_LEN + 1)+1;
    }

    uintptr_t stacktop = USTACKTOP - (argv_size/sizeof(long)+1)*sizeof(long);
    char** uargv=(char **)(stacktop  - argc * sizeof(char *));
    
    argv_size = 0;
    for (i = 0; i < argc; i ++) {//将传进的参数依次放入用户栈
        uargv[i] = strcpy((char *)(stacktop + argv_size ), kargv[i]);
        argv_size +=  strnlen(kargv[i],EXEC_MAX_ARG_LEN + 1)+1;
    }
    
    stacktop = (uintptr_t)uargv - sizeof(int);
    *(int *)stacktop = argc;
    
    //设置中断帧，这样从中断返回时即可开始执行从elf文件中载入的内容
    struct trapframe *tf = current->tf;
    memset(tf, 0, sizeof(struct trapframe));
    tf->tf_cs = USER_CS;
    tf->tf_ds = tf->tf_es = tf->tf_ss = USER_DS;
    tf->tf_esp = stacktop;
    tf->tf_eip = elf->e_entry;//第一条指令
    tf->tf_eflags = FL_IF;
    ret = 0;

    //错误处理，恢复之前的状态
out:
    return ret;
bad_cleanup_mmap:
    exit_mmap(mm);
bad_elf_cleanup_pgdir:
    put_pgdir(mm);
bad_pgdir_cleanup_mm:
    mm_destroy(mm);
bad_mm:
    goto out;
}
```

##【2.2】注意事项
执行make qemu后若没有从哲学家程序跳转至shell，此时按一下回车即可进入shell，进而执行ls与hello。


##【2.3】硬链接和软链接机制设计方案
###硬链接
与课上写的python程序思路一致，即使用inode结构的ref_count，记录文件的引用计数。

```
inode bitmap  11110000
inodes        [d a:0 r:5] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:1] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1) (q,2) (u,3)] [(.,1) (..,0)] [] [] [] [] [] [] 

link("/u", "/x");

inode bitmap  11110000
inodes        [d a:0 r:6] [d a:1 r:2] [f a:-1 r:1] [f a:-1 r:2] [] [] [] [] 
data bitmap   11000000
data          [(.,0) (..,0) (g,1) (q,2) (u,3) (x,3)] [(.,1) (..,0)] [] [] [] [] [] []
```

当创建文件的硬链接时，文件对应的inode的引用计数增加1。当删除文件时，将inode的引用计数减1，若引用计数为0，则删除文件与其对应的inode。两个硬链接的文件共用同一个inode。

###软链接

设文件B为文件A的软链接，则B亦相当于一个特殊的文件，其中记录其A的所在路径。当创建文件的软链接时，相当于创建一个新文件B，写入所链接文件A的路径。当删除软链接B时，相当于普通的删除文件操作。当访问软链接B时，读取B中存储的路径，以该路径访问，即可访问到A。






