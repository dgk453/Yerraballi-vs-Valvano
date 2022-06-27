// Print.h
// This software has interface for printing
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 9/2/2019
// Last Modified:   9/2/2019
// Lab number: 7

#ifndef PRINT_H
#define PRINT_H
#include <stdint.h>

//-----------------------LCD_OutDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: none
// Output: none
void LCD_OutDec(uint32_t);

//-----------------------LCD_OutFix-----------------------
// Output a 32-bit number in unsigned fixed-point format
// resolution is 0.01
// Input: none
// Output: none
void LCD_OutFix(uint32_t);

#endif
