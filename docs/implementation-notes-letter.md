# Test Task Implementation Notes

## Decision progress

The task execution took much longer than I originally estimated.

It was quite humiliating when the first version was 5-6 times slower than `grep`
at a very high level of speed optimization with all sorts of statically allocated arrays without memory re-allocations.
And I expected that the result should already be better than analogues. Moreover, I had a specialized algorithm,
while `grep` has a different syntax and the universal algorithm inside. I supposed `grep` to be slower by definition.

Originally I had the line match algorithm with dynamic programming and `O(P*N)` memory.
Then I replaced it with another one, faster and with constant memory.
And... it became only 2 times slower than `grep`. It was almost a success `:)`
At the same time, according to the profiler, 80+% of time was spent exactly in the line match algorithm.

I had no doubt that a more efficient algorithm must exist for this task.
The only way to defeat `grep` was to add a few optimizations that simply scrolled faster
algorithm in the most popular scenarios. This change accelerated the matching algorithm by about 6 times,
and the total running time of the program accelerated by 3 times.
The improved algorithm gave a victory over grep by only 25-35%.

After that, according to the profiler, almost half of the time was spent
in a line match algorithm, and near to 40% of the time was spent in a synchronous `ReadFile()` call.

I decided that this is the finest hour of asynchronous file reading!
Thus, the operating system will read the next data block of the file during parsing and processing the previous one.
I implemented and ... nothing. The total running time has not changed.
But the profiler showed a redistribution of time towards the line match algorithm.
It was very strange that the line matching algorithm slowed down. And I still don't understand why it slowed down.
I am convinced that this 40% spent by `ReadFile()` could be compressed to a maximum of 5%
by parallelizing data proofreading and data processing.
Perhaps this is somehow related to the fact that the data is in the system file cache,
and it is not really read from the disk (shorter IRP path).
Perhaps this banal copying of memory in kernel mode is poorly parallelized.
Maybe it was worth rewriting so that reading from disk in a dedicated thread was performed ...

In the next iteration I tried to map the file to memory.
This solution does not meet requirements because it may throw SEH exceptions in case of disk read errors.
And I had doubts about the effectiveness of the speed of loading new pages in this solution.
Result was slightly slower, the total time of the program has increased by 20% percent.
Although it is also strange when the disk cache is warmed up.
Theoretically, if the data is in the disk cache, then it would be possible to map it
to process readonly virtual memory in `O(1)`,
and then save time on transitions to kernel mode during memory scan + save time on copying memory.

## Testing and Notes

I tested using web server log, 2 GB, 5.5 million lines,
average line length was 380 bytes, all lines are no longer than 1024 bytes.
1600 lines out of 5.5M matched the pattern. I chose the pattern `*string*` as the most popular in everyday life.

The SSD drive was used, but I warmed it up so that all the data got to the system file cache.

CPU: `Intel Core i5 8th Gen`, laptop edition.

Application built under `x64` architecture worked faster than under `x86`.

The `FILE_FLAG_SEQUENTIAL_SCAN` flag did not give a performance boost on a warmed cache.
Without warming the cache, it must be measured separately.

Sometimes the application execution time is kept at +25% for a long time.
Most likely this is due to the fact that I have a laptop and CPU cores have economical modes.

The latest application version takes 1.6 seconds to process test data while `grep` takes 2.5 seconds.

**ADDED:**  
I also implemented reading the file in a separate thread. The file operation is synchronous,
synchronization between threads is done using spinlocks.
It gave a total gain of 25% over the synchronous and asynchronous API solutions (total work time is 1.2 seconds).
This is 2 times faster than `grep`.

## Implementation features

I kept all four implementations of reading files. You can switch them in code:

```cpp
#if 0
#if 0
    CSyncLineReader _lineReader;
#else
    CMappingLineReader _lineReader;
#endif
#else
#if 0
    CAsyncLineReader _lineReader;
#else
    CSpinlockLineReader _lineReader;
#endif
#endif
```

The solution contains unit tests in a separate project based on `gtest` framework.

As required by the challenge, the main console application is built with C++ exceptions disabled and no RTTI.

I have used some parts of the STL at my own risk.
These parts do not use exceptions and work without unnecessary overhead.
I see no reason not to use cheap abstractions that allow you to write cleaner and more error-free code.
I mean all kinds of `std::unique_ptr`, `std::string_view`, `std::optional` and etc

---
