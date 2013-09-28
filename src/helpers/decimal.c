#include "helpers/decimal.h"

void createDecimal(long left, unsigned long right, Decimal * dec){
	dec->left = left;
	dec->right = right;
}