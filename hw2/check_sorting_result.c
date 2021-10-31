#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main()
{
    FILE *result = fopen("result.txt", "r");
    int num1, num2;
    fscanf(result, "%d", &num1);
    while (fscanf(result, "%d", &num2) != EOF) {
        if (num1 > num2) {
            printf("Wrong!!!(%d > %d)\n", num1, num2);
        }
        num1 = num2;
    }

    fclose(result);
    return 0;
}
