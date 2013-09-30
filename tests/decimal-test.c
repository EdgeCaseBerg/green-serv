#include <stdio.h>
#include <stdlib.h>

#include "helpers/decimal.h"

int main(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;

	createDecimalFromString(&oper1, "22.05");
	/* Verify that the floating point is correct */
	printf("%ld.%08lu\n", oper1.left, oper1.right);

	createDecimalFromString(&oper2, "10.06");

	add_decimals(&oper1, &oper2, &result);
	printf("%ld.%08lu\n", result.left, result.right);

	subtract_decimals(&oper1, &oper2, &result);
	printf("%ld.%08lu\n", result.left, result.right);

	return 0;
}