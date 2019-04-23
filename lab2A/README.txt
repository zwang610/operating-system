

Files:
lab2_add.c ... a C program that implements and tests a shared variable add function, implements the (below) specified command line options, and produces the (below) specified output statistics.
SortedList.h ... a header file (supplied by us) describing the interfaces for linked list operations.
SortedList.c ... a C module that implements insert, delete, lookup, and length methods for a sorted doubly linked list (described in the provided header file, including correct placement of yield calls).
lab2_list.c ... a C program that implements the (below) specified command line options and produces the (below) specified output statistics.
Makefile ... a program to build the deliverable programs (lab2_add and lab2_list), output, graphs, and tarball. 
lab2_add.csv ... containing all of your results for all of the Part-1 tests.
lab2_list.csv ... containing all of your results for all of the Part-2 tests.
lab2_add-1.png ... the (existence of) successful runs vs the number of threads and iterations.
lab2_add-2.png ... the time per operation vs the number of iterations, for yield and non-yield executions.
lab2_add-3.png ... for a single thread, the average cost per operation (non-yield) as a function of the number of iterations.
lab2_add-4.png ... the (existence of) successful runs vs the number of threads and iterations for each synchronization option (none, mutex, spin, compare-and-swap).
lab2_add-5.png ... the average time per operation (non-yield), vs the number of threads.
lab2_list-1.png ... the mean cost per operation vs the number of iterations.
lab2_list-2.png ... the (existence of) successful runs vs the number of threads and iterations for non-yield and each of the above four yield combinations.
lab2_list-3.png ... the (existence of) successful runs vs the number of iterations for mutex and spin-lock protection with each of the above four yield combinations.
lab2_list-4.png ... the (corrected for list length) per operation times (for each of the synchronization options: mutex, spin) vs the number of threads.

Testing Methodology:
The tests is performed while computing the results for the graph. All the test cases is included in test.sh file.

Question and Anwer:
QUESTION 2.1.1 - causing conflicts:
It takes many iterations before errors are seen because smaller number of iterations may be finished before other threads are created. 
And If the iterations is finished before other threads' creation, there is no race condition at all. 

QUESTION 2.1.2 - cost of yielding:
Yield is much slower because we call function shed_yield which may cause context switch. 
The additional time goes to the context switchs. 
No, since the yield function may still run at the same time by different threads.

QUESTION 2.1.3 - measurement errors:
The fixed cost of time spending on creating and joining threads are inevitable. 
When the number of iteration increases, the fixed cost is apportion to more iteration, so the average cost decrease
When the number of iteration is very large, the average cost will get very close to the true average cost.
We can get the true cost by gradient descent.

QUESTION 2.1.4 - costs of serialization:
Smaller number of threads make the race condition less likely to happen, since less threads will use the same resource.
Larger number of threads make the race condition happen more often.
Therefore, protected operation take less execution time with less threads and more execution time with more threads.
The spin_locks is so expensive because it will cause the CPU to continue polling and waiting. Which wasted more times.

QUESTION 2.2.1 - scalability of Mutex:
Similarity: (reason is covered in 2.1.4)
The cost per operation increases when the number of iteration increases.
Difference:
The cost per operation for link list version is smaller than cost per operation for add version with small number of iteration.
(This is because there is a fixed cost for synchronization which is included in our average cost)
The cost per operation grows faster for for link list version. The relative rate is about 3:1 to 6:1.
(The cost of for link list version significantly increases when the number of iterations goes up, it costs more time waiting comparing to the add version)

QUESTION 2.2.2 - scalability of spin locks
Similarity: (reason is covered in 2.1.4)
The cost per operation increases when the number of iteration increases.
Difference:(reason is covered in 2.1.4)
The cost per operation of spin_locks is less than mutex when the number of iteration is small.
The cost per operation of mutex increase slower than spin_locks when the number of iterations increases.
