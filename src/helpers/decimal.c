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

static long powlu(long base, long raisemeto){
    int i;
    long result;
    if(raisemeto <= 0) /* I aint dealing with negative*/
        return 1L;
    result = base;
    
    for(i=1; i < raisemeto; ++i){
        result = result*base;
    }
    return result;
}

/*
Be sure to read decimal.h for a good explanation of why we do what we do
*/
void createDecimalFromString(Decimal * dec, const char * str){
    long left;
    unsigned long right;
    char * dotLocation;
    char rawLeft[9];
    char rawRight[9];
    int i;
    int dotPos;
    long leadingZeros;

    if(str == NULL)
        return;

    bzero(rawLeft,9);
    bzero(rawRight,9);

    dotLocation = strstr(str, ".");
    leadingZeros = 0;
    if(dotLocation == NULL){
        left = atol(str);
        right = 0;
    }else{
        /* ghetto strncpy */
        for(i=0; i != 9 && str[i] != *dotLocation; ++i)
            rawLeft[i] = str[i];
        rawLeft[i] = '\0';
        dotPos = i+1;
        left = atol(rawLeft);
        for(i=0; i != 9 && str[dotPos] != '\0'; ++i,++dotPos){
            if(str[dotPos] == '0')
                leadingZeros++;
            rawRight[i] = str[dotPos];
        }
        rawRight[i] = '\0';
        right = strtoul(rawRight,NULL,10);
        if(leadingZeros > 0)
            right = (right*(powlu(10,7-leadingZeros)));
        else
            right = right*10000000;
    }

    dec->left = left;
    dec->right = right;

}