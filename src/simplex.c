#include <stdio.h>

#define TOTAL_VARIABLES (tableau->number_of_original_variables +\
                                    tableau->number_of_slack_variables +\
                                    tableau->number_of_artificial_variables)

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
    double** table;
    variable_type* type_of_variable;
    int* is_variable_in_base;
    int number_of_original_variables, number_of_costraints,
        number_of_slack_variables, number_of_artificial_variables;
} tableau_format;

result_type simplex(tableau_format* const tableau, double* const result);
result_type simplex_first_phase(tableau_format* const tableau);
result_type simplex_second_phase(tableau_format* const tableau, double* const result);
double simplex_loop(tableau_format* const tableau);
void pivot_on_all_base_variables(tableau_format* const tableau);
int does_not_need_first_phase(tableau_format* const tableau);
int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau);
int createNewTableauFromInput(tableau_format* const tableau);
void print_variables(tableau_format* const tableau);

// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
int main(int argc, char *argv[]) {
    printf("PROVA\n");
    tableau_format tableau;
    double result = 0;
    createNewTableauFromFile("input.dat", &tableau);
    switch (simplex(&tableau, &result)) {
        case NO_SOLUTION: {
            printf("There is no solution for this problem!\n");
            break;
        }
        case UNBOUNDED_SOLUTION: {
            printf("The solution is unbounded!\n");
            break;
        }
        default: {
            print_variables(&tableau);
            printf("Optimal cost: %lf\n", result);
        }
    };
    return 0;
}
// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

result_type simplex(tableau_format* const tableau, double* const result) {
    *result = 0;
    if (simplex_first_phase(tableau) == NO_SOLUTION) {
        return NO_SOLUTION;
    }
    return simplex_second_phase(tableau, result);
}

result_type simplex_first_phase(tableau_format* const tableau) {
    if (does_not_need_first_phase(tableau)) return NORMAL_SOLUTION;
    double backup_objective_function[TOTAL_VARIABLES];

    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
         backup_objective_function[j] = tableau->table[0][j];
         tableau->table[0][j] =
                tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0;
    }
    pivot_on_all_base_variables(tableau);
    if (simplex_loop(tableau) > 0)  return NO_SOLUTION;

    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
         tableau->table[0][j] = backup_objective_function[j];
    // TODO
    return NO_SOLUTION;
}

result_type simplex_second_phase(tableau_format* const tableau, double* const result) {
    return 0.0;
}

double simplex_loop(tableau_format* const tableau) {
    return 0;
}

void pivot_on_all_base_variables(tableau_format* const tableau) {

}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
         if (tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)  return 0;
    return 1;
}

int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau) {
    return 0;
}

int createNewTableauFromInput(tableau_format* const tableau) {
    return 0;
}

// TODO check
void print_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        if (tableau->is_variable_in_base[j])
            printf("x(%zu) = %lf", j, tableau->table[tableau->is_variable_in_base[j]][j]);
    }
}

