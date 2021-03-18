#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "api.h"

const unsigned ITER = 100;
const unsigned MAX_THREAD = 16;

void usage(const char *programname) {
    fprintf( stderr, "Usage: %s [parmset]\n", programname );
    fprintf( stderr, "Suported parmeter sets:\n"
                     " sha128s sha128r sha192s sha192r sha256s sha256r\n" 
                     " shake128s shake128r shake192s shake192r shake256s shake256r\n" 
                     " har128s har128r har192s har192r har256s har256r\n"  );
}

static int compare_float(const void *a, const void *b ) {
    const float *p = (const float *)a;
    const float *q = (const float *)b;
    if (*p > *q) return  1;
    if (*p < *q) return -1;
    return 0;
}

void run_test( sphincs_plus::key& k, const char *title ) {
    printf( "%s:\n", title );

    if (!k.generate_key_pair()) {
        printf( "*** Key generation failure\n" );
       	return;
    }

    unsigned sig_len = k.len_signature();
    unsigned char* sig = new unsigned char [sig_len];

    float results[MAX_THREAD+1][ITER];

    // Run the tests
    // We stagger the number of threads, to spread out the
    // effects of medium term changes in the CPU performance
    // (e.g. by load from other processes)
    for (unsigned i=0; i<ITER; i++) {
        for (unsigned thread=1; thread<=MAX_THREAD; thread++) {
            k.set_num_thread(thread);

            static unsigned char message[3] = { 'a', 'b', 'c' };
            struct timespec start, stop;

	    // Generate a signature and measure the
	    // wallclock time it took
            clock_gettime(CLOCK_REALTIME, &start);
            if (!k.sign( sig, sig_len, message, 3)) {
                printf( "*** Signature generation failure\n" );
	       	return;
            }
            clock_gettime(CLOCK_REALTIME, &stop);

            // Double check - make sure the signature verifies
	    // Not part of the performance test; instead, just
	    // a check to make sure we generated it properly
            if (!k.verify( sig, sig_len, message, 3 )) {
                printf( "Signature verify failure\n" );
		return;
            }

	    results[thread][i] = (stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;
        }
    }

    // Print out the results
    float x1_time = 1.0;
    for (unsigned thread=1; thread<=MAX_THREAD; thread++) {
	// Find the median time (not average; that might have
	// anomalous values should the process be task-switched)
	qsort( results[thread], ITER, sizeof(float), compare_float );
	float avg_time = results[thread][ ITER/2 ];  /* Median time */

	printf( "%d thread - average time = %f msec", thread, avg_time / 1000.0 );
	if (thread == 1) {
	    x1_time = avg_time;
	    printf( "\n" );
	} else {
	    printf( "  Speedup = %f\n", x1_time / avg_time );
	}
    }

    fflush(stdout);

    delete[] sig;
}

#define CONCAT( a, b ) a ## b
int main(void) {
#define TEST( PARM ) {         \
    CONCAT( sphincs_plus::key_, PARM ) key;  \
    run_test( key, #PARM );   \
}

    TEST( sha256_128f_simple )
    TEST( sha256_128f_robust )
    TEST( sha256_128s_simple )
    TEST( sha256_128s_robust )

    TEST( shake256_128f_simple )
    TEST( shake256_128f_robust )
    TEST( shake256_128s_simple )
    TEST( shake256_128s_robust )

    TEST( haraka_128f_simple )
    TEST( haraka_128f_robust )
    TEST( haraka_128s_simple )
    TEST( haraka_128s_robust )

    TEST( sha256_192f_simple )
    TEST( sha256_192f_robust )
    TEST( sha256_192s_simple )
    TEST( sha256_192s_robust )

    TEST( shake256_192f_simple )
    TEST( shake256_192f_robust )
    TEST( shake256_192s_simple )
    TEST( shake256_192s_robust )

    TEST( haraka_192f_simple )
    TEST( haraka_192f_robust )
    TEST( haraka_192s_simple )
    TEST( haraka_192s_robust )

    TEST( sha256_256f_simple )
    TEST( sha256_256f_robust )
    TEST( sha256_256s_simple )
    TEST( sha256_256s_robust )

    TEST( shake256_256f_simple )
    TEST( shake256_256f_robust )
    TEST( shake256_256s_simple )
    TEST( shake256_256s_robust )

    TEST( haraka_256f_simple )
    TEST( haraka_256f_robust )
    TEST( haraka_256s_simple )
    TEST( haraka_256s_robust )

    return 0;
}

