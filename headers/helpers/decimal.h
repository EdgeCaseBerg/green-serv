/*
    The Decimal structure is composed of two fields. The left and right.
    Which correspond to the mathematical concepts of characteristic and
    mantissa. The characteristic is easy, a simple signed field. The
    mantissa is more troublesome due to the need for leading zeros.
    
    A unsigned long value cannot store these leading zeroes as that makes
    absolutely no sense. So instead, consider the table:
    
    +-----------+---------------------+
    | Mantissa  | Long Representation |
    +-----------+---------------------+
    |.5         | 50,000,000          |
    |.05        | 5,000,000           |
    |.005       | 500,000             |
    |.0005      | 50,000              |
    |.00005     | 5,000               |
    |.000005    | 500                 |
    |.0000005   | 50                  |
    |.00000005  | 5                   |
    +-----------+---------------------+
    
    As one can see, the zeroes are essentially fliped around. The reason for
    representing the mantissa in this way is wholy based on the need for
    precision over the mantissa (Due to representing latitude and longitudes
    where the precision does very much matter) and also because when a
    Decimal structure is transformed for a database query, it is sprintf-ed
    with the 08%lu formatter for the mantissa. Because of this, the simple
    .5 MUST have the neccesary number of zeros on it's left side to format
    correctly.
*/

#ifndef __DECIMAL_H__	
    #define __DECIMAL_H__
    #define MANTISSA_LIMIT 100000000
	#include <limits.h> 
    
    
    #define NEGATIVE_ZERO 1
    #define POSITIVE_ZERO 0
	typedef struct{			 /* Calling them more convenient terms: */
		long left;  		 /* characteristic */
		unsigned long right; /* mantissa */
        int signBit;         /* handle behavior -0.1 to -0.9 */
	}Decimal;

	void createDecimal(long left, unsigned long right, Decimal * dec);

    /* Perform arithmetic operations on Decimal structures */
    void add_decimals(Decimal* a, Decimal* b, Decimal* sum); 
    void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff); 
 	#include <stdlib.h>
 	#include <stdio.h>
 	#include <string.h>
 	#include <math.h>
 	void createDecimalFromString(Decimal * dec, const char * str);
    void formatDecimal(const Decimal dec, char *  output);

#endif
