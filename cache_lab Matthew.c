#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define S       16     //  (16 cache sets)
#define E       4      //  (4-way associative cache)
#define B       32     //  (32 elements in each block)
#define T       7      //  (7 tag bits)
#define M       65536  //  (65536 location memory)
#define READ    1
#define WRITE   0

int s ;
int b ;
int m ;

int rhits = 0 ; int rmiss = 0 ; int whits = 0 ; int wmiss = 0 ; int dwrit = 0 ;

struct cache_t
   {
   char dirty ;
   char valid ;  
   int  tag   ;
   int 	last  ;
   int  *block ;
   } cache[S][E] ;

int *memory ;

int callno = 0 ;

int x  =     0 ;
int y  = 16384 ;
int z  = 32768 ;
int ni =    20 ;
int nj =    20 ;
 
void stats( char *t )
   { 
   int i,j,k,A ;

   for( i=0; i<S; i++ )
	for( j=0; j<E; j++ )
		{
		if( cache[i][j].valid & cache[i][j].dirty )
			{
			A = cache[i][j].tag*pow(2.0, m - T) + i*B; 
			for( k=0; k<B; k++ )
				memory[A+k] = cache[i][j].block[k] ;		
			dwrit = dwrit+1 ;
			}
 		cache[i][j].valid = 0 ;
 		cache[i][j].dirty = 0 ;
		}
	
   printf( "%8s y=%5d ni=%5d nj=%5d rh=%5d rm=%5d wh=%5d wm= %5d dw=%5d\n", t, y,ni,nj,
            rhits, rmiss, whits, wmiss, dwrit ) ;		
   rhits = 0 ; rmiss = 0 ;
   whits = 0 ; wmiss = 0 ;
   dwrit = 0 ;
   }

void initcache()
   {
		int i, j, k;
	
		memory = (int*)malloc(sizeof(int)*M);		//Dynamically allocate memory using malloc
		
		//Take the log_base2 of S, B, and M to get s, b, and m, respectively
		s = log((double)S)/log(2.0);
		b = log((double)B)/log(2.0);
		m = log((double)M)/log(2.0);
		
   printf( "S= %d E= %d B= %d T= %d M= %d s= %d b= %d m= %d\n", S,E,B,T,M,s,b,m ) ;

		for (i = 0; i < S; i++)
		{
			for (j = 0; j < E; j++)
			{
				cache[i][j].valid = 0;
				cache[i][j].dirty = 0;
				cache[i][j].tag = 0;
				cache[i][j].last = 0;
				cache[i][j].block = (int*)malloc(B * sizeof(int));
			}
		}

		for (k = 0; k < M; k++)
			memory[k] = k;
   }

void readwritecache( int readwrite, int a, int *value, int *hitmiss, int voice )
{
	    
// readwrite = READ (1) for read, WRITE (0) for write
// a = binary memory address ( 0 <= a < 65535 ) to read or write
// *value is memory value returned when readwrite = READ
//           memory value to store when readwrite = WRITE
// hitmiss is set to 0 for a miss, 1 for a hit, in either read or write case
// voice is a debugging switch

	int si, ta, bo, li = 0;
	int ba = 0;

//	si = stack, index
//	ta = tag
//	bo = block offset
//	li = line, index

		//Obtain si, ta, and bo using shifts and masking
		si = (a >> b) & ((1 << s) - 1);
		ta = (a >> (b + s)) & ((1 << T) - 1);
       	bo = a & ((1 << b) - 1);
		callno++;									//Increment callno so we can track of the LRU

		if( voice ) 
			printf( "%5d rw= %d a= %5d bo= %3d si= %3d ta= %3d", callno,readwrite,a,bo,si,ta ) ;

	//Set hitmiss to 1 whenever there is a hit, otherwise leave it as 0
	 *hitmiss = 0;
	 for (li = 0; li < E; li++)
	 {
	
		 if(cache[si][li].valid && cache[si][li].tag == ta)	//instruction is a hit
		 {
			 *hitmiss = 1;					//Set hitmiss value to 1 if there is a hit.
			 cache[si][li].last = callno;		//increment last each time there is a hit.
			 
			 if (readwrite)		//instruction is read
			 {
				*value = cache[si][li].block[bo];		//A read hit means we set the value equal to the block offset in the cache block
				 rhits++;
				 break;
			 }

			 else		//instruction is write
			{
				cache[si][li].block[bo] = *value;		//A write hit means that the cache block offset gets the value of the pointer called "value"
				cache[si][li].dirty = 1;
				whits++;
				break;
			}
		 }
	 }

	 if(*hitmiss == 0)		//If there is a miss
	 {
		 /*Key rules for a miss
		 Always set the valid bit to 1 afterwards
		 Always readjust the tag of the cache to the current value of "ta"
		 Assign the last data member to callno if we access that particular line index
		 Everytime we evict, we have to manually increment dwrit
		 A successful read miss eviction means that the bit is no longer dirty*/

		 int LRU = cache[si][0].last;		//First, assume that the minimum value is the first value
		 int minIndex = 0;					//First, assume that the minimum is stored at the first index

		 if (readwrite)
			 rmiss++;
		 else
			 wmiss++;

		for (li = 0; li < E; li++)		
		{
	
			if(cache[si][li].valid && cache[si][li].last < LRU)		//Update minimum value and index if the current element is less than LRU and valid
			{
					LRU = cache[si][li].last;
					minIndex = li;
			}

			if(readwrite)	//If there is a read miss
			{
				if (!cache[si][li].valid)	//if line in cache is invalid
				{
					//memcpy(dest, src, length)
					memcpy(cache[si][li].block, memory + (a - bo), B * sizeof(int));		//Move memory into the cache block
					*value = cache[si][li].block[bo];										//Set value equal to the cache block
					cache[si][li].last = callno;											
					cache[si][li].valid = 1;												
					cache[si][li].tag = ta;											
					break;
				}

				if (li == (E-1))		//Evict using the LRU policy if all lines are valid
				{
					cache[si][minIndex].last = callno;		//Adjust last data member
					if (cache[si][minIndex].dirty)
					{
						ba = (cache[si][minIndex].tag << (s + b)) + (si << b);				//get the starting address
						memcpy(memory + ba, cache[si][minIndex].block, B * sizeof(int));	//Store the value in the cache back into memory
						cache[si][minIndex].dirty = 0;										
						dwrit++;
					}

					memcpy(cache[si][minIndex].block, memory + (a - bo), B * sizeof(int));		//Copy memory value into the cache
					*value = cache[si][minIndex].block[bo];											//Value gets the value at cache block 
					cache[si][minIndex].valid = 1;													
					cache[si][minIndex].tag = ta;													
				}
			}

			else	//If there is a write miss
			{
				if(!cache[si][li].valid)
				{	
					cache[si][li].block[bo] = *value;		//Set block to value
					cache[si][li].last = callno;			//Adjust the last data member
					cache[si][li].valid = 1;				
					cache[si][li].dirty = 1;				
					cache[si][li].tag = ta;			
					break;
				}

				if(li == (E - 1))					//LRU value will be evicted if all the lines are valid
				{
					if(cache[si][minIndex].dirty)
					{
						ba = (cache[si][minIndex].tag << (s + b)) + (si << b);							//get the starting address
						memcpy(memory + ba, cache[si][minIndex].block, B * sizeof(int));				//Move the block into memory if dirty
						dwrit++;
					}

					memcpy(cache[si][minIndex].block, memory + (a - bo), B * sizeof(int));				//Move the memory into the cache
					cache[si][minIndex].block[bo] = *value;												//Set block to value
					cache[si][minIndex].tag = ta;														
					cache[si][minIndex].last = callno;													
				}
			}
		}
	 }

	 //dwrit is dirty write.

	  if( voice ) 
           printf( "li= %d", li ) ;
	  if( voice ) 
			printf( " %d %d %d\n", *value, cache[si][li].valid, cache[si][li].dirty ) ;
}

void locationexample()
{
   int i,j,k,hm, r;

   for( y=16374; y< 16395; y=y+1 )
    {
        for( i=0; i< nj;i++ )
		{
           readwritecache( READ,   x+i, &r, &hm, 0 ) ;
             readwritecache( WRITE,  y+i, &r, &hm, 0 ) ;                                                
		}
        stats( "loc copy" ) ;
   }
   
 //code for row wise transponse
	for (i = 0; i < ni; i++)
    {
	   for (j = 0; j < nj; j++)			
	   {
		  readwritecache(READ, y + nj*i + j, &r, &hm, 0);
		  readwritecache(WRITE, x + nj*j + i, &r, &hm, 0);
	   }

	    stats( "loc rows" ) ; 
   }
     

 //code for col wise transponse
   for (j = 0; j < nj; j++)
   {
	   for (i = 0; i < ni; i++)
	   {
		   readwritecache(READ, y + nj*i + j, &r, &hm, 0);
		   readwritecache(WRITE, x + nj*j + i, &r, &hm, 0); 
	   }
    
	   stats( "loc cols" ) ;  
   } 
      
}
   
void wsexample()
{
   int i,j,ii,jj,hm,r ;

   y = 20000 ;

   // code for row wise transpose
   for( ni=88; ni<128; ni=ni+8 )
   {
		nj = ni ;

		for (i = 0; i < ni; i++)
		{
			for (j = 0; j < nj; j++)			
			{
				readwritecache(READ, y + nj*i + j, &r, &hm, 0);
				readwritecache(WRITE, x + nj*j + i, &r, &hm, 0);
			}
		}
	  
      stats( "ws rows" ) ; 
   }

// code for row wise transpose with 8x blocking
   for( ni=88; ni<128; ni=ni+8 )
   {
		nj = ni ;

		for (ii = 0; ii < ni; ii = ii + 8)
			for (jj = 0; jj < nj; jj = jj + 8)
				for (i = ii; i < ii + 8; i++)
					for (j = jj; j < jj + 8; j++)
					{
						readwritecache(READ, y + nj*i + j, &r, &hm, 0);
						readwritecache(WRITE, x + nj*j + i, &r, &hm, 0);
					}
			   
		stats( "wsbrows" ) ; 
   }

// code for col wise transpose 
   for( ni=88; ni<128; ni=ni+8 )
   {
		nj = ni ;

		for (j = 0; j < nj; j++)
		{
			for (i = 0; i < ni; i++)
			{
				readwritecache(READ, y + nj*i + j, &r, &hm, 0);
				readwritecache(WRITE, x + nj*j + i, &r, &hm, 0); 
			}
		}
      
		stats( "ws cols" ) ;
    }

// code for col wise transpose with 8x blocking
   for( ni=88; ni<128; ni=ni+8 )
   {
      nj = ni ;

	  //When doing a col wise transpose, we only change the inner for loops. Changing the outer for loops yields a different result.
	  	for (ii = 0; ii < ni; ii = ii + 8)
			for (jj = 0; jj < nj; jj = jj + 8)
				for (j = jj; j < jj + 8; j++)	
					for (i = ii; i < ii + 8; i++)
					{
						readwritecache(READ, y + nj*i + j, &r, &hm, 0);
						readwritecache(WRITE, x + nj*j + i, &r, &hm, 0);
					}
    
			stats( "wsbcols" ) ;
      }
}

int main()
   {

		initcache() ;
		locationexample() ;
		wsexample() ;

		free(memory);
		return 0 ;
   }
