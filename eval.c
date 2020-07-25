/* Slightly modified from the accepted answer at

https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c
*/


static const char *expressionToParse;

char peek()
{
    return *expressionToParse;
}

char get()
{
    return *expressionToParse++;
}

int expression();

int number()
{
    int result = get() - '0';
    while (peek() >= '0' && peek() <= '9')
    {
        result = 10*result + get() - '0';
    }
    return result;
}

int factor()
{
    if (peek() >= '0' && peek() <= '9')
        return number();
    else if (peek() == '(')
    {
        int result;

        get(); /* '(' */
        result = expression();
        get(); /* ')' */
        return result;
    }
    else if (peek() == '-')
    {
        get();
        return -factor();
    }
    return 0; /* error */
}

int term()
{
    int result = factor();
    while (peek() == '*' || peek() == '/')
        if (get() == '*')
            result *= factor();
        else
            result /= factor();
    return result;
}

int expression()
{
    int result = term();
    while (peek() == '+' || peek() == '-')
        if (get() == '+')
            result += term();
        else
            result -= term();
    return result;
}

int eval_expression( const char *expr)
{
   expressionToParse = expr;
   return( expression( ));
}

#include <stdio.h>

int main( const int argc, const char **argv)
{
   printf( "%d\n", eval_expression( argv[1]));
   return( 0);
}
