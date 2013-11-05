#include "helpers/decimal.h"
 
    
/*Expects strBuffer to be of at least 16 or stack smashing may occur */
void formatDecimal(Decimal dec, char * strBuffer){
    snprintf(strBuffer,DecimalWidth, DecimalFormat, dec);
}

/* Add two decimals */
void add_decimals(Decimal* a, Decimal* b, Decimal* sum){
    (*sum) = (*a) + (*b);
}


/* Subtract two decimals, a - b */
void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff){
    (*diff) = (*a) - (*b);
}

/*
Be sure to read decimal.h for a good explanation of why we do what we do
*/
Decimal createDecimalFromString(const char * str){
    return strtoDecimal(str,NULL);
}