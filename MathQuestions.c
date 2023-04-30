#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

typedef enum _Operator
{
    DEAD = -2, // math arg already used
    LONE = -1, // the last arg that doesn't have an operator
    PLUS = 0,
    MINUS,
    MULTIPLY,
    DIVIDE
} Operator;

typedef struct _Fraction
{
    int Numerator;
    int Denominator;
} Fraction;

typedef struct _ValueAndOp
{
    Fraction Value;
    Operator Op;
} ValueAndOp;

// 89 + 34 * 941 * 243 =
// 4 'args', each arg is a number + an operator type
// last operator is defined 'LONE'.
// each operator defines the type of operation that happens between current value and next arg's value
// so above would be:
// arg 0: 89 and +
// arg 1: 34 and *
// arg 2: 941 and *
// arg 3: 243 and LONE
// 
// When you find the highest priority operator, at arg X, you "figure it out" between X and X+1
// and then operator at X becomes X+1's operator, and X+1's operator becomes 'DEAD' (don't reference it)
// When you process the args 3 times, the one operator that isn't 'DEAD' is the one that contains the answer!
// For instance, 1st time through above, you pick "34 * 941"
// arg1's value becomes the value of 34 * 941, and it's operator becomes 941's * operator
// then arg2's value we can set to 0 (don't have to) and it's operator to be 'DEAD'
// Now, it looks like this:
// arg 0: 89 and +
// arg 1: 31994 and *
// arg 2: 0 and DEAD
// arg 3: 243 and LONE
// ....
// Now we pick the 2nd * operator, and we know it operates between 1 and 3
// So arg1's value is the result of 31994 * 243, and it gets operator 'LONE'
// and arg3's value becomes 0 and it's operator is 'DEAD'
// Now we have:
// arg 0: 89 and +
// arg 1: 7774542 and LONE
// arg 2: 0 and DEAD
// arg 3: 0 and DEAD
// ...
// Now there is only 1 operation possible, between 0 and 1!
// arg0's value becomes 89 + 7774542, and operator is 'LONE'
// arg1's value becomes 0 and it's operator is 'DEAD'
// the only thing left with operator 'LONE' is the answer.

const char* GetMathOpString(Operator op) // 89 + 34 * 941 * 243 
{
    switch (op)
    {
    case DEAD:
        return " DEAD "; // should not happen
    case LONE:
        return ""; // don't print an operator
    case PLUS:
        return " + ";
    case MINUS:
        return " - ";
    case MULTIPLY:
        return " X ";
    case DIVIDE:
        return " / ";
    default:
        return " ? "; // should not happen
    }
}

bool IsZero(Fraction A)
{
    return A.Numerator == 0 ? true : false;
}

bool IsInvalid(Fraction B)
{
    return B.Denominator == 0 ? true : false;
}

void PrintValue(Fraction n)
{
    if (n.Denominator == 0)
    {
        printf("[INF]");
        return;
    }

    // modulo by negative number is undefined

    assert(n.Denominator > 0);

    int Whole = n.Numerator / n.Denominator;
    int Remainder = n.Numerator % n.Denominator;
    if (Remainder == 0)
    {
        printf("%d", Whole);
        return;
    }

    // print in fractional form

    int Denominator = n.Denominator;

    if (Whole != 0)
    {
        // print the whole number too
        printf("[%d R %d/%d]", Whole, abs(Remainder), abs(Denominator));
    }
    else
    {
        // print just the fractional part
        printf("[R %d/%d]", Remainder, Denominator);
    }

}

void PrintExpression(ValueAndOp* pMathArgs, int Count)
{
    // print the entire equation

    for (int i = 0; i < Count; i++)
    {
        if (pMathArgs[i].Op != DEAD)
        {
            PrintValue(pMathArgs[i].Value);
            printf(" %s", GetMathOpString(pMathArgs[i].Op));
        }
    }
    printf("\n");
}

// flip both top and bottom if the denominator is negative. This
// works for 0 on top too, since -0 is equal to 0

void EnsureDenominatorIsPositive(Fraction* pNum, bool bPrint)
{
    if (pNum->Denominator < 0)
    {
        if (bPrint)
        {
            printf("Flipping sign for numerator for %d over %d\n", pNum->Numerator, pNum->Denominator);
        }
        pNum->Denominator *= -1;
        pNum->Numerator *= -1;
    }
}

// simplify this number to be the simplest fraction it can be.

void Simplify(Fraction* pNum, bool bPrint)
{
    EnsureDenominatorIsPositive(pNum, bPrint);

    if (pNum->Denominator == 1)
    {
        return;
    }

    bool simplifiedSomething = false;

    // find any # that is evenly divisible in the numerator and the denominator

    for (int i = 2; i < pNum->Denominator / 2; i++)
    {
        // this modulo, even with - numbers, works for finding if divisible by i

        int numerator_remainder = pNum->Numerator % i;
        int denominator_remainder = pNum->Denominator % i;

        if (numerator_remainder == 0 && denominator_remainder == 0)
        {
            // when we find a number both the top and bottom are dividable by,
            // divide both sides and start from the beginning. this is because it could be dividable by
            // the same number ( for example 2 ), more than once.

            if (bPrint)
            {
                printf("Dividing both numerator and denominator by %d\n", i);
            }
            pNum->Numerator /= i;
            pNum->Denominator /= i;
            i = 1; // start at beginning of loop. don't forget i gets incremented when it loops. setting i = 1 results in i=2 next time through the loop
            simplifiedSomething = true;
        }
    }

    if (bPrint && simplifiedSomething)
    {
        printf("Final simplification of value is :");
        PrintValue(*pNum);
        printf("\n");
    }
}

// only called for add. Don't call when eiter denominator is 0

void ConvertToCommonDenominator(Fraction* pA, Fraction* pB, bool bPrint) // 9/5 + 10/6  54/30 + 50/30 104/30 = 3 r 14/30
{
    // don't let the denominator be negative... make the numerator negative instead
    // this works even if the numerator is 0. 
    // This allows us to easily find a common denominator without worrying about sign
    EnsureDenominatorIsPositive(pA, bPrint);
    EnsureDenominatorIsPositive(pB, bPrint);

    int CommonDenominator = pA->Denominator * pB->Denominator;
    int ScaleA = CommonDenominator / pA->Denominator;
    pA->Numerator *= ScaleA;
    pA->Denominator *= ScaleA;
    int ScaleB = CommonDenominator / pB->Denominator;
    pB->Numerator *= ScaleB;
    pB->Denominator *= ScaleB;
}

Fraction AddTwoNumbers(Fraction A, Fraction B, bool bPrint)
{
    if (IsInvalid(A) || IsInvalid(B))
    {
        // anything plus invalid is invalid. We'll just return a generic invalid # as the result. If we try to use this in any other ops, they'll be invalid too
        Fraction invalid;
        invalid.Numerator = 1;
        invalid.Denominator = 0;
        return invalid;
    }
    if (IsZero(A)) return B;
    if (IsZero(B)) return A;

    ConvertToCommonDenominator(&A, &B, bPrint);
    Fraction Result;
    Result.Numerator = A.Numerator + B.Numerator;
    Result.Denominator = A.Denominator;
    Simplify(&Result, bPrint);
    return Result;
}

Fraction SubTwoNumbers(Fraction A, Fraction B, bool bPrint)
{
    // subtraction is simply addition of the -. Only negate the numerator, not the denominator.
    // it's okay if it's 0, since -0 is equal to 0

    Fraction NegativeB = B;
    NegativeB.Numerator = -NegativeB.Numerator;
    return AddTwoNumbers(A, NegativeB, bPrint);
}

Fraction MultiplyTwoNumbers(Fraction A, Fraction B, bool bPrint)
{
    if (IsInvalid(A) || IsInvalid(B))
    {
        // anything multiplied by invalid is invalid. 
        // We'll just return a generic invalid # as the result. If we try to use this in any other ops, they'll be invalid too
        Fraction invalid;
        invalid.Numerator = 1;
        invalid.Denominator = 0;
        return invalid;
    }

    // this works even if one is infinite or one is zero.
    // A/B * C/D if infinity will work out to be (A*C) / (B*D). If one is infinite, the result will be infinite, which is true.
    // Could end up being 0/0 which again, is technically infinite?
    Fraction result;
    result.Numerator = A.Numerator * B.Numerator;
    result.Denominator = A.Denominator * B.Denominator;

    // if the denominator becomes 0, then we're invalid from now on. The other math ops should realize that

    Simplify(&result, bPrint);
    return result;
}

Fraction DivideTwoNumbers(Fraction A, Fraction B, bool bPrint)
{
    if (IsInvalid(A) || IsInvalid(B))
    {
        // anything multiplied by invalid is invalid. 
        // We'll just return a generic invalid # as the result. If we try to use this in any other ops, they'll be invalid too
        Fraction invalid;
        invalid.Numerator = 1;
        invalid.Denominator = 0;
        return invalid;
    }

    Fraction result;
    result.Numerator = A.Numerator * B.Denominator;
    result.Denominator = A.Denominator * B.Numerator;
    Simplify(&result, bPrint);
    return result;
}

void GetTwoMathOpIndexes(ValueAndOp* pMathArgs, int Count, int* pLeftIndex, int* pRightIndex)
{
    // find the highest priority math operation. within the same operator, priority is left to right
    // the operation for the last arg in the list is always NONE, so we can ignore it

    Operator HighestOperation = DEAD;
    int BestLeftArgIndex = 0;
    for (int i = 0; i < Count - 1 ; i++)
    {
        if (pMathArgs[i].Op == DEAD)
            continue;
        if (pMathArgs[i].Op > HighestOperation)
        {
            HighestOperation = pMathArgs[i].Op;
            BestLeftArgIndex = i;
        }
    }

    // find any non-DEAD (even NONE works) operator to the right of us

    int BestRightArgIndex = 0;
    for (int RightArgIndex = BestLeftArgIndex + 1 ; RightArgIndex < Count ; RightArgIndex++)
    {
        if (pMathArgs[RightArgIndex].Op != DEAD)
        {
            BestRightArgIndex = RightArgIndex;
            break;
        }
    }

    assert(BestRightArgIndex != 0);

    // return them

    *pLeftIndex = BestLeftArgIndex;
    *pRightIndex = BestRightArgIndex;
}

int DoHighestPriMathArg(ValueAndOp* pMathArgs, int Count, bool bPrint)
{
    // find two things to operate upon

    int LeftIndex, RightIndex;
    GetTwoMathOpIndexes(pMathArgs, Count, &LeftIndex, &RightIndex);

    // switch on the operator and do the operation

    Operator op = pMathArgs[LeftIndex].Op;

    Fraction Answer = { 0 };
    switch (op)
    {
    case PLUS:
        Answer = AddTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value, bPrint);
        break;
    case MINUS:
        Answer = SubTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value, bPrint);
        break;
    case DIVIDE:
        Answer = DivideTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value, bPrint);
        break;
    case MULTIPLY:
        Answer = MultiplyTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value, bPrint);
        break;
    }

    // the left arg now has the answer betwen the two args, and 
    // we also move over the right arg's operator.
    // Then, the right arg becomes 'invalid' and is never looked at again,
    // because we set its operator to 'DEAD'. (or we could call it 'USED')
    pMathArgs[LeftIndex].Op = pMathArgs[RightIndex].Op;
    pMathArgs[LeftIndex].Value = Answer;
    pMathArgs[RightIndex].Value.Numerator = 0;
    pMathArgs[RightIndex].Value.Denominator = 0;
    pMathArgs[RightIndex].Op = DEAD;

    return LeftIndex; // this is the index with the answer
}

Fraction EvaluateExpression(ValueAndOp* pMathArgs, int Count, bool bPrint)
{
    int c = Count - 1; // process one less time than count of numbers
    int AnswerIndex = 0;
    while (c)
    {
        AnswerIndex = DoHighestPriMathArg(pMathArgs, Count, bPrint);
        if (bPrint)
        {
            PrintExpression(pMathArgs, Count);
        }
        c--;
    }

    return pMathArgs[AnswerIndex].Value;
}

int main()
{
    srand(time(NULL));

    int argcount = 4;
    ValueAndOp* pMathArgs = malloc(sizeof(ValueAndOp) * argcount);
    ValueAndOp* pMathArgsCopy = malloc(sizeof(ValueAndOp) * argcount);

    for (int trial = 0; trial < 10000; trial++)
    {
        Fraction Answer;
        Answer.Numerator = 1;
        Answer.Denominator = 1;

        do
        {
            for (int i = 0; i < argcount; i++)
            {
                while(true)
                {
                    pMathArgs[i].Value.Numerator = (rand() % 2001) - 1000;
                    if ((i>0) && pMathArgs[i - 1].Op == DIVIDE && pMathArgs[i].Value.Numerator == 0)
                    {
                        continue; // no 0 if prior math op is divide
                    }
                    break;
                }
                
                pMathArgs[i].Value.Denominator = 1;

                // If we have N numbers to operate upon, then we have N - 1 math operators.
                // Define the last operator in the chain to be "LONE". The math operator as position N
                // means "operate on values at indexes N and N+1 with the given operator"

                if (i != argcount - 1)
                {
                    pMathArgs[i].Op = (Operator)(rand() % 4); // we started valid math operators at 0
                }
                else
                {
                    pMathArgs[i].Op = LONE;
                }
            }

            memcpy(pMathArgsCopy, pMathArgs, argcount * sizeof(ValueAndOp));

            Answer = EvaluateExpression(pMathArgs, argcount, false /*print*/ );
            if (IsInvalid(Answer))
            {
                printf("Whoops, random args resulted in an invalid arg, picking another one.\n");
            }

        } while (IsInvalid(Answer));

        PrintExpression(pMathArgsCopy, argcount);
        Answer = EvaluateExpression(pMathArgsCopy, argcount, false /*print*/);

        printf("The final answer is: ");
        PrintValue(Answer);
        printf("\n\n");

        printf("Enter whole numbers normally, and mixed fractions as whole# R numerator#/denominator#.\nYou can, but don't need to, simplify the fraction\n");
        printf("Answer:");

        Fraction UserAnswer;
        UserAnswer.Numerator = 1;
        UserAnswer.Denominator = 1;

        bool IsWholeNumber = Answer.Denominator == 1;

        bool theyEnteredFraction = false;
        char buf[256];
        gets_s(buf, 256);
        for (int i = 0; i < strlen(buf); i++)
        {
            if (buf[i] == 'R')
            {
                theyEnteredFraction = true;
                break;
            }
        }

        if (theyEnteredFraction)
        {
            int whole;
            char R;
            char DivideSymbol;
            int numerator;
            int denominator;
            sscanf(buf, "%d %c %d %c %d", &whole, &R, &numerator, &DivideSymbol, &denominator);
            UserAnswer.Numerator = whole * denominator + numerator;
            UserAnswer.Denominator = denominator;
        }
        else
        {
            UserAnswer.Numerator = atoi(buf);
        }
        Simplify(&UserAnswer, true);

        if (UserAnswer.Numerator != Answer.Numerator || UserAnswer.Denominator != Answer.Denominator)
        {
            printf("nope.\n");
        }
        else
        {
            printf("You guessed it! :-) \n");
        }
    }

    free(pMathArgs);
    free(pMathArgsCopy);
}
