#include <stdio.h>
#include <stdlib.h>

#define MAX 10000

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
    int minimize;
} tableau_format;

result_type simplex(tableau_format* const tableau, double* const result);
result_type simplex_first_phase(tableau_format* const tableau);
result_type simplex_second_phase(tableau_format* const tableau, double* const result);
double simplex_loop(tableau_format* const tableau);
void pivot_on_all_base_variables(tableau_format* const tableau);
int does_not_need_first_phase(tableau_format* const tableau);
int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau);
void createNewTableauFromInput(tableau_format* const tableau);
void print_variables(tableau_format* const tableau);
void free_tableau(tableau_format* const tableau);

// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
int main(int argc, char *argv[]) {
    tableau_format tableau;
    double result = 0;
    //createNewTableauFromFile("input.dat", &tableau);
    createNewTableauFromInput(&tableau);
#if 0
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
#endif
    free_tableau(&tableau);
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
    printf("PROVA\n");
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
    return UNBOUNDED_SOLUTION;
}

double simplex_loop(tableau_format* const tableau) {
    return 0.0;
}

void pivot_on_all_base_variables(tableau_format* const tableau) {

}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
         if (tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)  return 0;
    return 1;
}

int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau) {
    return 0;
}

void createNewTableauFromInput(tableau_format* const tableau) {
    int lessEqualCostraints = 0, equalCostraints = 0,
        greaterEqualCostraints = 0, wantToReinsert, i;

    tableau->table = (double**) malloc(MAX*MAX*sizeof(double));
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

        tableau->table[0][0] = 0;
        for (int j = 1; j <= tableau->number_of_original_variables; j++) {
            printf("Enter the coefficient of " \
                        "variable number %d: ", j);
            scanf("%lf", &(tableau->table[i][j]));
        }
        i++;

        do {
            printf("Insert how many <= costraints this problem has: \n");
            scanf("%d", &lessEqualCostraints);
        } while (lessEqualCostraints < 0);
        for (int j = 0; j < lessEqualCostraints; j++) {
            for (int k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %d in " \
                       "\"<=\" costraint number %d: ", k, i);
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"<=\" costraint number %d: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
            i++;
        }

        do {
            printf("Insert how many = costraints this problem has: \n");
            scanf("%d", &equalCostraints);
        } while (equalCostraints < 0);
        for (int j = 0; j < equalCostraints; j++) {
            for (int k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %d in " \
                       "\"=\" costraint number %d: ", k, i);
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"=\" costraint number %d: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
            i++;
        }

        do {
            printf("Insert how many >= costraints this problem has: \n");
            scanf("%d", &greaterEqualCostraints);
        } while (greaterEqualCostraints < 0);
        for (int j = 0; j < greaterEqualCostraints; j++) {
            for (int k = 1; k <= tableau->number_of_original_variables; k++) {
                printf("Enter the coefficient of variable number %d in " \
                       "\">=\" costraint number %d: ", k, i);
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\">=\" costraint number %d: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
            i++;
        }

        tableau->number_of_costraints = lessEqualCostraints + equalCostraints +
            greaterEqualCostraints;
        printf("\n\nTHIS IS THE PROBLEM: \n");
        printf("%s ", tableau->minimize == 1 ? "Min" : "Max");
        for (int i = 0; i <= tableau->number_of_costraints; i++) {
            for (int j = 1; j <= tableau->number_of_original_variables; j++) {
                printf("%lf ", tableau->table[i][j]);
            }
            if (i) {
                printf("%s %lf", i <= lessEqualCostraints ? "<=" :
                       i <= lessEqualCostraints + equalCostraints ? "=" : ">=",
                       tableau->table[i][0]);
            }
            printf("\n%s", !i ? "s.t. " : "     ");
        }
        printf("\n");

        printf("\nInsert 1 to reinsert, 0 to proceed with the inserted data: ");
        scanf("%d", &wantToReinsert);
    } while (wantToReinsert);
    // initialization -> all disequations to <=, and add slack variablesNumber
    for (int j = 0; j <= tabvariablesNumber; j++) {
        tableau->table[0][j] *= -1 * cost;
        baseVariables[i] = 0;
    }
    tableau->variable_type[0] = NO_VARIABLE;
    for (int i = 1; i <= costraintsNumber; i++) {
        needsArtificialVariables[i] = 0;
        baseVariables[i] = 0;
        if (i <= lessEqualCostraints) {
            slackVariables++;
            for (int y = 0; y <= costraintsNumber; y++) {
                tableau[y][variablesNumber + slackVariables] = i == y ? 1 : 0;
            }
            if (tableau->table[i][0] < 0) {
                // addSlackVariable(const int i)
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
                needsArtificialVariables[i] = 1;
                baseVariables[variablesNumber + slackVariables] = 0;
            } else {
                baseVariables[variablesNumber + slackVariables] = i;
            }
        } else if (i <= lessEqualCostraints + equalCostraints) {
            needsArtificialVariables[i] = 1;
            if (tableau->table[i][0] < 0) {
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        } else {
            slackVariables++;
            for (int y = 0; y <= costraintsNumber; y++) {
                tableau[y][variablesNumber + slackVariables] = i == y ? -1 : 0;
            }
            if (tableau->table[i][0] > 0) {
                needsArtificialVariables[i] = 1;
                baseVariables[variablesNumber + slackVariables] = 0;
            } else {
                baseVariables[variablesNumber + slackVariables] = i;
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        }
    }
}

// TODO check
void print_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        if (tableau->is_variable_in_base[j])
            printf("x(%zu) = %lf", j, tableau->table[tableau->is_variable_in_base[j]][j]);
    }
}

void print_variables(tableau_format* const tableau) {
    free(tableau->table);
}
