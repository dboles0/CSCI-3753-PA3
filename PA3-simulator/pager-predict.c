/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"
#define K 15

// keep track of each page that gets swaped with matrix count
// initialie the matirx with 1's so we have an equal probability  p=(1/MAX)
// keep track of cold and hot programs by maintaining a total count for each process and selecting the lowest count 
// estimate the next one will be below a theashold = the number of counts that are over a certain amount 
// example: if we have 1/8 2/8 2/8 3/8 then we can see that 3 of the pages are => 2 (threashold) so we have 3/8 probability of next entry faulting on hot page) 
// we may also want to consider condiguious blocks of if there are hot block then we would not want to page maybe within 4 bocks afterords to account for locality
// to help with thrashing we make a window of say 4 at time t_1 if our "working set size" is say 4 (unique accesses) and at time t_2 with another 4 windo we have 2 (2, 4, 2, 4). 
// t_1 needs 4 frams and t_2 needs 2 framses (use large windows)
// we may want to swap out a process if there is thrashing happening
// (solution 1) to do this we will need to set a reference threashold for thrashing 
// if we set a timer to go off periotically any time a page is associated set a reference bits so we can see how many frames were refenced during that last time period. it then 0's out bit
// we can then use this information to approximate the page rate hit over that time
// the refereced bit can now be considered part of the working set and use this to calculate how many pages were being accessed in each one of those time periods
// (solution 2) directly mesure the page fault frequency (PFF)
// when PFF > upper threashold, then increase the # frames allocated to process
// when PFF < lower threashold, then decreased the # frames allocated to process

// deal with thrashing by: 
// when PFF > upper threashold, then increase the # frames allocated to process
// when PFF < lower threashold, then decreased the # frames allocated to process

typedef struct {
	int page; 
	int timestamp;
	int hit; 
	int miss;

} matrix_info;

void left_shift(matrix_info *m, int a_page, int timestamp, int hit, int miss){

	for(int i=1; i<K; ++i){
		m[i-1] = m[i];
	}

	m[K-1].page = a_page;
	m[K-1].hit += 1;
	m[K-1].timestamp = timestamp;
	m[K-1].hit = hit;
	m[K-1].miss = miss;
}

matrix_info ** init_matrix(void){
	
	matrix_info **m;
	m = (matrix_info **) malloc(MAXPROCESSES * sizeof(matrix_info *));
	for(int i=0; i< MAXPROCESSES; ++i){
		m[i] = (matrix_info *) malloc(K * sizeof(matrix_info));
	}

	for(int i=0; i<MAXPROCESSES; i++){
		for(int j=0; j<K; j++){
			m[i][j].page = 0;
			m[i][j].hit = 0;
			m[i][j].miss = 0;
			m[i][j].timestamp = 0;
		}
	}

	return m;
}

void destroy_matrix(matrix_info **m){

    
    for(int i=0; i<MAXPROCESSES; ++i){
	free(m[i]);
    }
}


void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    matrix_info **m;

    /* Loocal vars */
    int proctmp;
    int pagetmp;
    int pc;
    int proc;
    int page;
    int min_page;
    int chosen_page;
    

    /* initialize static vars on first run */
    /* zero-fill-on-deman */
    if(!initialized){
        /* Select first active process */
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
                for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                        timestamps[proctmp][pagetmp] = 0;
                }
        }
	/* initialize probability window */
	m = init_matrix();

    initialized = 1;
    }


    /* TODO: Implement LRU Paging */
    /* loop through process frames*/
    for(proc=0; proc < MAXPROCESSES; proc++){


                /* Is process active? = Yes */
                if(q[proc].active) {
                    /* Dedicate all work to first active process*/
                    pc = q[proc].pc;                    // program counter for process
                    page = pc/PAGESIZE;                 // page the program counter needs

                    /* Is Page Swapped In? = No */
                    /* if we do not find the given page in memory then continue*/
                    if(!q[proc].pages[page]) {

                        /* Call pagin() = Failure */
                        /* if we cannot swap the page then we need to do some work*/
                        if(!pagein(proc,page)) {

                                /* Select a Page to Evict */


				/* mark page that missed */
				left_shift(m[proc], page, tick, 0, 1);
				min_page = 9999;
                                chosen_page = 0;

                                for(int check_page = 0; check_page < K; check_page++){
						
					if((min_page > m[proc][check_page].page) & (q[proc].pages[check_page])){
						min_page = m[proc][check_page].page;
						chosen_page = check_page;
					}
				}

				/* Call pageout() = Failure */
				if(!pageout(proc, chosen_page)){
                                        //investigate failure
                                        break;
                                } /* Call pageout() = Success*/

                        } /* Call pagin() = Success */
                        // in the event swap success increment tick 
                        else {
				left_shift(m[proc], page, tick, 1, 0);
                        }

                    } /* Is Page Swapped In? = Yes */
	  	    

                } /* Is process active? = No */

    // advance the tick count for the next processes
    tick++;

    } /* Another Process? = Yes/No */

    /* free matrix */
    //destroy_matrix(m);

}

// refrence: https://homes.cs.washington.edu/~karlin/papers/Markov.pdf
// we have n pages that may be either in memory or on disk
// only k pages may be in memory at any givin time
// if a page currently requestin in memory (hit) no cost in incured
// if page is not in memory (fault) it must be fetched form memory for a cost
// if there are already k pages in memory one of the pages mus be evicted
// the decistion to evict must be made by decistion process
// let M = marcov chain whos state space is the set of n-pages { [p][p][p][p][p] } S = 5
// let A = paging alorithm then f_A(M,k) is the long term frequency of faults

// find transition probabilieis by sampeling large initial prefix of refrence string

// Phase 1: keep a windos of last k+i requested pages, for some 0 < i < k;
//          at the begining of the phase the window is just the k most recently requested pages; 
//          when a "new" page p is requested, it is added to the window.
//          the phase ends when the k+1 distinct page is requested in the current phase. 
//          the window then srings back to size k
//                    

//                  Event              Satisfied accesses
// P(hit) =>   P =  ________  so  P = ____________________
//                  Outcomes           Memory accesses

// e.x. P1 = [4][4][4][4][4][4][4][4][4][4][7][7][7]      access = {4, 3, 2, 1, 5, 7, 4, 3, 2, 4, 7, 3, 8} 
//           [0][3][3][3][3][3][3][3][3][3][3][3][3]
//           [0][0][2][1][5][7][7][7][2][2][2][2][8]

// k = 3
// window of 7
// (1) 4 = 1, (2) 3, = 1, (3) 2 = 1, (4) 1 = 1, (5) 5 = 1, (6) 7 = 1, (7) 4 = 2, (8) 3 = 2, (9) 2 = 2, (10) 4 = 2, (11) 7 =2
// P(P1[0]) = 1st[1], 2nd[.5], 3rd[.33], 4th[.25], 5th[.2], 6th[.16], 7th[.28], 8th[.14], 9th[.14], 10th[.28]
// P(P1[1]) = 1st[0], 2nd[.5], 3rd[.33], 4th[.25], 5th[.2], 6th[.16], 7th[.14], 8th[.28], 9th[.14], 10th[.14]
// P(P1[2]) = 1st[0], 2nd[0],  3rd[.33], 4th[.25], 5th[.2], 6th[.16], 7th[.14], 8th[.14], 9th[.28], 10th[.14]

//            1  2  3  4  5  6  7  8  9 10    11
//            4  3  2  1  5  7  4  3  2  4     7
//      P1 = [4][4][4][1][5][7][4][4][4][4]   [.68][7]
//           [0][3][3][3][3][3][3][3][3][3]   [.85][3]
//           [0][0][2][2][2][2][2][2][2][2]   [.85][2]

// clearly we need marcov chaining to predict next event since this only gets current prob
// so let take a look form 10 to 11

// repeat hits (rep hit#) = A -> A [.28] "p(hit given hit)"
              

//    TREE REPRESENTATION
//              -.07 A
//       .28 A-       
//     -        -.93 A'
// 0 -          -.20 A
//     - .72 A'-
//              -.80 A'


//  TRANSISTION MATIRIX
//        A  A'
//     A|.07 .93| 
//    A'|.20 .80|

//         A   A'
// S_o = |.28 .72| (initial state distrubution matrix/initial state probability matrix)

//         A   A'         A  A'
// S_1 = |.28 .72| *  A|.07 .93|   => [ (.28 * .07) + (.72 * .93) , (.28 *.20) + (.72 * .80) ] => 
//                   A'|.20 .80|

//         A   
// S_1 = |.68 |


//  TRANSISTION MATIRIX 2
//        A  A'
//     A|.01 .99| 
//    A'|.10 .90|

//         A   A'
// S_o = |.14 .86| (initial state distrubution matrix/initial state probability matrix)

//         A   A'         A  A'
// S_1 = |.14 .86| *  A|.01 .99|   => [ (.14 * .01) + (.86 * .99) , (.14 *.10) + (.86 * .90) ] => 
//                   A'|.10 .90|

//         A  
// S_1 = |.85 |


