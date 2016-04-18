#实现的调度方式

*   FIFO
*   SJF
*   RR
*   MLFQ

命令分别为：

*	./scheduler-homework.py  -p FIFO
*	./scheduler-homework.py  -p SJF
*	./scheduler-homework.py  -p RR
*	./scheduler-homework.py  -p MLFQ

注：本程序将job均视为cpu密集型程序，未考虑IO

#输出
FIFO：

```
HYdeMacBook-Pro:lab6 apple$ ./scheduler-homework.py  -p FIFO
ARG policy FIFO
ARG jobs 3
ARG maxlen 10
ARG seed 0

Here is the job list, with the run time of each job: 
  Job 0 ( length = 9 )
  Job 1 ( length = 8 )
  Job 2 ( length = 5 )


** Solutions **

Execution trace:
[ time   0 ] Run job 0 for 9.00 secs ( DONE at 9.00 )
[ time   9 ] Run job 1 for 8.00 secs ( DONE at 17.00 )
[ time   17 ] Run job 2 for 5.00 secs ( DONE at 22.00 )

Final statistics:
  Job   0 -- Response: 0.00  Turnaround 9.00  Wait 0.00
  Job   1 -- Response: 9.00  Turnaround 17.00  Wait 9.00
  Job   2 -- Response: 17.00  Turnaround 22.00  Wait 17.00

  Average -- Response: 8.67  Turnaround 16.00  Wait 8.67
```

SJF:

```
HYdeMacBook-Pro:lab6 apple$ ./scheduler-homework.py  -p SJF
ARG policy SJF
ARG jobs 3
ARG maxlen 10
ARG seed 0

Here is the job list, with the run time of each job: 
  Job 0 ( length = 9 )
  Job 1 ( length = 8 )
  Job 2 ( length = 5 )


** Solutions **

Execution trace:
[ time   0 ] Run job 2 for 5.00 secs ( DONE at 5.00 )
[ time   5 ] Run job 1 for 8.00 secs ( DONE at 13.00 )
[ time   13 ] Run job 0 for 9.00 secs ( DONE at 22.00 )

Final statistics:
  Job   2 -- Response: 0.00  Turnaround 5.00  Wait 0.00
  Job   1 -- Response: 5.00  Turnaround 13.00  Wait 5.00
  Job   0 -- Response: 13.00  Turnaround 22.00  Wait 13.00

  Average -- Response: 6.00  Turnaround 13.33  Wait 6.00
```

RR:

```
HYdeMacBook-Pro:lab6 apple$ ./scheduler-homework.py  -p RR
ARG policy RR
ARG jobs 3
ARG maxlen 10
ARG seed 0

Here is the job list, with the run time of each job: 
  Job 0 ( length = 9 )
  Job 1 ( length = 8 )
  Job 2 ( length = 5 )


** Solutions **

Execution trace:
  [ time   0 ] Run job   0 for 1.00 secs
  [ time   1 ] Run job   1 for 1.00 secs
  [ time   2 ] Run job   2 for 1.00 secs
  [ time   3 ] Run job   0 for 1.00 secs
  [ time   4 ] Run job   1 for 1.00 secs
  [ time   5 ] Run job   2 for 1.00 secs
  [ time   6 ] Run job   0 for 1.00 secs
  [ time   7 ] Run job   1 for 1.00 secs
  [ time   8 ] Run job   2 for 1.00 secs
  [ time   9 ] Run job   0 for 1.00 secs
  [ time  10 ] Run job   1 for 1.00 secs
  [ time  11 ] Run job   2 for 1.00 secs
  [ time  12 ] Run job   0 for 1.00 secs
  [ time  13 ] Run job   1 for 1.00 secs
  [ time  14 ] Run job   2 for 1.00 secs ( DONE at 15.00 )
  [ time  15 ] Run job   0 for 1.00 secs
  [ time  16 ] Run job   1 for 1.00 secs
  [ time  17 ] Run job   0 for 1.00 secs
  [ time  18 ] Run job   1 for 1.00 secs
  [ time  19 ] Run job   0 for 1.00 secs
  [ time  20 ] Run job   1 for 1.00 secs ( DONE at 21.00 )
  [ time  21 ] Run job   0 for 1.00 secs ( DONE at 22.00 )

Final statistics:
  Job   0 -- Response: 0.00  Turnaround 22.00  Wait 13.00
  Job   1 -- Response: 1.00  Turnaround 21.00  Wait 13.00
  Job   2 -- Response: 2.00  Turnaround 15.00  Wait 10.00

  Average -- Response: 1.00  Turnaround 19.33  Wait 12.00
```

MLFQ:

```
HYdeMacBook-Pro:lab6 apple$ ./scheduler-homework.py  -p MLFQ
ARG policy MLFQ
ARG jobs 3
ARG maxlen 10
ARG seed 0

Here is the job list, with the run time of each job: 
  Job 0 ( length = 9 )
  Job 1 ( length = 8 )
  Job 2 ( length = 5 )


** Solutions **

Execution trace:
  [ time   0 ] Run job   0 for 1.00 secs
  [ time   1 ] Run job   1 for 1.00 secs
  [ time   2 ] Run job   2 for 1.00 secs
  [ time   3 ] Run job   0 for 2.00 secs
  [ time   5 ] Run job   1 for 2.00 secs
  [ time   7 ] Run job   2 for 2.00 secs
  [ time   9 ] Run job   0 for 4.00 secs
  [ time  13 ] Run job   1 for 4.00 secs
  [ time  17 ] Run job   2 for 2.00 secs ( DONE at 19.00 )
  [ time  19 ] Run job   0 for 2.00 secs ( DONE at 21.00 )
  [ time  21 ] Run job   1 for 1.00 secs ( DONE at 22.00 )

Final statistics:
  Job   0 -- Response: 0.00  Turnaround 21.00  Wait 12.00
  Job   1 -- Response: 1.00  Turnaround 22.00  Wait 14.00
  Job   2 -- Response: 2.00  Turnaround 19.00  Wait 14.00

  Average -- Response: 1.00  Turnaround 20.67  Wait 13.33
```



