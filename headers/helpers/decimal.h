#ifndef __DECIMAL_H__	
	#define __DECIMAL_H__
	typedef struct{			 /* Calling them more convenient terms: */
		long left;  		 /* characteristic */
		unsigned long right; /* mantissa */
	}Decimal;

	void createDecimal(long left, unsigned long right, Decimal * dec);
#endif