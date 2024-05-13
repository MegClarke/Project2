# You Spin Me Round Robin

This module simulates a round robin scheduling for a given workload and quantum length. It outputs the verage waiting & response time.

## Building

```shell
make
```

## Running
```shell
./rr processes.txt 3
```
processes.txt:
4
1,0,7
2,2,4
3,4,1
4,5,4

quantum length: 3

Results 
```shell
Average waiting time: 7.00
Average response time: 2.75
```

## Cleaning up

```shell
make clean
```
