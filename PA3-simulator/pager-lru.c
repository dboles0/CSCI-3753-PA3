/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
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
    static int subset_pages[3];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int pc;
    int proc;
    int page;
    int oldpage;
    int chosen_page;
    int flag;

    /* initialize static vars on first run */
    /* zero-fill-on-deman */
    if(!initialized){
    	/* Select first active process */
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
		for(pagetmp=0; pagetmp < MAXPROCESSES; pagetmp++){
			timestamps[proctmp][pagetmp] = 0;
		}
	}
    initialized = 1;
    }	


    /* TODO: Implement LRU Paging */
    /* loop through process frames*/	
    for(proc=0; proc < MAXPROCESSES; proc++){
	
		/* Is process active? */
		if(q[proc].active) {
		    /* Dedicate all work to first active process*/
		    pc = q[proc].pc;	 	        // program counter for process
		    page = pc/PAGESIZE; 		// page the program counter needs

		    /* Is Page Swapped In? = No */
		    /* if we do not find the given page in memory then continue*/
		    if(!q[proc].pages[page]) {

			/* Call pagin() = Failure */
			/* if we cannot swap the page then we need to do some work*/
			if(!pagein(proc,page)) {
			
		
				/* Select a Page to Evict */	
				/* find the page that has not been used for the longest time period */
				/* loop through the process q returning smallest used timestamp */
				for(oldpage=0; oldpage < q[proc].npages; oldpage++){
				
					if(oldpage != page){	
						// loop through add one more if not there
						// if full call find_LRU_page 
						// replace then move to next one
						chosen_page = -1;
						chosen_page = find_LRU_page(timestamps[proc], oldpage);		

						/* Call pageout() = Failure */
						if(!pageout(proc, chosen_page)){
							//investigate failure
							break;
						}
					}
					
				}


				/* Call pageout() = Success*/
			}	
    			timestamps[proc][page] = tick;      // set time stamp for swaped process

			/* Call pagin() = Success */
		    }

		    /* Is Page Swapped In? = Yes */
		    break;
		}
    }

    /* advance time for next pageit iteration */
    tick++;
}

int find_LRU_page(int process_times[], int a_page){
	// process_times = [8][9][6][7][10]   <-(proc)
	//                       ^
	//                      (i)
	// step 1: i=8, min_page=8
	// step 2: i=9, min_page=8
	// step 3: i=6, min_page=6; 
	// step 4: i=7, min_page=6; 

	int i = 0;
	int min_page = process_times[0];
	int chosen_page = 0;
	
	for(i = 1; i < a_page; i++){
		if(process_times[i] < min_page){
			min_page = process_times[i];
			chosen_page = i;
		}
	}

	return chosen_page;
}
