#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>

#include "helpers/decimal.h"

void testAndPrint(const char * decimalString){
	int nptrs;
	void *buffer[100];
	char decBuff[DecimalWidth];
    char **strings;
    bzero(decBuff,DecimalWidth);

    nptrs = backtrace(buffer, 100);
    strings = backtrace_symbols(buffer, nptrs);
	Decimal testDec;
	testDec = createDecimalFromString(decimalString);
	formatDecimal(testDec, decBuff);
	printf("Test: %s  Decimal: %s\n", strings[1], decBuff);

	free(strings);
}

void traceAndPrintDecimal(Decimal testDec){
	int nptrs;
	char decBuff[DecimalWidth];
	void *buffer[100];
    char **strings;	
    bzero(decBuff, sizeof decBuff);
    
    nptrs = backtrace(buffer, 100);
    strings = backtrace_symbols(buffer, nptrs);
    formatDecimal(testDec, decBuff);
    printf("Test: %s  Decimal: %s\n", strings[1], decBuff);

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
	testAndPrint("-22.01");
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
	oper1 = createDecimalFromString( "22.1");
	oper2 = createDecimalFromString( "10.2");
	add_decimals(&oper1, &oper2, &result);
	printf("Adding " DecimalFormat " to " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);
}

void addDecimalsWithCarry(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	oper1 = createDecimalFromString( "22.5");
	oper2 = createDecimalFromString( "10.5");
	add_decimals(&oper1, &oper2, &result);
	printf("Adding " DecimalFormat " to " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);
}

void subtractDecimals(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	oper1 = createDecimalFromString( "10.04");
	oper2 = createDecimalFromString( "10.02");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting " DecimalFormat  " from " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);
}

void subtractDecimalsWithCarry(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	oper1 = createDecimalFromString( "10.0");
	oper2 = createDecimalFromString( "10.1");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting " DecimalFormat  " from " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);
}

void subtractPositiveFromNegative(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	oper1 = createDecimalFromString( "-40.0");
	oper2 = createDecimalFromString( "1.0");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting " DecimalFormat  " from " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);	
}

void subtractNegativeFromNegative(){
	Decimal oper1;
	Decimal oper2;
	Decimal result;
	oper1 = createDecimalFromString( "-40.0");
	oper2 = createDecimalFromString( "-1.0");
	subtract_decimals(&oper1, &oper2, &result);
	printf("Subtracting " DecimalFormat  " from " DecimalFormat "\n",oper1, oper2);
	traceAndPrintDecimal(result);	
}

void negativeZeroValue(){
	Decimal testDec;
	testDec = createDecimalFromString( "-0.1");
	traceAndPrintDecimal(testDec);
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
	negativeZeroValue();
	subtractPositiveFromNegative();
	subtractNegativeFromNegative();
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}