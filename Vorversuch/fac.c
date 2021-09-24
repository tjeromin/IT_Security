#include <stdio.h>
#include <stdlib.h>

// Returns the factorial of paramter n
unsigned long long int fac(int n)
{
    unsigned long long int acc = 1;
    for (int i = 2; i <= n; i++)
    {
        acc *= i;
    }
    return acc;
}

int main(int argc, char const *argv[])
{
    // Check program arguments for correct values
    if (argc < 2)
    {
        printf("No integer argument given!\n");
        return 0;
    }
    else if (atoi(argv[1]) <= 0)
    {
        printf("Argument must be an integer greater zero.\n");
        return 0;
    }
    else if (atoi(argv[1]) > 20)
    {
        printf("Argument is too big.\n");
        return 0;
    }

    // Calculate factorial of the argument
    // and prints the result to the console
    printf("%lld \n", fac(atoi(argv[1])));

    return 0;
}
