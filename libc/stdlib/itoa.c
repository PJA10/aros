#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void reverse_string(char str[], int length)
{
    int start = 0;
    int end = length -1; // don't reverse the last char - the '\0'
    while (start < end)
    {
	// swap(*(str+start), *(str+end));
	char tmp = *(str+start);
	*(str+start) = *(str+end);
	*(str+end) = tmp;
        start++;
        end--;
    }
}


char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
    unsigned posNum = (unsigned int) num;
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
     }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        posNum = (unsigned int) -num;
    }

    // Process individual digits
    while (posNum != 0)
    {
        int rem = posNum % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        posNum = posNum/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse_string(str, i);

    return str;
}
