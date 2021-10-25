#include <math.h>
#include <stdio.h>

int main () {
    int number;
    double x, next_x, square, result = 0.0;
    scanf("%d", &number);
    double h = 2.0 / number;
    for (int i = 0; i <= number - 1; i++) {
        x = h * i;
        next_x = h * (i + 1);
        square = (h * (sqrt(4 - pow(x, 2)) + sqrt(4 - pow(next_x, 2)))) / 2;
        result += square;
    }
    printf("result: %f", result);
    return 0;
}
