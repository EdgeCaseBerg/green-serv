#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>

#include "helpers/decimal.h"

void testAndPrint(const char * decimalString){
	int nptrs;
	void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, 100);
    strings = backtrace_symbols(buffer, nptrs);
	Decimal testDec;
	createDecimalFromString(&testDec, decimalString);
	printf("Test: %s  Decimal: %ld.%08lu\n", strings[1], testDec.left, testDec.right);

	free(strings);
}

void noLeading(){
	testAndPrint("22.1");
}

void noMantissa(){
	testAndPrint("22.");
}

void noDot(){
	testAndPrint("22");
}

void leadingZero(){
	testAndPrint("22.01");
}

void multiDigitNum(){
	testAndPrint("22.0156");	
}

void largeMantissa(){
	testAndPrint("22.123456789");
}

void withExtraZeros(){
	testAndPrint("22.1000");		
}

void withExtraZerosAndLeading(){
	testAndPrint("22.015600");	
}

int main(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;

	noDot();
	noMantissa();
	noLeading();
	leadingZero();
	multiDigitNum();
	largeMantissa();
	withExtraZeros();
	withExtraZerosAndLeading();

	createDecimalFromString(&oper1, "22.0550");
	printf("%ld.%08lu\n", oper1.left, oper1.right);

	createDecimalFromString(&oper2, "10.06");
	printf("%ld.%08lu\n", oper2.left, oper2.right);

	add_decimals(&oper1, &oper2, &result);
	printf("%ld.%08lu\n", result.left, result.right);

	subtract_decimals(&oper1, &oper2, &result);
	printf("%ld.%08lu\n", result.left, result.right);

	return 0;
}