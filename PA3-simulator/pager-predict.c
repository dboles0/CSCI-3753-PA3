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


void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int p_table[MAXPROCESSES+1][MAXPROCPAGES+1];


    /* Loocal vars */
    int proctmp;
    int pagetmp;
    int pc;
    int proc;
    int page;
    int min_page;
    int chosen_proc;
    int chosen_page;
    int cnt = 85999;

    /* initialize static vars on first run */
    /* zero-fill-on-deman */
    if(!initialized){
        /* Select first active process */
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
                for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                        timestamps[proctmp][pagetmp] = 0;
                        p_table[proctmp][pagetmp] = 0;
                }
        }
	/* initialize a totals column */
	for(int x=0; x<MAXPROCESSES; x++){
		p_table[x][MAXPROCPAGES] = 0;
	}
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

				/* Check threashold for thrashing */
				if(p_table[proc][MAXPROCPAGES] != 0){
					check_threashold(q, proc, timestamps, p_table);
				}

                                /* initialize min_page and chosen proc/page*/
                                min_page = timestamps[0][0];
                                chosen_proc = 0;
                                chosen_page = 0;

                                /* find the page that has not been used for the longest time period */
                                /* look throught all processes to find lowest timestamp */
                                for(int check_proc = 0; check_proc < MAXPROCESSES; check_proc++){
                                        for(int check_page = 0; check_page < MAXPROCPAGES; check_page++){

                                                //if we have a memory block that is smaller and exists in memeory update 
                                                if(min_page > timestamps[check_proc][check_page] && q[proc].pages[check_page]){
                                                        min_page = timestamps[check_proc][check_page];
                                                        chosen_proc = check_proc;
                                                        chosen_page = check_page;
                                                }
                                        }
                                }

                                timestamps[chosen_proc][chosen_page] = tick;      // set time stamp for swaped process
				p_table[chosen_proc][chosen_page] += 1;           // add count to page
				p_table[chosen_proc][MAXPROCPAGES] += 1;          // add to chosen program total
				p_table[MAXPROCESSES][MAXPROCPAGES] += 1;         // add to total p_table

				/* Call pageout() = Failure */
				if(!pageout(chosen_proc, chosen_page)){
                                        //investigate failure
                                        //perror("pageout error");
                                        break;
                                } /* Call pageout() = Success*/

                        } /* Call pagin() = Success */
                        // in the event swap success increment tick 
                        else {
                                timestamps[proc][page] = tick;
                        }

                    } /* Is Page Swapped In? = Yes */

                } /* Is process active? = No */

    // advance the tick count for the next processes
    tick++;

    } /* Another Process? = Yes/No */
    if(p_table[MAXPROCESSES][MAXPROCPAGES] > cnt){

	    for(int i=0; i< MAXPROCESSES; i++){
		for(int j=0; j<MAXPROCPAGES; j++){
			printf("[%d]", p_table[j][i]);
		}
		printf("\n");
	    }
	    printf("\n");

    }

}
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
void check_threashold(Pentry q[MAXPROCESSES], int proc, int timestamps[MAXPROCESSES][MAXPROCPAGES], int p_table[MAXPROCESSES+1][MAXPROCPAGES+1]) {
	int total = p_table[MAXPROCESSES][MAXPROCPAGES];
	int upper_threashold = 8/10;
	int lower_threashold = 2/10;

	int max_process = 0; 
	int max_process_value = 0;
	int min_process = 0;
	int min_process_value = total;
	
	int p_upper = 0;
	int p_lower = 0;

	// find the max and min processes
	for(int i=0; i<MAXPROCESSES; i++){
		if(p_table[i][MAXPROCPAGES] > max_process_value){
			max_process_value = p_table[i][MAXPROCPAGES];
			max_process = i;
		}		
		if(p_table[i][MAXPROCPAGES] < min_process_value){
			min_process_value = p_table[i][MAXPROCPAGES];
			min_process = i;
		}		
	}
		
	p_upper = max_process_value/total;
	p_lower = min_process_value/total;

	// if there's over an 80% probablility the next action will be above the threashold	
	// then we will oppen free 10 memory blocks for alloacation for that process
	if(p_upper > upper_threashold){
	
		for(int i=0; i<10; i++){
			/* initialize min_page and chosen proc/page*/
			int min_page = timestamps[0][0];
			int chosen_page = 0;

			for(int check_page = 0; check_page < MAXPROCPAGES; check_page++){

				//if we have a memory block that is smaller and exists in memeory update 
				if(min_page > timestamps[min_process][check_page] && q[proc].pages[check_page]){
					min_page = timestamps[min_process][check_page];
					chosen_page = check_page;
				}
			}
			

			if(!pageout(min_process, chosen_page)){
				//investigate failure
				//perror("pageout error");
				break;
			} 
		}
	}
	// if there is an 80% probablility the next action will be above the threashold	
	if(p_lower < lower_threashold){
		
		for(int i=0; i<10; i++){
			/* initialize min_page and chosen proc/page*/
			int max_page = timestamps[0][0];
			int chosen_page = 0;

			/* find the page that has not been used for the longest time period */
			/* look throught all processes to find lowest timestamp */
			for(int check_page = 0; check_page < MAXPROCPAGES; check_page++){

				//if we have a memory block that is smaller and exists in memeory update 
				if(max_page < timestamps[max_process][check_page] && q[proc].pages[check_page]){
					max_page = timestamps[max_process][check_page];
					chosen_page = check_page;
				}
			}

                        pagein(max_process, chosen_page);
		}
	}

}

// If there was a miss on any given page 
// what is the probability the next action will be a miss?
// well if we have A then we know A' is the only other option
// hit rate = percentage of memory accesses which are satisfied by page
// miss rate = 1 - hit rate
// 

//                  Event              Satisfied accesses
// P(hit) =>   P =  ________  so  P = ____________________
//                  Outcomes           Memory accesses

// e.x. P1 = [2][4][10][1][7]      access = {4, 3, 2, 1, 5, 7, 4, 3, 2} P = 6/9 hit = .666 and miss = .333                   
// current matrix = A [.66] "p(hit)"
// repeat hits (rep hit#) = A -> A [.33] "p(hit given hit)"
              

//    TREE REPRESENTATION
//              -.33 A
//       .66 A-       
//     -        -.66 A'
// 0 -          - _ A
//     - .33 A'-
//              - _ A'


//  TRANSISTION MATIRIX
//        A  A'
//     A|.66 .33| 
//    A'| _   _ |

