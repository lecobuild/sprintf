#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define MAX_SPEC 15
#define MAX_LENGTH 3
#define MAX_FLAG 5

typedef struct {
    char flags[MAX_FLAG];
    int width;
    int precision;
    char length;
    char specifier;
} specifier_builder;


int s21_sprintf(char *str, const char *format, ...);

specifier_builder ParserForSpecifier(const char *format, va_list *ap, int *shift);

char *BuilderForStr(char *str, specifier_builder specifiers, char *buffer_str);

char *BuilderForPrecision(char *str, char *buffer_str, int *i, int *chars_to_print, specifier_builder specifiers);

char *SprintForC(char *str, specifier_builder specifiers, va_list *ap);

char *SprintForD(char *str, specifier_builder specifiers, va_list *ap);

char *SprintForE(char *str, specifier_builder specifiers, va_list *ap, int precision);

char *SprintForF(char *str, specifier_builder specifiers, va_list *ap, int precision);

void printSpecifierBuilder(const specifier_builder sb);

int main(void) {
    int i = 1;
    int i2 = 12;
    int i3 = 123;
    char i4 = 'Y';
    int decimal = 123;
    int octal = 0123;
    double hex = 123.991;

    double ii = 123.123;
    //printf("%*d\n", 10, i);
//    printf("%0123f\n", ii);

    char str[100000] = {};
    //char format[100] = "Kek%+*.5Ldstopc kek\n";
    char format[100] = "Kek %g stop kek\n";

    printf(format, hex, hex, i3);

    s21_sprintf(str, format, hex, hex, i3);
    //s21_sprintf(str, format, i);
    return 0;
}


int s21_sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char *p = str;
    specifier_builder specifiers;

    for (int i = 0; i < strlen(format); ++i) {
        if (format[i] == '%') {
            int shift = 0;
            specifiers = ParserForSpecifier(format + (i + 1), &ap, &shift);

            i += shift;
            int num = 0;

            if(specifiers.specifier == 'g'){
                va_list args_copy;
                va_copy(args_copy, ap);

                num = va_arg(args_copy, double);

                if (num < 0.0001 || num >= 1e6) {
                    specifiers.specifier = 'e';
                } else {
                    specifiers.specifier = 'f';
                }
                va_end(args_copy);
            }
            switch (specifiers.specifier) {
                case 'c':
                    str = SprintForC(str, specifiers, &ap);
                    p = &(str[strlen(str)]);
                    *p = '\0';
                    break;
                case 'd':
                case 'i':
                    str = SprintForD(str, specifiers, &ap);
                    p = &(str[strlen(str)]);
                    *p = '\0';
                    break;
                case 'e':
                case 'E':
                    str = SprintForE(str, specifiers, &ap, specifiers.precision);
                    p = &(str[strlen(str)]);
                    *p = '\0';
                    break;
                case 'f':
                    str = SprintForF(str, specifiers, &ap, specifiers.precision);
                    p = &(str[strlen(str)]);
                    *p = '\0';
                    break;
            }
        } else {
            *p = format[i];
            p++;
            *p = '\0';
        }
    }

    printf("%s\n", str);
    printSpecifierBuilder(specifiers);

    va_end(ap);
    return 0;
}

specifier_builder ParserForSpecifier(const char *format, va_list *ap, int *shift) {


    specifier_builder specifiers = {{0}, -1, -1, 0, 0};
    char specifier_list[
            MAX_SPEC + 1] = {'c', 'd', 'i', 'e', 'E', 'f', 'g', 'G', 'o', 's', 'u', 'x', 'X', 'p', 'n', '\0'};
    char flag_list[MAX_FLAG + 1] = {'-', '+', ' ', '#', '0', '\0'};
    char length_list[MAX_LENGTH + 1] = {'h', 'l', 'L', '\0'};
    int break_status = 6;

    for (int i = 0; break_status != 0 && i < strlen(format); ++i) {
        *shift += 1;
        if (break_status == 6 && strchr(flag_list, format[i]) != NULL) {
            if (strchr(specifiers.flags, format[i]) == NULL) {
                int length_of_flags = (int) strlen(specifiers.flags);
                specifiers.flags[length_of_flags] = format[i];
                specifiers.flags[length_of_flags + 1] = '\0';
            }
        } else if (break_status == 6) break_status = 5;

        if (break_status == 5 && ((format[i] - '0' >= 0 && format[i] - '0' <= 9) || format[i] == '*')) {

            specifiers.width = specifiers.width * 10 + (format[i] - '0');
            if (format[i] == '*') {
                specifiers.width = va_arg(*ap, int);
                break_status = 4;
                continue;
            }
        } else if (break_status == 5) break_status = 4;

        if (break_status == 4 && format[i] == '.') {
            break_status = 3;
            continue;
        } else if (break_status == 4) break_status = 3;

        if (break_status == 3 && ((format[i] - '0' >= 0 && format[i] - '0' <= 9) || format[i] == '*')) {
            if (specifiers.precision == -1) specifiers.precision = 0;
            specifiers.precision = specifiers.precision * 10 + (format[i] - '0');
            if (format[i] == '*') {
                specifiers.precision = va_arg(*ap, int);
                break_status = 2;
                continue;
            }
        } else if (break_status == 3) break_status = 2;


        if (break_status == 2 && strchr(length_list, format[i]) != NULL) {
            specifiers.length = format[i];
            break_status = 1;
        } else if (break_status == 2) break_status = 1;

        if (strchr(specifier_list, format[i]) != NULL) {
            specifiers.specifier = format[i];
            break_status = 0;
        }

    }


    switch (specifiers.specifier) {
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            if (specifiers.precision == -1) {
                specifiers.precision = 6;
            }
            break;
        default:
            if (specifiers.precision == -1) {
                specifiers.precision = 1;
            }
    }


    return specifiers;
}


char *SprintForC(char *str, specifier_builder specifiers, va_list *ap) {

    char buffer_str[2] = {(char) va_arg(*ap, int), '\0'};

    str = BuilderForStr(str, specifiers, buffer_str);


    return str;
}

char *SprintForD(char *str, specifier_builder specifiers, va_list *ap) {

    char buffer_str[12] = {};

    int num = va_arg(*ap, int);
    long int temp = 0;
    int i = 0;

    while (num != 0) {
        temp = temp * 10 + num % 10;
        num /= 10;
    }

    while (temp != 0) {
        buffer_str[i] = '0' + temp % 10;
        buffer_str[i + 1] = '\0';
        i++;
        temp /= 10;
    }

    str = BuilderForStr(str, specifiers, buffer_str);
    return str;
}

char *SprintForE(char *str, specifier_builder specifiers, va_list *ap, int precision) {

    char buffer_str[50] = {};

    double num = va_arg(*ap, double);
    int i = 0;
    int counter = 0;

    while (num >= 10) {
        num /= 10.0;
        counter++;
    }

    while (num < 1) {
        num *= 10.0;
        counter--;
    }


    int intPart = (int) num;
    i = 0;
    while (intPart > 0) {
        buffer_str[i] = '0' + intPart % 10;
        intPart /= 10;
        i++;
    }

    buffer_str[i] = '.';
    i++;

    double fractionalPart = fabs(num - (double) ((int) num));
    for (int j = 0; j < precision; j++) {
        fractionalPart *= 10;
        buffer_str[i] = '0' + (int) fractionalPart;
        i++;
        fractionalPart -= (int) fractionalPart;
    }
//    int totalLen = i;
//
//    for (int k = 0; k < totalLen / 2; k++) {
//        char temp = buffer_str[k];
//        buffer_str[k] = buffer_str[totalLen - k - 1];
//        buffer_str[totalLen - k - 1] = temp;
//    }
    buffer_str[i] = specifiers.specifier;
    i++;
    if (counter > 0) {
        buffer_str[i] = '+';
        i++;
    } else {
        buffer_str[i] = '-';
        counter *= -1;
        i++;
    }

    int temp = 0;

    if (counter < 10) buffer_str[i] = '0';
    i++;

    while (counter != 0) {
        temp = temp * 10 + counter % 10;
        counter /= 10;
    }

    while (temp != 0) {
        buffer_str[i] = '0' + temp % 10;
        buffer_str[i + 1] = '\0';
        i++;
        temp /= 10;
    }


    buffer_str[i] = '\0';


    str = BuilderForStr(str, specifiers, buffer_str);
    return str;
}

char *SprintForF(char *str, specifier_builder specifiers, va_list *ap, int precision) {

    char buffer_str[50] = {};

    double num = va_arg(*ap, double);
    int i = 0;


    long int intPart = (int) num;

    long int temp = 0;

    while (intPart > 0) {
        temp = temp * 10 + intPart % 10;
        intPart /= 10;
    }
    intPart = temp;

    while (intPart > 0) {
        buffer_str[i] = '0' + intPart % 10;
        intPart /= 10;
        i++;
    }

    buffer_str[i] = '.';
    i++;

    double fractionalPart = fabs(num - (double) ((int) num));
    for (int j = 0; j < precision; j++) {
        fractionalPart *= 10;
        buffer_str[i] = '0' + (int) fractionalPart;
        i++;
        fractionalPart -= (int) fractionalPart;
    }

    buffer_str[i] = '\0';


    str = BuilderForStr(str, specifiers, buffer_str);
    return str;
}

char *BuilderForStr(char *str, specifier_builder specifiers, char *buffer_str) {


    int chars_in_specifier = strlen(buffer_str);
    int chars_to_print = 0;
    if (specifiers.specifier == 'd' && specifiers.precision >= 0 && chars_in_specifier < specifiers.precision)
        chars_in_specifier = specifiers.precision;
    char *p = str;
    int i = strlen(str);
    if (specifiers.width == -1) specifiers.width = strlen(buffer_str);

    while (chars_to_print < specifiers.width) {
        if ((strchr(specifiers.flags, '-') != NULL && chars_to_print == 0) ||
            (strchr(specifiers.flags, '-') == NULL && chars_to_print >= specifiers.width - chars_in_specifier)) {

            str = BuilderForPrecision(str, buffer_str, &i, &chars_to_print, specifiers);

        } else {
            p[i] = ' ';
            p[i + 1] = '\0';
            i++;
            chars_to_print++;
        }

    }

    return str;
}

char *BuilderForPrecision(char *str, char *buffer_str, int *i, int *chars_to_print, specifier_builder specifiers) {

    char *p = str;
    int precision_counter = 0;

    switch (specifiers.specifier) {
        case 'c':
            while (*buffer_str != '\0') {
                p[*i] = *buffer_str;
                buffer_str++;
                p[*i + 1] = '\0';
                *i += 1;
                *chars_to_print += 1;
            }
            break;
        case 'E':
        case 'e':
            while (*buffer_str != '\0') {
                p[*i] = *buffer_str;
                buffer_str++;
                p[*i + 1] = '\0';
                *i += 1;
                *chars_to_print += 1;
            }
            break;
        case 'f':
            while (*buffer_str != '\0') {
                p[*i] = *buffer_str;
                buffer_str++;
                p[*i + 1] = '\0';
                *i += 1;
                *chars_to_print += 1;
            }
            break;
        case 'i':
        case 'd':
            while (precision_counter + strlen(buffer_str) < specifiers.precision) {
                p[*i] = '0';
                p[*i + 1] = '\0';
                *i += 1;
                *chars_to_print += 1;
                precision_counter++;
            }
            while (*buffer_str != '\0') {
                p[*i] = *buffer_str;
                buffer_str++;
                p[*i + 1] = '\0';
                *i += 1;
                *chars_to_print += 1;

            }
            break;
        default:
            break;

    }

    return str;
}


void printSpecifierBuilder(const specifier_builder sb) {
    printf("Flags: %s\n", sb.flags);
    printf("Width: %d\n", sb.width);
    printf("Precision: %d\n", sb.precision);
    printf("Length: %c\n", sb.length);
    printf("Specifier: %c\n", sb.specifier);
}