#include <stdio.h>

int add(int a, int b)
{
    return a+b;
}

int multiply(int a, int b)
{
    return a*b;
}

int substract(int a, int b)
{
    return a-b;
}

void print_result(int result)
{
    printf("Result = %d\n", result);
}

/*
    Passing two function pointers to this function.
*/
void calculate_and_display(int a, 
    int b, 
    int(*operation)(int, int),
    void(*display)(int))
{
    int result = operation(a, b);
    display(result);
}

int main(void)
{

    calculate_and_display(10, 20, add, print_result);
    calculate_and_display(10, 20, multiply, print_result);
    calculate_and_display(10, 20, substract, print_result);

    return 0;
}