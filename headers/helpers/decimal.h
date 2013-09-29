#ifndef __DECIMAL_H__	
	#include <limits.h> 
    #define MANTISSA_LIMIT 1000000000
    #define __DECIMAL_H__
	typedef struct{			 /* Calling them more convenient terms: */
		long left;  		 /* characteristic */
		unsigned long right; /* mantissa */
	}Decimal;

	void createDecimal(long left, unsigned long right, Decimal * dec);

    /* Perform arithmetic operations on Decimal structures */
    void add_decimals(Decimal* a, Decimal* b, Decimal* sum); 
    void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff); 
 
#endif
