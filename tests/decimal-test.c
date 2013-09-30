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

void traceAndPrintDecimal(Decimal testDec){
	int nptrs;
	void *buffer[100];
    char **strings;	
    nptrs = backtrace(buffer, 100);
    strings = backtrace_symbols(buffer, nptrs);
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

void addDecimals(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	createDecimalFromString(&oper1, "22.1");
	createDecimalFromString(&oper2, "10.2");
	add_decimals(&oper1, &oper2, &result);
	printf("Adding %ld.%08lu to %ld.%08lu\n",oper1.left,oper1.right,oper2.left,oper2.right);
	traceAndPrintDecimal(result);
}

void addDecimalsWithCarry(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	createDecimalFromString(&oper1, "22.5");
	createDecimalFromString(&oper2, "10.5");
	add_decimals(&oper1, &oper2, &result);
	printf("Adding %ld.%08lu to %ld.%08lu\n",oper1.left,oper1.right,oper2.left,oper2.right);
	traceAndPrintDecimal(result);
}

void subtractDecimals(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	createDecimalFromString(&oper1, "10.4");
	createDecimalFromString(&oper2, "10.2");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting %ld.%08lu from %ld.%08lu\n",oper2.left,oper2.right,oper1.left,oper1.right);
	traceAndPrintDecimal(result);
}

void subtractDecimalsWithCarry(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	createDecimalFromString(&oper1, "10.2");
	createDecimalFromString(&oper2, "10.4");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting %ld.%08lu from %ld.%08lu\n",oper2.left,oper2.right,oper1.left,oper1.right);
	traceAndPrintDecimal(result);
}

int main(){
	

	noDot();
	noMantissa();
	noLeading();
	leadingZero();
	multiDigitNum();
	largeMantissa();
	withExtraZeros();
	withExtraZerosAndLeading();
	addDecimals();
	addDecimalsWithCarry();
	subtractDecimals();
	subtractDecimalsWithCarry();
	return 0;
}