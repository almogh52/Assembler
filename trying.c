/*********************************
* Class: MAGSHIMIM C1			 *
* Week 13           			 *
* Question 5        			 *
**********************************/
#include <stdio.h>
#include <math.h>

#define MAX_SIZE 25

int main(void)
{
  unsigned int hex, mul, end;
  int i;

  printf("Enter the wanted hex: ");
  scanf("%x", &hex);

  for (i = 0; i < 0xFFFF; i++)
  {
    mul = i * i;
    end = (mul << 16) >> 16;
    end += i;

    if (end == hex)
    {
      printf("The number is %x", i);
      break;
    }
  }

  if (end != hex)
  {
    printf("Not found!\n");
  }

  return 0;
}
