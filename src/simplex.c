#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX 10000
//#define TOLERANCE 1e-7
#define TOLERANCE 1e-7
#define MAX_DECIMAL 3
//#define TOLERANCE 0.00001


// TODO temp
#include <math.h>
#define ABSOLUTE_VALUE(x) ((x) < 0 ? -(x) : (x))
typedef struct {
    long numerator;
    unsigned long denominator;
} fraction;

unsigned long least_common_multiple(unsigned long a, unsigned long b) {
    if (b > a) {
        long tmp = b;
        b = a;
        a = tmp;
    }
    for (size_t i = a; i < a * b; i++) {
        if (!(a % i) && !(b % i)) return i;
    }
    return a * b;
}

fraction simplify(fraction a);

unsigned long greatest_common_divisor(unsigned long a, unsigned long b) {
    if (b < a) {
        unsigned long tmp = b;
        b = a;
        a = tmp;
    }
    for (size_t i = a; i >= 2; i--) {
        if (!(a % i) && !(b % i)) return i;
    }
    return 1;
}

fraction fraction_from_decimal(double a) {
    fraction result = {.numerator = 0, .denominator = 1};
    char is_negative = 0;
    if (a < 0) {
        is_negative = 1;
        a = -a;
    }
    //while ((result.numerator = a) != a) {
    //while ((result.numerator = a) != a || result.numerator - a > TOLERANCE) { NON CREDO FUNZIONI
    //while ((result.numerator = a) != a && result.numerator - a > TOLERANCE) {
    unsigned long tmp = log10(a) + 1 + MAX_DECIMAL;
    while ((result.numerator = a) != a && tmp-- > 0) {
        //printf("%ld    %ld %lf\n", tmp, result.numerator, a);
        a *= 10;
        result.numerator = (long) a;
        result.denominator *= 10;
    }
    result = simplify(result);
    result.numerator *= is_negative ? -1L : 1L;
    return result;
}

double double_from_fraction(const fraction a) {
    assert(a.denominator);
    return ((double) a.numerator) / ((double) a.denominator);
}

fraction simplify(fraction a) {
    assert(a.denominator);
    fraction result = a;
    unsigned long gcm = greatest_common_divisor(ABSOLUTE_VALUE(a.numerator), a.denominator);
    if (gcm > 1) {
        result.numerator /= (long) gcm;
        result.denominator /= gcm;
    }
    if (!result.numerator)  result.denominator = 1;
    return result;
}

fraction fraction_sum(const fraction a, const fraction b) {
    assert(a.denominator);
    assert(b.denominator);
    fraction result;
    //result.denominator = least_common_multiple(a.denominator, b.denominator);
    result.denominator = a.denominator * b.denominator;
    result.numerator =  a.numerator * ((long) result.denominator / a.denominator) + b.numerator * ((long) result.denominator / b.denominator);
    return simplify(result);
}

fraction fraction_subtract(const fraction a, const fraction b) {
    assert(a.denominator);
    assert(b.denominator);
    fraction new_b = {.numerator = -b.numerator, .denominator = b.denominator};
    return fraction_sum(a, new_b);
}

fraction fraction_multiply(const fraction a, const fraction b) {
    assert(a.denominator);
    assert(b.denominator);
    fraction result;
    result.numerator = a.numerator * b.numerator;
    result.denominator = a.denominator * b.denominator;
    return simplify(result);
}

fraction fraction_divide(const fraction a, const fraction b) {
    assert(a.denominator);
    assert(b.denominator);
    assert(b.numerator);
    fraction new_b = {
        .numerator = (long) b.denominator * (b.numerator < 0 ? -1 : 1),
        .denominator = ABSOLUTE_VALUE(b.numerator)
    };
    fraction result = fraction_multiply(a, new_b);
    //printf("\nDOPO %ld/%lu * %ld/%lu = %ld/%lu\n", a.numerator, a.denominator, new_b.numerator, new_b.denominator, result.numerator, result.denominator);
    return result;
}

int fraction_compare_with_double(const fraction a, double b) {
    assert(a.denominator);
    if (!b) return !a.numerator ? 0 : a.numerator > 0 ? 1 : -1;
    double result = double_from_fraction(a) - b;
    return !result ? 0 :
            result < 0 ? -1 : 1;
}

int fraction_compare_with_fraction(const fraction a, const fraction b) {
    return fraction_compare_with_double(a, double_from_fraction(b));
}

//#define IS_ZERO(x) ((x) < TOLERANCE && (x) > -TOLERANCE)

#define TOTAL_VARIABLES (tableau->number_of_original_variables +\
    tableau->number_of_slack_variables +\
    tableau->number_of_artificial_variables)

#define PRINT_ALL_BASE_VARIABLES do {\
    printf("\n");\
    int count = 0;\
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)\
        if (tableau->is_variable_in_base[j]) {\
            count++;\
            printf("variable %zu in base for costraint %d\n", j, tableau->is_variable_in_base[j]);\
        }\
    printf("Number of variables in base: %d\n", count);\
} while (0);

#define PRINT_ALL_TABLEAU do {\
    printf("\n");\
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)\
        printf("%10lf ", (double)j);\
    printf("\n\n");\
    for (size_t i = 0; i <= tableau->number_of_costraints; i++) {\
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {\
            printf("%ld/%lu %s", tableau->table[i][j].numerator, tableau->table[i][j].denominator, !j ? "| " : "");\
        }\
        printf("\n%s", !i ? "-----------------------------------\n" : "");\
    }\
    printf("\n");\
} while (0);

typedef enum {
    NO_SOLUTION = 0,
    NORMAL_SOLUTION,
    UNBOUNDED_SOLUTION
} result_type;

typedef enum {
    NO_VARIABLE = 0,
    NORMAL_VARIABLE,
    SLACK_VARIABLE,
    ARTIFICIAL_VARIABLE
} variable_type;

typedef struct {
    fraction(* table)[MAX];
    variable_type type_of_variable[MAX];
    int is_variable_in_base[MAX];
    int number_of_original_variables, number_of_costraints,
    number_of_slack_variables, number_of_artificial_variables;
    int minimize;
} tableau_format;

result_type simplex(tableau_format* const tableau, fraction* const result);
result_type simplex_first_phase(tableau_format* const tableau);
result_type simplex_second_phase(tableau_format* const tableau, fraction* const result);
result_type simplex_loop(tableau_format* const tableau, fraction* const result);
size_t minimum_ratio(tableau_format* const tableau, const size_t j);
void pivot_on_all_base_variables(tableau_format* const tableau);
size_t has_artificial_variable_in_base(tableau_format* const tableau);
void exclude_all_artificial_variables_from_base(tableau_format* const tableau);
void pivot(tableau_format* const tableau, const size_t i, const size_t j);
void restore_zeroes(tableau_format* const tableau);
int does_not_need_first_phase(tableau_format* const tableau);
void add_artificial_variables(tableau_format* const tableau, int* needsArtificialVariables);
void print_variables(tableau_format* const tableau);
void free_tableau(tableau_format* const tableau);
int create_tableau_from_file(tableau_format* const tableau, const char* file_name);
int create_tableau_from_input(tableau_format* const tableau);

// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
int main(int argc, char *argv[]) {
    tableau_format tableau;
    fraction result = fraction_from_decimal(0);
    if (argc > 1) {
        if (!create_tableau_from_file(&tableau, argv[1])) {
            printf("An error occurred while reading the file!\n");
            return EXIT_FAILURE;
        }
    } else {
        create_tableau_from_input(&tableau);
    }
    switch (simplex(&tableau, &result)) {
        case NO_SOLUTION: {
            printf("\n\nThere is no solution for this problem!\n");
            break;
        }
        case UNBOUNDED_SOLUTION: {
            printf("\n\nThe solution is unbounded!\n");
            break;
        }
        default: {
            printf("\n\nFound optimal solution!\n");
            printf("\nOptimal cost: %ld", result.numerator);
            if (ABSOLUTE_VALUE(result.denominator) != 1)
                printf("/%lu", result.denominator);
            printf(" = %lf\n\n", double_from_fraction(result));
            printf("Non zero variables:\n\n");
            print_variables(&tableau);
        }
    };
    free_tableau(&tableau);
    return 0;
}
// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

result_type simplex(tableau_format* const tableau, fraction* const result) {
    //result->numerator = 0;
    //result->denominator = 1;
    *result = fraction_from_decimal(0);
    //PRINT_ALL_TABLEAU
    if (simplex_first_phase(tableau) == NO_SOLUTION)  return NO_SOLUTION;
    printf("\n\n Fase 2 \n\n");
    result_type res = simplex_second_phase(tableau, result);
    *result = fraction_multiply(*result, fraction_from_decimal(tableau->minimize));
    //PRINT_ALL_TABLEAU
    return res;
}

result_type simplex_first_phase(tableau_format* const tableau) {
    if (does_not_need_first_phase(tableau)) return NORMAL_SOLUTION;
    printf("\n\n Fase 1 \n\n");
    fraction backup_objective_function[TOTAL_VARIABLES + 1];
    //PRINT_ALL_TABLEAU
    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
//        printf("%lf ", tableau->table[0][j]);
        backup_objective_function[j] = tableau->table[0][j];
        /*
        fraction coefficient = {
            .numerator = tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0,
            .denominator = 1
        };
        tableau->table[0][j] = coefficient;
        */
        tableau->table[0][j] = fraction_from_decimal(tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0);
    }
    //PRINT_ALL_TABLEAU

    pivot_on_all_base_variables(tableau);
    //fraction result = {.numerator = 0, .denominator = 1};
    fraction result = fraction_from_decimal(0);
    simplex_loop(tableau, &result);
    //printf("\nTableau ottimo fase 1\n");
    //PRINT_ALL_TABLEAU
    //if (result > 0.0) return NO_SOLUTION;
    //if (!IS_ZERO(result)) return NO_SOLUTION;
    if (fraction_compare_with_double(result, 0) > 0) return NO_SOLUTION;
    exclude_all_artificial_variables_from_base(tableau);
    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
        tableau->table[0][j] = backup_objective_function[j];
    // TODO
    // TODO assert input dimension
    pivot_on_all_base_variables(tableau);
    return NORMAL_SOLUTION;
}

result_type simplex_second_phase(tableau_format* const tableau, fraction* const result) {
    return simplex_loop(tableau, result);
}

result_type simplex_loop(tableau_format* const tableau, fraction* const result) {
    result_type type_of_result = NO_SOLUTION;

    while (type_of_result == NO_SOLUTION) {
        type_of_result = NORMAL_SOLUTION;
        //PRINT_ALL_TABLEAU
        for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
            //if (tableau->table[0][j] <= TOLERANCE) continue;
            if (fraction_compare_with_double(tableau->table[0][j], 0) <= 0) continue;
            //if (tableau->is_variable_in_base[j])  continue;
            size_t minimum_index = minimum_ratio(tableau, j);
            type_of_result = UNBOUNDED_SOLUTION;

            //printf("index = %zu\n", minimum_index);
            if (!minimum_index) continue;
            type_of_result = NO_SOLUTION;

            printf("Cost: %24lf.\t", double_from_fraction(tableau->table[0][0]));
            pivot(tableau, minimum_index, j);
            //PRINT_ALL_TABLEAU
            break;
        }
        // TODO check why empty costraint gives problems
    }

    *result = tableau->table[0][0];
    return type_of_result;
}

size_t minimum_ratio(tableau_format* const tableau, const size_t j) {
    size_t minimum_index = 0;
    size_t exiting_variable_index = 0;
    fraction minimum;

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        //if (tableau->table[i][j] <= TOLERANCE) continue;
        if (fraction_compare_with_double(tableau->table[i][j], 0) <= 0) continue;
        fraction ratio = fraction_divide(tableau->table[i][0], tableau->table[i][j]);
        //printf("ratio: %lf\n", ratio);
        //if (minimum_index == 0 || (ratio - minimum) < TOLERANCE) {
        int comparison = fraction_compare_with_fraction(ratio, minimum);
        if (minimum_index == 0 || comparison == -1) {
            minimum = ratio;
            minimum_index = i;
        } else if (!comparison) {
            for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
                if (tableau->is_variable_in_base[j] == i && (!exiting_variable_index || j < exiting_variable_index)) {
                    exiting_variable_index = j;
                    minimum = ratio;
                    minimum_index = i;
                    break;
                }
            }
        }
    }
    return minimum_index;
}

void pivot_on_all_base_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->is_variable_in_base[j])
            pivot(tableau, tableau->is_variable_in_base[j], j);
    //printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
}

size_t has_artificial_variable_in_base(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->is_variable_in_base[j] && tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)
            return j;
    return 0;
}

void exclude_all_artificial_variables_from_base(tableau_format* const tableau) {
    size_t j;
    int non_artificial_variables = TOTAL_VARIABLES - tableau->number_of_artificial_variables;
    fraction zero_fraction = fraction_from_decimal(0);

    while ((j = has_artificial_variable_in_base(tableau))) {
        int found_pivotable_variable = 0;
        size_t i = tableau->is_variable_in_base[j];
        found_pivotable_variable = 0;
        for (size_t current_j = 1; current_j <= non_artificial_variables; current_j++) {
            //if (!IS_ZERO(tableau->table[i][current_j])) {
            if (fraction_compare_with_double(tableau->table[i][current_j], 0)) {
                found_pivotable_variable = 1;
                pivot(tableau, i, current_j);
            }
        }
        if (!found_pivotable_variable) {
            for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++)
                tableau->table[i][current_j] = zero_fraction;
            for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++)
                tableau->table[current_i][j] = zero_fraction;
            tableau->is_variable_in_base[j] = 0;
        }
    }
    tableau->number_of_artificial_variables = 0;
}

void pivot(tableau_format* const tableau, const size_t i, const size_t j) {
    fraction coefficient = fraction_from_decimal(1);
    printf("Pivot on (%zu, %zu)\n", i, j);

    int found = 0;
    for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
        if (tableau->is_variable_in_base[current_j] == i){
            tableau->is_variable_in_base[current_j] = 0;
            tableau->is_variable_in_base[j] = i;
            found = 1;
            break;
        }
    }
    assert(found == 1);
    //if (!IS_ZERO(tableau->table[i][j] - 1.0)) {
    if (fraction_compare_with_double(tableau->table[i][j], 1)) {
        coefficient = tableau->table[i][j];
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
    //printf("E qui no? %zu / %d\n", current_j, TOTAL_VARIABLES);
            //printf("%lu/%ld // %lu/%ld\n", tableau->table[i][current_j].numerator, tableau->table[i][current_j].denominator, coefficient.numerator, coefficient.denominator);
            //printf("prima: %lu/%ld\n", tableau->table[i][current_j].numerator, tableau->table[i][current_j].denominator);
            //printf("coeff: %lu/%ld\n", coefficient.numerator, coefficient.denominator);
// TODO
        //printf("WEWE: %ld/%lu\n", tableau->table[i][j].numerator, tableau->table[i][j].denominator);
            tableau->table[i][current_j] = fraction_divide(tableau->table[i][current_j], coefficient);
            //printf("dopo: %lu/%ld\n", tableau->table[i][current_j].numerator, tableau->table[i][current_j].denominator);
        }
    }

    printf("PRIMA\n");
    //printf("Fin qua va\n");
    //restore_zeroes(tableau);
    for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++) {
        //coefficient = -tableau->table[current_i][j];
        coefficient = fraction_multiply(tableau->table[current_i][j], fraction_from_decimal(-1));
        //if (IS_ZERO(coefficient) || current_i == i)  continue;
        if (!fraction_compare_with_double(coefficient, 0) || current_i == i)  continue;
    //printf("Sono i sono qui %zu / %d\n", current_i, tableau->number_of_costraints);
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
    //printf("Sono qui %zu / %d\n", current_j, TOTAL_VARIABLES);
        //printf("(%zu, %zu) -> %ld/%lu = ", current_i, current_j, tableau->table[current_i][current_j].numerator, tableau->table[current_i][current_j].denominator);
            tableau->table[current_i][current_j] = fraction_sum(tableau->table[current_i][current_j], fraction_multiply(coefficient, tableau->table[i][current_j]));
        //printf("%ld/%lu + %ld/%lu * %ld/%lu\n", tableau->table[current_i][current_j].numerator, tableau->table[current_i][current_j].denominator, tableau->table[i][current_j].numerator, tableau->table[i][current_j].denominator, coefficient.numerator, coefficient.denominator);
            //printf("mult: %ld/%lu\n", fraction_multiply(coefficient, tableau->table[i][current_j]));
        }
        //printf("Pivot coefficient for line %zu = %lf\n", current_i, coefficient);
        if (current_i)
            //assert(tableau->table[current_i][0] >= -TOLERANCE);
            assert(fraction_compare_with_double(tableau->table[current_i][0], 0) >= 0);
    }
    //restore_zeroes(tableau);
    printf("DOPO\n");
}

void restore_zeroes(tableau_format* const tableau) {
    for (size_t i = 0; i <= tableau->number_of_costraints; i++)
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
            //if (IS_ZERO(tableau->table[i][j]))  tableau->table[i][j] = 0.0;
            if (!fraction_compare_with_double(tableau->table[i][j], 0))  tableau->table[i][j] = fraction_from_decimal(0);
}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)  return 0;
    //printf("DoesNotNEED\n");
    return 1;
}

void add_artificial_variables(tableau_format* const tableau, int* needsArtificialVariables) {
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        if (needsArtificialVariables[i]) {
           for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
                assert(tableau->is_variable_in_base[j] != i);
            tableau->number_of_artificial_variables++;
            tableau->type_of_variable[TOTAL_VARIABLES] = ARTIFICIAL_VARIABLE;
            //tableau->table[0][TOTAL_VARIABLES] = -1;
            tableau->table[0][TOTAL_VARIABLES] = fraction_from_decimal(-1);
            tableau->is_variable_in_base[TOTAL_VARIABLES] = i;
            for (size_t k = 1; k <= tableau->number_of_costraints; k++) {
                //tableau->table[k][TOTAL_VARIABLES] = k == i ? 1 : 0;
                tableau->table[k][TOTAL_VARIABLES] = fraction_from_decimal(k == i ? 1 : 0);
            }
        }
    }
}

void print_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        if (tableau->is_variable_in_base[j]) {
            fraction tmp = tableau->table[tableau->is_variable_in_base[j]][0];
            printf("x(%zu) = %ld", j, tmp.numerator);
            if (ABSOLUTE_VALUE(tmp.denominator) != 1)
                printf("/%lu", tmp.denominator);
            printf("\n");
        }
    }
}

void free_tableau(tableau_format* const tableau) {
    free(tableau->table);
}

int create_tableau_from_file(tableau_format* const tableau, const char* file_name) {
    if (tableau == NULL)  return 0;

    tableau->table = malloc(MAX * sizeof(*(tableau->table)));
    tableau->minimize = 1;

    FILE *fdata;

    // Apri il file dati
    if ((fdata = fopen(file_name, "r")) == NULL) return 0;

    // Leggi il numero di righe e colonne
    fscanf(fdata, "%d %d", &(tableau->number_of_original_variables), &(tableau->number_of_costraints));

    int typeOfEquation[tableau->number_of_costraints];
    // Inizializza il Tableau
    for (size_t i = 0; i <= tableau->number_of_costraints; i++)
        for (size_t j = 0; j <= tableau->number_of_original_variables; j++)
            //tableau->table[i][j] = 0.0;
            tableau->table[i][j] = fraction_from_decimal(0);

    // Leggi il vettore dei termini noti
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        //double cost;
        //fscanf(fdata,"%lf", &cost);
        //tableau->table[0][j] = -cost;

        //fscanf(fdata,"%lf", &(tableau->table[i][0]));
        double tmp;
        fscanf(fdata,"%lf", &tmp);
        tableau->table[i][0] = fraction_from_decimal(tmp);
    }

    // Leggi il vettore dei "versi"
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        int segno;
        tableau->type_of_variable[i] = NORMAL_VARIABLE;
        fscanf(fdata,"%d", &segno);
        typeOfEquation[i - 1] = segno;
    }

    // Leggi la matrice dei coefficienti
    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        // Leggi il costo della colonna j
        double cost;
        fscanf(fdata,"%lf", &cost);
        //tableau->table[0][j] = -cost;
    //printf("OIOIOI %zu/%zu: %ld/%lu\n", j, tableau->number_of_original_variables, fraction_from_decimal(cost).numerator, fraction_from_decimal(cost).denominator);
        tableau->table[0][j] = fraction_from_decimal(-cost);
        //fscanf(fdata,"%lf", &(tableau->table[i][0]));

        int no;
        // Leggi il numero di coefficienti non-zero della colonna j
        fscanf(fdata,"%d", &no);

        double tmp;
        int lastVariable = 0;
        // Leggi i coefficienti non-zero della colonna j
        for (size_t k = 1; k <= no; k++) {
    printf("WEWEWE %zu/%d %zu/%zu\n", k, no, j, tableau->number_of_original_variables);
            fscanf(fdata, "%d", &lastVariable);
            //fscanf(fdata, "%lf", &(tableau->table[lastVariable][j]));
            fscanf(fdata, "%lf", &tmp);
            printf("last: %lf\n", tmp);
            tableau->table[lastVariable][j] = fraction_from_decimal(tmp);
            printf("PROVA\n");
        }
    }
    // Chiudi il file dati
    fclose(fdata);

    // Aggiungo le variabili artificiali
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
        tableau->type_of_variable[j] = NORMAL_VARIABLE;
        tableau->is_variable_in_base[j] = 0;
    }

    int needsArtificialVariables[tableau->number_of_costraints];

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        needsArtificialVariables[i] = 0;
        if (typeOfEquation[i - 1] == 1) {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                //tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = i == y ? 1 : 0;
                tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = fraction_from_decimal(i == y ? 1 : 0);
            }
            //if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) < 0) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = i;
            }
        } else if (typeOfEquation[i - 1] == 0) {
                needsArtificialVariables[i] = 1;
            //if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) < 0) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
            }
        } else {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                //tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = i == y ? -1 : 0;
                tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = fraction_from_decimal(i == y ? -1 : 0);
            }
            //if (tableau->table[i][0] > 0) {
            //if (tableau->table[i][0] > -TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) > 0) {
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = i;
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
            }
        }
    }
    add_artificial_variables(tableau, needsArtificialVariables);
    return 1;
}

int create_tableau_from_input(tableau_format* const tableau) {
    if (tableau == NULL)  return 0;

    int lessEqualCostraints = 0, equalCostraints = 0,
    greaterEqualCostraints = 0, wantToReinsert, i;

    tableau->table = malloc(MAX * sizeof(*(tableau->table)));
    do {
        i = 0;
        do {
            printf("How many variables does this problem have? Answer: ");
            scanf("%d", &(tableau->number_of_original_variables));
        } while (tableau->number_of_original_variables <= 0);

        int choice = 0;
        do {
            printf("Is this a max (-1) or min (1) problem? Choice: ");
            scanf("%d", &choice);
        } while (choice != -1 && choice != 1);
        tableau->minimize = choice;

        printf("\nObjective function\n");

        //tableau->table[0][0] = 0;
        tableau->table[0][0] = fraction_from_decimal(0);

        for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
            printf("Enter the coefficient of " \
                   "variable number %zu: ", j);
            //scanf("%lf", &(tableau->table[i][j]));
            double tmp;
            scanf("%lf", &tmp);
            tableau->table[i][j] = fraction_from_decimal(tmp);
        }
        i++;

        do {
            printf("Insert how many <= costraints this problem has: \n");
            scanf("%d", &lessEqualCostraints);
        } while (lessEqualCostraints < 0);
        for (size_t j = 0; j < lessEqualCostraints; j++) {
            for (size_t k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %zu in " \
                       "\"<=\" costraint number %d: ", k, i);
                //scanf("%lf", &(tableau->table[i][k]));
                double tmp;
                scanf("%lf", &tmp);
                tableau->table[i][k] = fraction_from_decimal(tmp);
            }
            printf("Enter the well-known term for the " \
                   "\"<=\" costraint number %zu: ", j + 1);
            //scanf("%lf", &(tableau->table[i][0]));
            double tmp;
            scanf("%lf", &tmp);
            tableau->table[i][0] = fraction_from_decimal(tmp);
            i++;
        }

        do {
            printf("Insert how many = costraints this problem has: \n");
            scanf("%d", &equalCostraints);
        } while (equalCostraints < 0);
        for (size_t j = 0; j < equalCostraints; j++) {
            for (size_t k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %zu in " \
                       "\"=\" costraint number %d: ", k, i);
                //scanf("%lf", &(tableau->table[i][k]));
                double tmp;
                scanf("%lf", &tmp);
                tableau->table[i][k] = fraction_from_decimal(tmp);
            }
            printf("Enter the well-known term for the " \
                   "\"=\" costraint number %zu: ", j + 1);
            //scanf("%lf", &(tableau->table[i][0]));
            double tmp;
            scanf("%lf", &tmp);
            tableau->table[i][0] = fraction_from_decimal(tmp);
            i++;
        }

        do {
            printf("Insert how many >= costraints this problem has: \n");
            scanf("%d", &greaterEqualCostraints);
        } while (greaterEqualCostraints < 0);
        for (size_t j = 0; j < greaterEqualCostraints; j++) {
            for (size_t k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %zu in " \
                       "\">=\" costraint number %d: ", k, i);
                //scanf("%lf", &(tableau->table[i][k]));
                double tmp;
                scanf("%lf", &tmp);
                tableau->table[i][k] = fraction_from_decimal(tmp);
            }
            printf("Enter the well-known term for the " \
                   "\">=\" costraint number %zu: ", j + 1);
            //scanf("%lf", &(tableau->table[i][0]));
            double tmp;
            scanf("%lf", &tmp);
            tableau->table[i][0] = fraction_from_decimal(tmp);
            i++;
        }

        tableau->number_of_costraints = lessEqualCostraints + equalCostraints +
            greaterEqualCostraints;
        printf("\n\nTHIS IS THE PROBLEM: \n");
        printf("%s ", tableau->minimize == 1 ? "Min" : "Max");
        for (size_t i = 0; i <= tableau->number_of_costraints; i++) {
            for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
                //printf("%lf ", tableau->table[i][j]);
                printf("%lf ", double_from_fraction(tableau->table[i][j]));
            }
            if (i) {
                /*
                printf("%s %lf", i <= lessEqualCostraints ? "<=" :
                       i <= lessEqualCostraints + equalCostraints ? "=" : ">=",
                       tableau->table[i][0]);
                */
                printf("%s %lf", i <= lessEqualCostraints ? "<=" :
                       i <= lessEqualCostraints + equalCostraints ? "=" : ">=",
                       double_from_fraction(tableau->table[i][0]));
            }
            printf("\n%s", !i ? "s.t. " : "     ");
        }
        printf("\n");

        do {
            printf("\nInsert 1 to reinsert, 0 to proceed with the inserted data: ");
            scanf("%d", &wantToReinsert);
        } while (wantToReinsert && wantToReinsert != 1);
    } while (wantToReinsert);
    // initialization -> all disequations to <=, and add slack variablesNumber
    for (size_t j = 0; j <= tableau->number_of_original_variables; j++) {
        //tableau->table[0][j] *= -1 * tableau->minimize;
        tableau->table[0][j] = fraction_multiply(tableau->table[0][j], fraction_multiply(fraction_from_decimal(-1), fraction_from_decimal(tableau->minimize)));
        tableau->is_variable_in_base[j] = 0;
        //baseVariables[i] = 0;
    }
    tableau->type_of_variable[0] = NO_VARIABLE;
    int needsArtificialVariables[tableau->number_of_costraints];
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        needsArtificialVariables[i] = 0;
        //tableau->is_variable_in_base[i] = NO_VARIABLE;
        //baseVariables[i] = 0;
        if (i <= lessEqualCostraints) {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    //tableau->number_of_slack_variables] = i == y ? 1 : 0;
                    tableau->number_of_slack_variables] = fraction_from_decimal(i == y ? 1 : 0);
            }
            //if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) < 0) {
                // addSlackVariable(const int i)
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i;
            }
        } else if (i <= lessEqualCostraints + equalCostraints) {
            needsArtificialVariables[i] = 1;
            //if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) < 0) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
            }
        } else {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = fraction_from_decimal(i == y ? -1 : 0);
            }
            //if (tableau->table[i][0] > 0) {
            //if (tableau->table[i][0] > -TOLERANCE) {
            if (fraction_compare_with_double(tableau->table[i][0], 0) > 0) {
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i;
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    //tableau->table[i][j] *= -1;
                    tableau->table[i][j] = fraction_multiply(tableau->table[i][j], fraction_from_decimal(-1));
                }
            }
        }
    }
    add_artificial_variables(tableau, needsArtificialVariables);
    return 1;
}

