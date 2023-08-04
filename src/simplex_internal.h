#ifndef __SIMPLEX_INTERNAL_H__
#define __SIMPLEX_INTERNAL_H__

#define MAX 10000
#define TOLERANCE 1e-10

#define IS_ZERO(x) ((x) < TOLERANCE && (x) > -TOLERANCE)

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
            printf("%10lf %s", tableau->table[i][j], !j ? "| " : "");\
        }\
        printf("\n%s", !i ? "-----------------------------------\n" : "");\
    }\
    printf("\n");\
} while (0);

typedef enum {
    NO_VARIABLE = 0,
    NORMAL_VARIABLE,
    SLACK_VARIABLE,
    ARTIFICIAL_VARIABLE
} variable_type;

static result_type simplex_first_phase(tableau_format* const tableau);
static result_type simplex_loop(tableau_format* const tableau, double* const result);
static size_t minimum_ratio(tableau_format* const tableau, const size_t j);
static void pivot_on_all_base_variables(tableau_format* const tableau);
static size_t has_artificial_variable_in_base(tableau_format* const tableau);
static void exclude_all_artificial_variables_from_base(tableau_format* const tableau);
static void pivot(tableau_format* const tableau, const size_t i, const size_t j);
static void restore_zeroes(tableau_format* const tableau);
static int does_not_need_first_phase(tableau_format* const tableau);
static void add_artificial_variables(tableau_format* const tableau, int* needsArtificialVariables);
static void free_tableau(tableau_format* const tableau);

#endif /* __SIMPLEX_INTERNAL_H__ */

