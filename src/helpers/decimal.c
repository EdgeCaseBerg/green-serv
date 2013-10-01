#include "helpers/decimal.h"

void createDecimal(long left, unsigned long right, Decimal * dec){
    /* There is SO much that can go wrong here. 
     * creating a decimal should be done from the createfromString function
    */
	dec->left = left;
	dec->right = right;
    dec->signBit = left < 0 ? NEGATIVE_ZERO : POSITIVE_ZERO;
}

void formatDecimal(const Decimal dec, char *  output){
    char * format;
    if(dec.left > -1 && dec.signBit == NEGATIVE_ZERO)/* -0.9 to -0.1*/
        format = "-%ld.%08lu";
    else
        format = "%ld.%08lu";
    sprintf(output, format, dec.left,dec.right);
}

/* Add two decimals */
void add_decimals(Decimal* a, Decimal* b, Decimal* sum){
    sum->left = a->left + b->left;
    sum->right = a->right + b->right;
    if(sum->right >= MANTISSA_LIMIT) {
        sum->left += 1;
        sum->right -= MANTISSA_LIMIT;
    }
    if(sum->left >0)
        sum->signBit = POSITIVE_ZERO;
}


/* Subtract two decimals, a - b */
void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff){
    diff->left = a->left - b->left;
    diff->right = a->right - b->right;
    if(diff->right > MANTISSA_LIMIT) { //it wll be around the limit for longs but it should never be lower than MANTISSA_LIMIT
        if(diff->left != 0)
            diff->left -= 1;            
        else
            diff->signBit = NEGATIVE_ZERO; /* Set flag here */
        diff->right = (ULONG_MAX+1) - diff->right;
    }  
    if(b->left >= a->left){
        diff->signBit = NEGATIVE_ZERO;
        diff->left = diff->left < -1 ? diff->left+1 : diff->left;
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
    int numDetected;

    if(str == NULL)
        return;

    bzero(rawLeft,9);
    bzero(rawRight,9);

    dotLocation = strstr(str, ".");
    leadingZeros = numDetected = 0;
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
            if(str[dotPos] == '0' && numDetected == 0)
                leadingZeros++;
            else
                numDetected = 1;
             
            rawRight[i] = str[dotPos];
        }
        rawRight[i] = '\0';
        right = strtoul(rawRight,NULL,10);
        if(leadingZeros > 0)
            /* subtract the leading zeros, then also the powers of ten taken by the number itself*/
            right = (right*(powlu(10,7-leadingZeros-(i-2))));
        else
            right = right*(powlu(10,(i > 1 ? 8-(i-1) : 7 ))); 
    }

    dec->left = left;
    dec->right = right;
    dec->signBit = left < 0 ? NEGATIVE_ZERO : POSITIVE_ZERO;
    if(str[0] == '-')
        dec->signBit = NEGATIVE_ZERO;

}