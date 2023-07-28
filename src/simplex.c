#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX 10000

#define TOTAL_VARIABLES (tableau->number_of_original_variables +\
    tableau->number_of_slack_variables +\
    tableau->number_of_artificial_variables)

#define PRINT_ALL_TABLEAU do {\
    printf("\n");\
    for (size_t i = 0; i <= tableau->number_of_costraints; i++) {\
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {\
            printf("%lf %s", tableau->table[i][j], !j ? "| " : "");\
        }\
        printf("\n%s", !i ? "-----------------------------------\n" : "");\
    }\
    printf("\n");\
} while(0);

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
    double(* table)[MAX];
    variable_type type_of_variable[MAX];
    int is_variable_in_base[MAX];
    int number_of_original_variables, number_of_costraints,
    number_of_slack_variables, number_of_artificial_variables;
    int minimize;
} tableau_format;

result_type simplex(tableau_format* const tableau, double* const result);
result_type simplex_first_phase(tableau_format* const tableau);
result_type simplex_second_phase(tableau_format* const tableau, double* const result);
result_type simplex_loop(tableau_format* const tableau, double* const result);
void pivot_on_all_base_variables(tableau_format* const tableau);
size_t has_artificial_variable_in_base(tableau_format* const tableau);
void exclude_all_artificial_variables_from_base(tableau_format* const tableau);
void pivot(tableau_format* const tableau, const size_t i, const size_t j);
int does_not_need_first_phase(tableau_format* const tableau);
void add_artificial_variables(tableau_format* const tableau, int* needsArtificialVariables);
void print_variables(tableau_format* const tableau);
void free_tableau(tableau_format* const tableau);
int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau);
int createNewTableauFromInput(tableau_format* const tableau);

// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
int main(int argc, char *argv[]) {
    tableau_format tableau;
    double result = 0;
    //createNewTableauFromFile("input.dat", &tableau);
    createNewTableauFromInput(&tableau);
#if 1
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
    if (does_not_need_first_phase(tableau)) return NORMAL_SOLUTION;
    double backup_objective_function[TOTAL_VARIABLES];

    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
        printf("%lf ", tableau->table[0][j]);
        backup_objective_function[j] = tableau->table[0][j];
        tableau->table[0][j] =
            tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0;
    }
    pivot_on_all_base_variables(tableau);
    double result;
    simplex_loop(tableau, &result);
    if (result > 0) return NO_SOLUTION;
    exclude_all_artificial_variables_from_base(tableau);
    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
        tableau->table[0][j] = backup_objective_function[j];
    // TODO
    return NORMAL_SOLUTION;
}

result_type simplex_second_phase(tableau_format* const tableau, double* const result) {
    return simplex_loop(tableau, result);
}

result_type simplex_loop(tableau_format* const tableau, double* const result) {
    int is_over = 0, is_unbounded;
    while (!is_over) {
        is_over = 1;
        is_unbounded = 1;
        for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
            size_t minimum_index = 0;
            double minimum = -1;
            if (tableau->table[0][j] > 0) {
                is_over = 0;
                // TODO function
                for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
                    if (tableau->table[i][j] <=0) {
                        continue;
                    }
                    double ratio = tableau->table[i][0] / tableau->table[i][j];
                    if (minimum < 0 || ratio < minimum) {
                        minimum = ratio;
                        minimum_index = i;
                    }
                }
            }
            if (minimum >= 0) {
                printf("pivot_on_all_base_variables %d %zu\n", tableau->is_variable_in_base[j], j);
                pivot(tableau, minimum_index, j);
                is_unbounded = 0;

            }
        }
        // TODO add pivoting and unbounded checks
    }
    *result = tableau->table[0][0];
    return is_unbounded ? UNBOUNDED_SOLUTION : NORMAL_SOLUTION;
}

void pivot_on_all_base_variables(tableau_format* const tableau) {
    printf("pivot on all base\n");
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
        if (tableau->is_variable_in_base[j]) {
            pivot(tableau, tableau->is_variable_in_base[j], j);
        }
    }
}

size_t has_artificial_variable_in_base(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
        if (tableau->is_variable_in_base[j] &&
            tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)
            return j;
    }
    return 0;
}

void exclude_all_artificial_variables_from_base(tableau_format* const tableau) {
    size_t j;
    int non_artificial_variables = TOTAL_VARIABLES - tableau->number_of_artificial_variables;

    while ((j = has_artificial_variable_in_base(tableau))) {
        int found_pivotable_variable = 0;
        size_t i = tableau->is_variable_in_base[j];
        found_pivotable_variable = 0;
        for (size_t current_j = 1; current_j <= non_artificial_variables; current_j++) {
            if (tableau->table[i][current_j]) {
                found_pivotable_variable = 1;
                pivot(tableau, i, current_j);
            }
        }
        if (!found_pivotable_variable) {
            // remove row and column
            for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
                tableau->table[i][current_j] = 0;
            }
            for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++) {
                tableau->table[current_i][j] = 0;
            }
            tableau->is_variable_in_base[j] = 0;
        }
    }
    tableau->number_of_artificial_variables = 0;
}

void pivot(tableau_format* const tableau, const size_t i, const size_t j) {
    double coefficient = 1;
    printf("(%d, %zu) -> %lf\n", tableau->is_variable_in_base[j], j, tableau->table[i][j]);
    printf("pivot on (%zu, %zu)\n", i, j);
    assert(tableau->table[i][j]);
    for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
        if (tableau->is_variable_in_base[current_j] == i){
            tableau->is_variable_in_base[current_j] = 0;
            tableau->is_variable_in_base[j] = i;
            break;
        }
    }
    printf("(%d, %zu) -> %lf\n", tableau->is_variable_in_base[j], j, tableau->table[i][j]);
    if (tableau->table[i][j] != 1) {
        // TODO (even pivot)
        coefficient = 1.0 / tableau->table[i][j];
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[i][current_j] *= coefficient;
        }
    }
    for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++) {
        if (tableau->table[current_i][j] == 0 || current_i == i)  continue;
        coefficient = -tableau->table[current_i][j];
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[current_i][current_j] += coefficient * tableau->table[i][current_j];
        }
    }
}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)  return 0;
    printf("DoesNotNEED\n");
    return 1;
}

void add_artificial_variables(tableau_format* const tableau, int* needsArtificialVariables) {
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        if (needsArtificialVariables[i]) {
            tableau->number_of_artificial_variables++;
            tableau->type_of_variable[TOTAL_VARIABLES] = ARTIFICIAL_VARIABLE;
            tableau->table[0][TOTAL_VARIABLES] = -1;
            tableau->is_variable_in_base[TOTAL_VARIABLES] = i;
            for (size_t k = 1; k <= tableau->number_of_costraints; k++) {
                tableau->table[k][TOTAL_VARIABLES] = k == i ? 1 : 0;
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

void free_tableau(tableau_format* const tableau) {
    free(tableau->table);
}

int createNewTableauFromFile(const char* file_name,
                             tableau_format* const tableau) {
    if (tableau == NULL)  return 0;
    tableau->minimize = 1;
    return 0;
}

int createNewTableauFromInput(tableau_format* const tableau) {
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

        do {
            printf("\nInsert 1 to reinsert, 0 to proceed with the inserted data: ");
            scanf("%d", &wantToReinsert);
        } while (wantToReinsert && wantToReinsert != 1);
    } while (wantToReinsert);
    // initialization -> all disequations to <=, and add slack variablesNumber
    for (int j = 0; j <= tableau->number_of_original_variables; j++) {
        tableau->table[0][j] *= -1 * tableau->minimize;
        tableau->is_variable_in_base[j] = 0;
        //baseVariables[i] = 0;
    }
    tableau->type_of_variable[0] = NO_VARIABLE;
    int needsArtificialVariables[tableau->number_of_costraints];
    for (int i = 1; i <= tableau->number_of_costraints; i++) {
        needsArtificialVariables[i] = 0;
        //tableau->is_variable_in_base[i] = NO_VARIABLE;
        //baseVariables[i] = 0;
        if (i <= lessEqualCostraints) {
            tableau->number_of_slack_variables++;
            for (int y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i == y ? 1 : 0;
            }
            if (tableau->table[i][0] < 0) {
                // addSlackVariable(const int i)
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
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
            if (tableau->table[i][0] < 0) {
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        } else {
            tableau->number_of_slack_variables++;
            for (int y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i == y ? -1 : 0;
            }
            if (tableau->table[i][0] > 0) {
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i;
                for (int j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        }
    }
    add_artificial_variables(tableau, needsArtificialVariables);
    return 1;
}

