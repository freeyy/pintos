# Pintos
Private reprository for [Pintos](http://pintos-os.org).
Pintos is a teaching operating system for x86, challenging but not overwhelming, small
but realistic enough to understand OS in depth (it can run x86 machine and simulators 
including QEMU, Bochs and VMWare Player!). The main source code, documentation and assignments 
are developed by Ben Pfaff and others from Stanford (refer to its [LICENSE](./LICENSE)).
This repository also contains some changes made by [Ryan Huang](huang@cs.jhu.edu).

DO NOT USE THIS FOR YOUR HOMEWORK.

# Source code

- `src/threads`: main part of the code you need to add and modify

- `src/device` :  only need to modify `timer.c`

- `src/lib/kernel`:  C libraries, `list.c` is specially important

- `src/tests`: the test code 

- `init.c`:  kernel initialization entry

# Run and Test

### Run whole test

Just `make grade` in `threads` , and you'll get a `grade` file.

### Run single test

- `pintos -v run alarm-single` where `alarm-single` is a test name.
- The result of every single test is also generated in `threads/build/test` .  open `xxx.output` to see what your program printed during the test.

# Project 1
Includes three parts:
- Alarm Clock
- Priority Scheduling
- Advanced Scheduler

## Some Key Requirements
### pirority shcheduling:
    - interface:
        - `thread_set_priority()`
            - It sets the thread's base priority.
            - The thread's effective priority becomes the higher of the newly set priority or the highest donated priority.
            - When the donations are released, the thread's priority becomes the one set through the function call.
        - `thread_get_priority(void)`
            - In the presence of priority donation, returns the higher (donated) priority.
    - If multiple threads have the same highest priority, `thread_yield()` should switch among them in "round robin" order.

    the `less_func()`  should return `t1.priority < t2.priority`

    - current thread should immediately yield the processor to the new thread with higher priority that added to `ready_list`

    call `thread_yield()` at the end of `thread_unblock()` if `cur.priority ≤ new.priority` ,

    - if current thread lower its priority below the highest priority, call `yield()`
        - thread_set_priority(int new_priority)
    - for a lock, semaphore, or condition variable, the highest priority waiting thread should be awakened first.
        - when a lock is released, the highest priority thread waiting for that lock should be unblocked and put on the list of ready threads.
        - The scheduler should then run the highest priority thread on the ready list.
### priority donation: only for locks
    - single donation:
        - a thread's priority can be changed while it is on the `ready_list` due to a donation.
        - a thread's priority can be changed while it is blocked on a semaphore...
    - multiple donation: multiple priorities are donated to a single thread
    - nested donation: all thread on a lock chain boosted to the highest priority

## Overview Of My Design

### Process Priority

- modify all `list_push_back()` → `ordered_push_back()` on priority
    - `thread.c`: on ready_list
        - thread_yield()
        - thread_unblock()
    - `synch.c`: on `waiters`
        - sema_down()
            - actually meaningless because order can be changed due to donation
            - instead sort `waiters` in `sema_up()`
        - cond_wait()
- preemptive switch check after `thread_unblock()` is called
    - add `intr_yield_on_return()` in interrupt handler
        - timer_interrupt()
    - add `thread_yield()` to others
        - sema_up() (all synch tools rely on semaphore)
        - thread_create()
        - thread_set_priority()

### Priority Donation

- Add new definition:
    - `lock->priority` = max (`waiting threads priority`)
    - `thread->effective_priority` = max(`base priority`, max(`holding-locks priority`))
    - `thread->locks_holding` track all locks held by current thread, for multiple donation
    - `thread->lock_waiting` track the lock a blocked thread is waiting to track a block chain
- A thread's `effective_priority` may change when:
    - its `base_priority` changes: `thread_priority_set()`
    - its `locks_holding_list`  changes, i.e.
        - it holds a new lock: in `lock_acquire()`
        - it releases a lock: in `lock_release()`
        - the priority of an existing held lock changes
- A lock's `lock→priority` may change when its `waiting threads` change:
    - a new thread start waiting: in `lock_acquire()`
    - a waiting thread succeeds to get the lock: in `lock_acquire()`
    - a waiting thread priority changes due to donation: in `lock_acquire()`
- Prevent race whenever setting `effective_priority` using `semaphore`

### MLFQ

- `t->nice`
    - init = `parent->nice`  (zero for the initial thread)
    - `thread_get_nice()`
    - `thread_set_nice(int new_nice)`: Sets the current thread's `nice` value and recalculates thread's priority
- `t→priority` for each thread is an `integer`, recalculated once every fourth clock tick
    - `priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)`
    - current thread `yields` if it no longer has the highest priority, may happen when:
        - creates a child thread
        - thread got a new nice, `thread_set_nice()`
        - every fourth clock tick that updates all thread's priority (except IDLE)
- `recent_cpu` measures how much CPU time each process has received "recently"
    - init = `parent->recent_cpu`  ()
    - at each time tick,  current thread's `recent_cpu ++`
    - at each second, every thread (except IDLE) updates its `recent_cpu`, can be negative:
    `(2*load_avg)/(2*load_avg + 1) * recent_cpu + nice`
- `load_avg`  is a system-wide moving average of the number of threads ready to run
    - at each second, updates `load_avg = (59/60)*load_avg + (1/60)*ready_threads`
    - `ready_threads` is the number of threads that are either running or ready to run at time of update (not including the idle thread)
- disable priority schedule when `thread_mlfqs == true`:
    - priority donation in `lock_acquire` and `lock_release` should be ignored
    - The priority argument to `thread_create()` should be ignored
    - `thread_set_priority()` should be ignored
    - `thread_get_priority()` should return the thread's current priority as set by the scheduler
