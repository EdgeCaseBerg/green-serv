#include <stdio.h>
#include <stdlib.h>

#include "helpers/decimal.h"

int main(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;

	createDecimalFromString(&oper1, "22.5");
	createDecimalFromString(&oper2, "10.5");

	add_decimals(&oper1, &oper2, &result);
	printf("%ld.%lu\n", result.left, result.right);

	subtract_decimals(&oper1, &oper2, &result);
	printf("%ld.%lu\n", result.left, result.right);

	return 0;
}