#ifndef __DECIMAL_H___
	#define __DECIMAL_H___
	#include "stdlib.h"
	#include "string.h"
	#include "stdio.h"
	#ifndef Decimal
		#define Decimal double
		#define DecimalWidth 16 /* The maximum width of the decimal */
		#define DecimalFormat "%3.08F"
		#define strtoDecimal strtod
		/* Wrapper functions. 
	 	* To provide your own, define the Decimal as whatever you'd like
	 	* before including this file and then make wrapper functions.
		*/
		void formatDecimal(Decimal dec, char * strBuffer);
		void add_decimals(Decimal* a, Decimal* b, Decimal* sum);
		void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff);
		Decimal createDecimalFromString(const char * str);
	#endif

#endif
