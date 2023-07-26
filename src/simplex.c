#include <stdio.h>

#define TOTAL_VARIABLES (tableau->number_of_original_variables +\
                                    tableau->number_of_slack_variables +\
                                    tableau->number_of_artificial_variables)

typedef enum {
    NONE = 0,
    NORMAL,
    SLACK,
    ARTIFICIAL
} variable_type;

typedef struct {
    double **table;
    variable_type *type_of_variable;
    int number_of_original_variables, number_of_costraints,
        number_of_slack_variables, number_of_artificial_variables;
} tableau_format;

double simplex(tableau_format* const tableau);
void simplex_first_phase(tableau_format* const tableau);
double simplex_second_phase(tableau_format* const tableau);
double simplex_loop(tableau_format* const tableau);
void pivot_on_all_base_variables(tableau_format* const tableau);
int does_not_need_first_phase(tableau_format* const tableau);
int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau);
int createNewTableauFromInput(tableau_format* const tableau);

// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
int main(int argc, char *argv[]) {
    printf("PROVA\n");
    tableau_format tableau;
    createNewTableauFromFile("input.dat", &tableau);
    double result = simplex(&tableau);
    printf("Optimal cost: %lf\n", result);
    return 0;
}
// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

double simplex(tableau_format* const tableau) {
    simplex_first_phase(tableau);
    return simplex_second_phase(tableau);
}

void simplex_first_phase(tableau_format* const tableau) {
    if (does_not_need_first_phase(tableau)) return;
    double backup_objective_function[TOTAL_VARIABLES];
    for (int j = 0; j <= TOTAL_VARIABLES; j++) {
         backup_objective_function[j] = tableau->table[0][j];
         tableau->table[0][j] =
                tableau->type_of_variable[j] == ARTIFICIAL ? -1 : 0;
    }
    pivot_on_all_base_variables(tableau);
    simplex_loop(tableau);
    // TODO
    for (int j = 0; j <= TOTAL_VARIABLES; j++)
         tableau->table[0][j] = backup_objective_function[j];
}

double simplex_second_phase(tableau_format* const tableau) {
    return 0.0;
}

double simplex_loop(tableau_format* const tableau) {

}

void pivot_on_all_base_variables(tableau_format* const tableau) {

}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (int j = 0; j <= TOTAL_VARIABLES, j++)
         if (tableau->type_of_variable[j] == ARTIFICIAL)  return 0;
    return 1;
}

int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau) {
    return 0;
}

int createNewTableauFromInput(tableau_format* const tableau) {
    return 0;
}

