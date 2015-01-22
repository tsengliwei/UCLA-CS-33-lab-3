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
                A = cache[i][j].tag*exp2(m-T)+i*B ;
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
    //
    // put code to calculate s, b, m here
    //
    s = log(S)/log(2); // 4
    b = log(B)/log(2); // 5
    m = log(M)/log(2); // 16
    
    memory = (int*)malloc(sizeof(int) * M);
    
    printf( "S= %d E= %d B= %d T= %d M= %d s= %d b= %d m= %d\n", S,E,B,T,M,s,b,m ) ;
    
    //
    // put code to initialize cache and memory here
    //
    
    // for cache
    int i, j;
    for (i = 0; i < S; i++) {
        for (j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].dirty = 0;
            cache[i][j].tag = 0;
            cache[i][j].last = 0;
            cache[i][j].block = (int*)malloc(B);
        }
    }
    
    // for memory
    for (j = 0; j < M; j++) {
        memory[j] = j;
    }
}

//void readwritecache( int readwrite, int a, int *value, int *hitmiss, int voice )
//{
//    
//    //
//    // readwrite = READ (1) for read, WRITE (0) for write
//    // a = binary memory address ( 0 <= a < 65535 ) to read or write
//    // *value is memory value returned when readwrite = READ
//    //           memory value to store when readwrite = WRITE
//    // hitmiss is set to 0 for a miss, 1 for a hit, in either read or write case
//    // voice is a debugging switch
//    //
//    //
//    //   compute si, ta, and bo from the address a
//    int si, ta, bo, li = 0 ;
//    //	si = stack, index
//    //	ta = tag
//    //	bo = block offset
//    //      increment callno
//    //
//    int t = m - (s + b);
//    
//    si = (a >> (m-T-s)) % S;
//    ta = a >> (m-T);
//    bo = a % B; // a & 0x1f to get bo when b = 5. 0x1f == (1 << b) - 1
//    callno++;
//    if( voice )
//        printf( "%5d rw= %d a= %5d bo= %3d si= %3d ta= %3d", callno,readwrite,a,bo,si,ta ) ;
//    
//    //
//    //   check each line of the set:
//    //	if( cache[si][line#].valid && cache[si][line#].tag = ta )
//    //	to find a hit
//    //
//    //   if no hit, choose a line (not valid or LRU )
//    //
//    //   if chosen line dirty, copy to memory
//    //
//    *hitmiss = 0;
//    
//    
//    int ba = 0; // base index for memory
//    
//    // check for hit////
//    *hitmiss = 0;
//    for (li = 0; li < E; li++) {
//        if (cache[si][li].valid && cache[si][li].tag == ta) {
//            *hitmiss = 1;
//            cache[si][li].last = callno;
//            if (readwrite == READ) {
//                *value = cache[si][li].block[bo];
//                rhits++;
//                break;
//            }
//            else
//            {
//                cache[si][li].block[bo] = *value;
//                cache[si][li].dirty = 1;
//                whits++;
//                break;
//            }
//        }
//    }
//    
//    // check for hit end ///
//    
//    int j, k;
//    
//    /// deal with miss ///
//    
//    if(*hitmiss == 0)
//    {
//        int min = cache[si][0].last;
//        int minIndex = 0;
//        for (li = 0; li < E; li++) {
//            if (cache[si][li].last < min) {
//                // find LRU
//                minIndex = li;
//                min = cache[si][li].last;
//            }
//            if (readwrite == READ) {
//                rmiss++;
//        
//                if (!cache[si][li].valid) {
//                    // replace invalid
//                    //memcpy(cache[si][li].block, memory + a - bo, B * sizeof(int)); // fetch from memory to cache
//                    for(j = (a - a%B), k = 0; j < (a - a%B + B); j++, k++)
//                    {
//                        cache[si][li].block[k] = memory[j];
//                        cache[si][li].valid = 1;
//                        cache[si][li].dirty = 0;
//                        cache[si][li].tag = ta;
//                    }
//
//                    *value = cache[si][li].block[bo];
//                    cache[si][li].last = callno; // read the value
//                    //cache[si][li].valid = 1;
//                    break;
//                }
//                if (li == E - 1) {
//                    // if all are valid, check LRU
//                    if (cache[si][minIndex].dirty == 1)
//                    {
//                        // update cache and memory if the line is dirty
//                        ba = (cache[si][minIndex].tag << (s + b)) + (si << b);
//                        //memcpy(memory+ba, cache[si][minIndex].block, B * sizeof(int)); // write back from cache to memory
//                        for(j = (((cache[si][li].tag)<<(b+s)) | si<<b |0x0000 ), k =0; k < B; j++,k++)
//                        {
//                            memory[j]= cache[si][li].block[k];
//                        }
//                        cache[si][minIndex].dirty = 0;
//                    }
//                    cache[si][minIndex].tag = ta;
//                   // memcpy(cache[si][minIndex].block, memory + a - (a % b), B * sizeof(int)); // fetch from memory to cache
//                    for(j = (a - a%B), k = 0; j < (a - a%B + B); j++, k++)
//                    {
//                        cache[si][li].block[k] = memory[j];
//                        cache[si][li].valid = 1;
//                        cache[si][li].dirty = 0;
//                        cache[si][li].tag = ta;
//                    }
//
//                    *value = cache[si][minIndex].block[bo];
//                    cache[si][minIndex].last = callno;
//                }
//            }
//            else{
//                wmiss++;
//                
//                if (!cache[si][li].valid) {
//                    // replace invalid
////                    memcpy(cache[si][li].block, memory + a - (a % b), B * sizeof(int)); // fetch from memory to cache
//                    cache[si][li].block[bo] = *value;
//                    cache[si][li].last = callno; // read the value
//                    cache[si][li].valid = 1; // update valid bit
//                    cache[si][li].dirty = 1;
//                    break;
//                }
//                if (li == E - 1) {
//                    // if all are valid, check LRU
//                    if (cache[si][minIndex].dirty == 1)
//                    {
//                        // update cache and memory if the line is dirty
//                        ba = (cache[si][minIndex].tag << (s + b)) + (si << b);
//                        //memcpy(memory+ba, cache[si][minIndex].block, B * sizeof(int)); // write back from cache to memory
//                        for(j = (((cache[si][li].tag)<<(b+s)) | si<<b |0x0000 ), k =0; k < B; j++,k++)
//                        {
//                            memory[j]= cache[si][li].block[k];
//                        }
//                        cache[si][minIndex].dirty = 1;
//                    }
//                    cache[si][minIndex].tag = ta;
//                    //memcpy(cache[si][minIndex].block, memory + a - bo, B * sizeof(int)); // fetch from memory to cache
//                    for(j = (a - a%B), k = 0; j < (a - a%B + B); j++, k++)
//                    {
//                        cache[si][li].block[k] = memory[j];
//                        cache[si][li].valid = 1;
//                        cache[si][li].dirty = 0;
//                        cache[si][li].tag = ta;
//                    }
//
//                    cache[si][minIndex].block[bo] = *value;
//                    cache[si][minIndex].last = callno;
//                    cache[si][minIndex].dirty = 1;
//                }
//
//            }
//        }
//    }
//    
//    
//
//    if( voice )
//        printf( "li= %d", li ) ;
//    //
//    //    copy from memory to chosen line
//    //
//    //    if write, copy value into line, set dirty
//    //
//    //    else copy value from line, not dirty
//    //
//    //    set last for line
//    //
//    if( voice )
//        printf( " %d %d %d\n", *value, cache[si][li].valid, cache[si][li].dirty ) ;
//}

void readwritecache( int readwrite, int a, int *value, int *hitmiss, int voice )
{
    
    //
    // readwrite = READ (1) for read, WRITE (0) for write
    // a = binary memory address ( 0 <= a < 65535 ) to read or write
    // *value is memory value returned when readwrite = READ
    //           memory value to store when readwrite = WRITE
    // hitmiss is set to 0 for a miss, 1 for a hit, in either read or write case
    // voice is a debugging switch
    //
    //
    //   compute si, ta, and bo from the address a
    int si, ta, bo, li = -100;
    si = (a >>(m-T-s)) % S;
    ta = a >>(m-T);
    bo  = a % B;
    callno++;
    //	si = stack, index
    //	ta = tag
    //	bo = block offset
    //      increment callno
    //
    if( voice )
        printf( "%5d rw= %d a= %5d bo= %3d si= %3d ta= %3d", callno,readwrite,a,bo,si,ta ) ;
    int i, j, k;
    
    
    ////check for hit//////
    *hitmiss = 0;
    for(i = 0; i < E; i++)
    {
        if(cache[si][i].valid && cache[si][i].tag == ta)
        {
            *hitmiss = 1;
            li  = i;
            if(readwrite == READ)
                rhits++;
            else
                whits++;
            break;
        }
        
    }
    //// check for hit end///
    
    //// check for miss////
    if(*hitmiss == 0)
    {
        if(readwrite == READ)
            rmiss++;
        else
            wmiss++;
        
        for(i = 0; i < E; i++)
        {
            if(!cache[si][i].valid )
            {
                // check for invalid bits
                li = i;
                break;
            }
        }
        
        if(li == -100)
        {
            // if all are valid, check LRU
            int LRU = cache[si][0].last;
            li = 0;
            for(i = 1; i < E; i++)
            {
                if(cache[si][i].last < LRU)
                {
                    li = i;
                    LRU = cache[si][i].last;
                }
            }
        }
    }
    
    if(cache[si][li].valid && cache[si][li].dirty && !(*hitmiss))
    {
        // write from cache to memory if there's a miss and a dirty bit
        for(j = (((cache[si][li].tag)<<(b+s)) | si<<b |0x0000 ), k =0; k < B; j++,k++)
        {
            memory[j]= cache[si][li].block[k];
        }
       dwrit++;
    }
    
    
    //
    //   check each line of the set:
    //	if( cache[si][line#].valid && cache[si][line#].tag = ta )
    //	to find a hit
    //
    //   if no hit, choose a line (not valid or LRU )
    //
    //   if chosen line dirty, copy to memory
    //
    if( voice )
        printf( "li= %d", li );
    
    if(!(*hitmiss))
    {
        for(j = (a - a%B), k = 0; j < (a - a%B + B); j++, k++)
        {
            // fetch contents from memory to cache
            cache[si][li].block[k] = memory[j];
            cache[si][li].valid = 1;
            cache[si][li].dirty = 0;
            cache[si][li].tag = ta;
        }
    }
    if (readwrite == WRITE )
    {
        cache[si][li].block[bo] = *value;
        cache[si][li].dirty = 1;
    }
    else
        *value = cache[si][li].block[bo];

    cache[si][li].last = callno;
    
    //
    //    copy from memory to chosen line
    //
    //    if write, copy value into line, set dirty
    //
    //    else copy value from line, not dirty
    //
    //    set last for line
    //
    if( voice )
        printf( " %d %d  %d %d %d\n", *value, cache[si][li].valid, cache[si][li].tag, cache[si][li].dirty, *hitmiss ) ;
}

void locationexample()
{
    int i,j,k,hm, r;
    
    for( y=16374; y< 16395; y=y+1 )
    {
        for( i=0; i< nj;i++ )
        {
            readwritecache( READ,   x+i, &r, &hm, 0 );
            readwritecache( WRITE,  y+i, &r, &hm, 0 );
        }
        stats( "loc copy" ) ;

    }
    
//    
    //
    // code for row wise transponse
    //

        //
        // code for row wise transpose
        //
    for( y=16374; y< 16395; y=y+1 )
    {
        for (i = 0; i < ni; i++) // access y row wise
        {
            for (j=0; j<nj; j++)
            {
                readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
            }
        }
        stats( "ws rows" ) ;
    }
        
    
    

   
    
    //
    // code for col wise transponse
    //

        //
        // code for col wise transpose
        //
    for( y=16374; y< 16395; y=y+1 )
    {
        for (j = 0; j < nj; j++) // access y column wise
        {
            for (i=0; i <ni; i++)
            {
                readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
            }
        }
        stats( "loc cols" ) ;
    }

    
    
}

void wsexample()
{
    int i,j,ii,jj,hm,r ;
    
    y = 20000 ;
    for( ni=88; ni<128; ni=ni+8 )
    {
        nj = ni ;
        //
        // code for row wise transpose
        //
        for (i = 0; i < ni; i++) // access y row wise
        {
            for (j=0; j<nj; j++)
            {
                readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
            }
        }


        stats( "ws rows" ) ;
    }
    
    for( ni=88; ni<128; ni=ni+8 )
    {
        nj = ni ;
        //
        // code for row wise transpose with 8x blocking
        //
       for( ii=0; ii<ni; ii=ii+8 )
           for( jj=0; jj<nj; jj=jj+8 )
               for( i=ii; i<ii+8; i++ )
                   for( j=jj; j<jj+8; j++ )
                   {
                       {
                           readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                           readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
                       }

                   }
                    
        stats( "wsbrows" ) ;
    }
    
    for( ni=88; ni<128; ni=ni+8 )
    {
        nj = ni ;
        //
        // code for col wise transpose 
        //
        for (j = 0; j < nj; j++) // access y column wise
        {
            for (i=0; i <ni; i++)
            {
                readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
            }
        }

        stats( "ws cols" ) ;
    }
    
    for( ni=88; ni<128; ni=ni+8 )
    {
        nj = ni ;
        //
        // code for col wise transpose with 8x blocking
        //
        for( jj=0; jj<nj; jj=jj+8 )
            for( ii=0; ii<ni; ii=ii+8 )
                for( j=jj; j<jj+8; j++ )
                    for( i=ii; i<ii+8; i++ )
                    {
                        {
                            readwritecache(READ, y + nj * j + i, &r, &hm, 0);
                            readwritecache(WRITE, x + nj * i + j, &r, &hm, 0);
                        }
                        
                    }
        stats( "wsbcols" ) ;
    }
    
}

/*
Hit
cache[i][*]
 write cache[si][*].block[bo] = *value;
 cache[si][*].dirty = 1
 *value = cache[si][*].block[bo];
 
Miss
ID si 
 for each li E cache[si][li]
    if valid = 0
    move block into si, li
    break
 All valid, select LRU
    lru = li min(cache[si][li].last)
 if lru dirty
    move lru block to memory, dirty = 0
    move block in cache, memcpy(M->cache)
    valid = 1

*/


/*
    for(i: 0... nj-1
    for(j:0 ... nj-1)
 
    x[i][j] = y[j][i]
    *(x + nji + j) = *(y + nij + i)
 
 
 
 for(j:0 ... nj-1)
 for(i: 0... nj-1
 
 x[i][j] = y[j][i]
 *(x + nji + j) = *(y + nij + i)

 
 
 for(jj = 0; jj < n; jj+=8)
 for(ii = 0; ii < ni; ii+= 8)
 for(j:0 ... nj-1)
 for(i: 0... nj-1
 
 x[i][j] = y[j][i]
 *(x + nji + j) = *(y + nij + i)

 
*/

int main()
{
    
    initcache() ;
    locationexample() ;
    wsexample() ;
    
    return 0 ;
}