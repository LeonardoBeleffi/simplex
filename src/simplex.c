#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX 10000
#define TOLERANCE 1e-10
//#define TOLERANCE 0.00001

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
        printf("%10Lf ", (long double)j);\
    printf("\n\n");\
    for (size_t i = 0; i <= tableau->number_of_costraints; i++) {\
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {\
            printf("%10Lf %s", tableau->table[i][j], !j ? "| " : "");\
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
    long double(* table)[MAX];
    variable_type type_of_variable[MAX];
    int is_variable_in_base[MAX];
    int number_of_original_variables, number_of_costraints,
    number_of_slack_variables, number_of_artificial_variables;
    int minimize;
} tableau_format;

result_type simplex(tableau_format* const tableau, long double* const result);
result_type simplex_first_phase(tableau_format* const tableau);
result_type simplex_second_phase(tableau_format* const tableau, long double* const result);
result_type simplex_loop(tableau_format* const tableau, long double* const result);
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
    long double result = 0;
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
            printf("There is no solution for this problem!\n");
            break;
        }
        case UNBOUNDED_SOLUTION: {
            printf("The solution is unbounded!\n");
            break;
        }
        default: {
            printf("Found optimal solution!\n");
            printf("\nOptimal cost: %Lf\n\nNon zero variables:\n\n", result);
            print_variables(&tableau);
        }
    };
    free_tableau(&tableau);
    return 0;
}
// REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

result_type simplex(tableau_format* const tableau, long double* const result) {
    *result = 0;
    if (simplex_first_phase(tableau) == NO_SOLUTION)  return NO_SOLUTION;
    printf("\n\n Fase 2 \n\n");
    result_type res = simplex_second_phase(tableau, result);
    *result *= tableau->minimize;
    //PRINT_ALL_TABLEAU
    return res;
}

result_type simplex_first_phase(tableau_format* const tableau) {
    if (does_not_need_first_phase(tableau)) return NORMAL_SOLUTION;
    printf("\n\n Fase 1 \n\n");
    long double backup_objective_function[TOTAL_VARIABLES + 1];
    //PRINT_ALL_TABLEAU
    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
//        printf("%Lf ", tableau->table[0][j]);
        backup_objective_function[j] = tableau->table[0][j];
        tableau->table[0][j] =
            tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0;
    }
    //PRINT_ALL_TABLEAU

    pivot_on_all_base_variables(tableau);
    long double result;
    simplex_loop(tableau, &result);
    //printf("\nTableau ottimo fase 1\n");
    //PRINT_ALL_TABLEAU
    //if (result > 0.0) return NO_SOLUTION;
    if (!IS_ZERO(result)) return NO_SOLUTION;
    exclude_all_artificial_variables_from_base(tableau);
    // TODO maybe function
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
        tableau->table[0][j] = backup_objective_function[j];
    // TODO
    // TODO assert input dimension
    pivot_on_all_base_variables(tableau);
    return NORMAL_SOLUTION;
}

result_type simplex_second_phase(tableau_format* const tableau, long double* const result) {
    return simplex_loop(tableau, result);
}

result_type simplex_loop(tableau_format* const tableau, long double* const result) {
    result_type type_of_result = NO_SOLUTION;

            //PRINT_ALL_TABLEAU
    while (type_of_result == NO_SOLUTION) {
        type_of_result = NORMAL_SOLUTION;
        for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
            if (tableau->table[0][j] <= TOLERANCE || tableau->is_variable_in_base[j]) continue;
            //printf("row 0: %Lf\n", tableau->table[0][j]);
            //if (tableau->table[0][j] <= 0) continue;
            //if (tableau->is_variable_in_base[j])  continue;
            //printf("%24.14Lf\n", tableau->table[0][j]);
            size_t minimum_index = minimum_ratio(tableau, j);
            type_of_result = UNBOUNDED_SOLUTION;

            //PRINT_ALL_TABLEAU
            //printf("index = %zu\n", minimum_index);
            if (!minimum_index) continue;
            type_of_result = NO_SOLUTION;

            printf("Cost: %24.14Lf.\t", tableau->table[0][0]);
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
    long double minimum;

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        if (tableau->table[i][j] <= TOLERANCE) continue;
        long double ratio = tableau->table[i][0] / tableau->table[i][j];
        //printf("Qui: %Lf\n", ratio);
        //printf("ratio = %24.14Lf / %24.14lf\n", tableau->table[i][0], tableau->table[i][j]);
        //printf("ratio: %24.14Lf\n", ratio);
#if 1
        if(IS_ZERO(ratio))
            return i;
        else if (!minimum_index || ratio < minimum + TOLERANCE) {
            minimum = ratio;
            minimum_index = i;
        }
#else
        if (!minimum_index) {
            minimum = ratio;
            minimum_index = i;
            continue;
        }
#endif
#if 0
        //printf("%24.14Lf %24.14lf\n", ratio, minimum);
        if (IS_ZERO(ratio - minimum)) {
            //printf("SOPRA\n");
            //printf("Ma anche qui: \n");
            for (size_t k = 1; k <= TOTAL_VARIABLES; k++) {
                //if (tableau->is_variable_in_base[k] == i && (!exiting_variable_index || k < exiting_variable_index)) {
                if (tableau->is_variable_in_base[k] == i) {
                    minimum = ratio;
                    minimum_index = i;
                    break;
                }
            }
        //} else if (ratio < minimum + TOLERANCE) {
        } else if (ratio < minimum - TOLERANCE) {
            //printf("SOTTO\n");
            //printf("Qui: %Lf\n", ratio);
            minimum = ratio;
            minimum_index = i;
        }
#elif 0
        //if (minimum_index == 0 || (ratio - minimum) < TOLERANCE) {
        //if (ratio - minimum < TOLERANCE) {
        if (ratio < minimum) {
            //printf("Qui: %Lf\n", ratio);
            minimum = ratio;
            minimum_index = i;
        } else if (IS_ZERO(ratio - minimum)) {
            //printf("Ma anche qui: \n");
            for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
                if (tableau->is_variable_in_base[j] == i && (!exiting_variable_index || j < exiting_variable_index)) {
                    exiting_variable_index = j;
                    minimum = ratio;
                    minimum_index = i;
                    break;
                }
            }
        }
#endif
    }
    //printf("Minimum: (%zu, %zu) -> %Lf\n", minimum_index, minimum);
    return minimum_index;
}

void pivot_on_all_base_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->is_variable_in_base[j])
            pivot(tableau, tableau->is_variable_in_base[j], j);
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

    while ((j = has_artificial_variable_in_base(tableau))) {
        int found_pivotable_variable = 0;
        size_t i = tableau->is_variable_in_base[j];
        found_pivotable_variable = 0;
        for (size_t current_j = 1; current_j <= non_artificial_variables; current_j++) {
            if (!IS_ZERO(tableau->table[i][current_j])) {
                found_pivotable_variable = 1;
                pivot(tableau, i, current_j);
            }
        }
        if (!found_pivotable_variable) {
            for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++)
                tableau->table[i][current_j] = 0;
            for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++)
                tableau->table[current_i][j] = 0;
            tableau->is_variable_in_base[j] = 0;
        }
    }
    tableau->number_of_artificial_variables = 0;
}

void pivot(tableau_format* const tableau, const size_t i, const size_t j) {
    long double coefficient = 1.0;
    printf("Pivot on (%zu, %zu)\n", i, j);

    int found = 0;
    // swap variables in base
    //getc(stdin);
#if 0
    int prova1 = 0, prova2 = 0;
    for (size_t current_j = 1; current_j <= TOTAL_VARIABLES; current_j++)
        if (tableau->is_variable_in_base[current_j])
            prova1++;
#endif
    for (size_t current_j = 1; current_j <= TOTAL_VARIABLES; current_j++) {
        if (tableau->is_variable_in_base[current_j] == i) {
            tableau->is_variable_in_base[current_j] = 0;
            tableau->is_variable_in_base[j] = i;
            found = 1;
            break;
        }
    }
#if 0
    for (size_t current_j = 1; current_j <= TOTAL_VARIABLES; current_j++)
        if (tableau->is_variable_in_base[current_j])
            prova2++;
    assert(prova1 == prova2);
#endif
    assert(found == 1);

    if (!IS_ZERO(tableau->table[i][j] - 1.0)) {
        coefficient = tableau->table[i][j];
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[i][current_j] /= coefficient;
        }
    }

    //restore_zeroes(tableau);
    for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++) {
        coefficient = -tableau->table[current_i][j];
        if (IS_ZERO(coefficient) || current_i == i)  continue;
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[current_i][current_j] += coefficient * tableau->table[i][current_j];
        }
        //printf("Pivot coefficient for line %zu = %Lf\n", current_i, coefficient);
        //printf("%Lf += %lf * %lf)\n", tableau->table[current_i][0], coefficient, tableau->table[i][0]);
        if (current_i) {
            //printf("%Lf\n", tableau->table[current_i][0]);
            assert(tableau->table[current_i][0] >= -TOLERANCE);
        } 
    }
    //restore_zeroes(tableau);
}

void restore_zeroes(tableau_format* const tableau) {
    for (size_t i = 0; i <= tableau->number_of_costraints; i++)
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
            if (IS_ZERO(tableau->table[i][j]))  tableau->table[i][j] = 0.0;
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
            printf("x(%zu) = %Lf\n", j, tableau->table[tableau->is_variable_in_base[j]][0]);
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
            tableau->table[i][j] =0.0;

    // Leggi il vettore dei termini noti
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        //long double cost;
        //fscanf(fdata,"%Lf", &cost);
        //tableau->table[0][j] = -cost;
        fscanf(fdata,"%Lf", &(tableau->table[i][0]));
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
        long double cost;
        fscanf(fdata,"%Lf", &cost);
        tableau->table[0][j] = -cost;
        //fscanf(fdata,"%Lf", &(tableau->table[i][0]));

        int no;
        // Leggi il numero di coefficienti non-zero della colonna j
        fscanf(fdata,"%d", &no);

        int lastVariable = 0;
        // Leggi i coefficienti non-zero della colonna j
        for (size_t k = 1; k <= no; k++) {
            fscanf(fdata, "%d", &lastVariable);
            fscanf(fdata, "%Lf", &(tableau->table[lastVariable][j]));
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
                tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = i == y ? 1 : 0;
            }
            if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = i;
            }
        } else if (typeOfEquation[i - 1] == 0) {
                needsArtificialVariables[i] = 1;
            if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        } else {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables + tableau->number_of_slack_variables] = i == y ? -1 : 0;
            }
            if (tableau->table[i][0] > 0) {
            //if (tableau->table[i][0] > -TOLERANCE) {
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables + tableau->number_of_slack_variables] = i;
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        }
    }
    //PRINT_ALL_TABLEAU
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

        tableau->table[0][0] = 0;
        for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
            printf("Enter the coefficient of " \
                   "variable number %zu: ", j);
            scanf("%Lf", &(tableau->table[i][j]));
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
                scanf("%Lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"<=\" costraint number %zu: ", j + 1);
            scanf("%Lf", &(tableau->table[i][0]));
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
                scanf("%Lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"=\" costraint number %zu: ", j + 1);
            scanf("%Lf", &(tableau->table[i][0]));
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
                scanf("%Lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\">=\" costraint number %zu: ", j + 1);
            scanf("%Lf", &(tableau->table[i][0]));
            i++;
        }

        tableau->number_of_costraints = lessEqualCostraints + equalCostraints +
            greaterEqualCostraints;
        printf("\n\nTHIS IS THE PROBLEM: \n");
        printf("%s ", tableau->minimize == 1 ? "Min" : "Max");
        for (size_t i = 0; i <= tableau->number_of_costraints; i++) {
            for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
                printf("%Lf ", tableau->table[i][j]);
            }
            if (i) {
                printf("%s %Lf", i <= lessEqualCostraints ? "<=" :
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
    for (size_t j = 0; j <= tableau->number_of_original_variables; j++) {
        tableau->table[0][j] *= -1 * tableau->minimize;
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
                    tableau->number_of_slack_variables] = i == y ? 1 : 0;
            }
            if (tableau->table[i][0] < 0) {
            //if (tableau->table[i][0] < TOLERANCE) {
                // addSlackVariable(const int i)
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
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
            //if (tableau->table[i][0] < TOLERANCE) {
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        } else {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i == y ? -1 : 0;
            }
            if (tableau->table[i][0] > 0) {
            //if (tableau->table[i][0] > -TOLERANCE) {
                needsArtificialVariables[i] = 1;
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = 0;
            } else {
                tableau->is_variable_in_base[tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i;
                for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
                    tableau->table[i][j] *= -1;
                }
            }
        }
    }
    add_artificial_variables(tableau, needsArtificialVariables);
    return 1;
}

