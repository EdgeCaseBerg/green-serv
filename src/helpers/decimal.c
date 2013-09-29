#include "helpers/decimal.h"

void createDecimal(long left, unsigned long right, Decimal * dec){
	dec->left = left;
	dec->right = right;
}


/* Add two decimals */
void add_decimals(Decimal* a, Decimal* b, Decimal* sum){
    sum->left = a->left + b->left;
    sum->right = a->right + b->right;
    if(sum->right >= MANTISSA_LIMIT) {
        sum->left += 1;
        sum->right -= MANTISSA_LIMIT;
    }
}


/* Subtract two decimals, a - b */
void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff){
    diff->left = a->left - b->left;
    diff->right = a->right - b->right;
    if(diff->right >= MANTISSA_LIMIT) { //it wll be around the limit for longs but it should never be lower than MANTISSA_LIMIT
        diff->left -= 1;                //because neither operand can be more than 1 billion
        diff->right = ULONG_MAX - diff->right;
    }
}

