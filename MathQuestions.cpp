#include <iostream>
#include <assert.h>

enum MathOp
{
    DEAD = -2, // math arg already used
    LONE = -1, // the last arg that doesn't have an operator
    PLUS = 0,
    MINUS,
    MULTIPLY,
    DIVIDE
};

struct NumberFraction
{
    int Numerator;
    int Denominator;
};

struct MathArg
{
    NumberFraction Value;
    MathOp Op;
};

const char* GetMathOpString(MathOp op)
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

bool IsZero(NumberFraction A)
{
    return A.Numerator == 0 ? true : false;
}

bool IsInfinite(NumberFraction B)
{
    return B.Denominator == 0 ? true : false;
}

void PrintValue(NumberFraction n)
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
        printf("[%d %d/%d]", Whole, abs(Remainder), abs(Denominator));
    }
    else
    {
        // print just the fractional part
        printf("[%d/%d]", Remainder, Denominator);
    }

}

void PrintExpression(MathArg* pMathArgs, int Count)
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

void EnsureDenominatorIsPositive(NumberFraction* pNum)
{
    if (pNum->Denominator < 0)
    {
        printf("Flipping sign for numerator for %d over %d\n", pNum->Numerator, pNum->Denominator);
        pNum->Denominator *= -1;
        pNum->Numerator *= -1;
    }
}

// simplify this number to be the simplest fraction it can be.

void Simplify(NumberFraction* pNum)
{
    EnsureDenominatorIsPositive(pNum);

    if (pNum->Denominator == 1)
    {
        return;
    }

    bool retry;
    bool simplifiedSomething = false;
    do
    {
        NumberFraction num = *pNum; // don't reference pNum-> all the way through this loop, it's slow
        retry = false;
        for (int i = 2; i < num.Denominator / 2; i++)
        {
            // this modulo, even with - numbers, works for finding if divisible by i

            int numerator_remainder = num.Numerator % i;
            int denominator_remainder = num.Denominator % i;

            if (numerator_remainder == 0 && denominator_remainder == 0)
            {
                // when we find a number both the top and bottom are dividable by,
                // divide both sides and start from the beginning. this is because it could be dividable by
                // the same number ( for example 2 ), more than once.

                printf("Dividing both numerator and denominator by %d\n", i);
                pNum->Numerator /= i;
                pNum->Denominator /= i;
                simplifiedSomething = true;
                retry = true;
                break;
            }
        }
    } while (retry);

    if (simplifiedSomething)
    {
        printf("Final simplification of value is :");
        PrintValue(*pNum);
        printf("\n");
    }
}

// only called for add. Don't call when eiter denominator is 0

void ConvertToCommonDenominator(NumberFraction* pA, NumberFraction* pB)
{
    // don't let the denominator be negative...
    // this works even if the numerator is 0
    EnsureDenominatorIsPositive(pA);
    EnsureDenominatorIsPositive(pB);

    int CommonDenominator = pA->Denominator * pB->Denominator;
    int ScaleA = CommonDenominator / pA->Denominator;
    pA->Numerator *= ScaleA;
    pA->Denominator *= ScaleA;
    int ScaleB = CommonDenominator / pB->Denominator;
    pB->Numerator *= ScaleB;
    pB->Denominator *= ScaleB;
}

NumberFraction AddTwoNumbers(NumberFraction A, NumberFraction B)
{
    if (IsInfinite(A) || IsInfinite(B))
    {
        // anything plus infinite is infinite. We'll just return a generic INFINITE
        NumberFraction inf;
        inf.Numerator = 1;
        inf.Denominator = 0;
        return inf;
    }
    if (IsZero(A)) return B;
    if (IsZero(B)) return A;

    if (A.Denominator != 1 || B.Denominator != 1)
    {
        int stop = 0;
    }

    ConvertToCommonDenominator(&A, &B);
    NumberFraction Result;
    Result.Numerator = A.Numerator + B.Numerator;
    Result.Denominator = A.Denominator;
    Simplify(&Result);
    return Result;
}

NumberFraction SubTwoNumbers(NumberFraction A, NumberFraction B)
{
    // subtraction is simply addition of the -. Only negate the numerator, not the denominator.
    // it's okay if it's 0, since -0 is equal to 0

    NumberFraction NegativeB = B;
    NegativeB.Numerator = -NegativeB.Numerator;
    return AddTwoNumbers(A, NegativeB);
}

NumberFraction MultiplyTwoNumbers(NumberFraction A, NumberFraction B)
{
    // this works even if one is infinite or one is zero.
    // A/B * C/D if infinity will work out to be (A*C) / (B*D). If one is infinite, the result will be infinite, which is true.
    // Could end up being 0/0 which again, is technically infinite?
    NumberFraction result;
    result.Numerator = A.Numerator * B.Numerator;
    result.Denominator = A.Denominator * B.Denominator;
    Simplify(&result);
    return result;
}

NumberFraction DivideTwoNumbers(NumberFraction A, NumberFraction B)
{
    // if one of the numbers is infinite, it's denominator will be 0. Any number X divided by infinite becomes 0...
    // so that works... If you divide by 0, B.Numerator is 0, so result.Denominator will become 0, and this
    // number will correctly result in "infinite"

    NumberFraction result;
    result.Numerator = A.Numerator * B.Denominator;
    result.Denominator = A.Denominator * B.Numerator;
    Simplify(&result);
    return result;
}

void GetTwoMathOpIndexes(MathArg* pMathArgs, int Count, int* pLeftIndex, int* pRightIndex)
{
    // find the highest priority math operation. within the same operator, priority is left to right
    // the operation for the last arg in the list is always NONE, so we can ignore it

    MathOp HighestOperation = DEAD;
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

int DoHighestPriMathArg(MathArg* pMathArgs, int Count)
{
    // find two things to operate upon

    int LeftIndex, RightIndex;
    GetTwoMathOpIndexes(pMathArgs, Count, &LeftIndex, &RightIndex);

    // switch on the operator and do the operation

    MathOp op = pMathArgs[LeftIndex].Op;

    NumberFraction Answer = { 0 };
    switch (op)
    {
    case PLUS:
        Answer = AddTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value);
        break;
    case MINUS:
        Answer = SubTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value);
        break;
    case DIVIDE:
        Answer = DivideTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value);
        break;
    case MULTIPLY:
        Answer = MultiplyTwoNumbers(pMathArgs[LeftIndex].Value, pMathArgs[RightIndex].Value);
        break;
    }

    pMathArgs[LeftIndex].Op = pMathArgs[RightIndex].Op;
    pMathArgs[LeftIndex].Value = Answer;
    pMathArgs[RightIndex].Value.Numerator = 0;
    pMathArgs[RightIndex].Value.Denominator = 0;
    pMathArgs[RightIndex].Op = DEAD;

    return LeftIndex; // this is the index with the answer
}

int EvaluateExpression(MathArg* pMathArgs, int Count)
{
    int c = Count - 1; // process one less time than count of numbers
    int AnswerIndex = 0;
    while (c)
    {
        AnswerIndex = DoHighestPriMathArg(pMathArgs, Count);
        PrintExpression(pMathArgs, Count);
        c--;
    }

    return AnswerIndex;
}

int main()
{
    srand(time(NULL));

    for (int trial = 0; trial < 10; trial++)
    {
        int argcount = 4;
        MathArg* pMathArgs = new MathArg[argcount];
        for (int i = 0; i < argcount; i++)
        {
            pMathArgs[i].Value.Numerator = (rand() % 2001) - 1000;
            pMathArgs[i].Value.Denominator = 1;

            // If we have N numbers to operate upon, then we have N - 1 math operators.
            // Define the last operator in the chain to be "LONE". The math operator as position N
            // means "operate on values at indexes N and N+1 with the given operator"

            if (i != argcount - 1)
            {
                pMathArgs[i].Op = (MathOp)(rand() % 4); // we started valid math operators at 0
            }
            else
            {
                pMathArgs[i].Op = LONE;
            }
        }

        PrintExpression(pMathArgs, argcount);

        int FinalArg = EvaluateExpression(pMathArgs, argcount);
        printf("The final answer is: ");
        PrintValue(pMathArgs[FinalArg].Value);
        printf("\n\n");

        delete[] pMathArgs;
    }
}
