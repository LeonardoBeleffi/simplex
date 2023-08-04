#include "simplex.h"

result_type simplex(tableau_format* const tableau, double* const result) {
    *result = 0;
    if (simplex_first_phase(tableau) == NO_SOLUTION)  return NO_SOLUTION;
    printf("\n\n Fase 2 \n\n");
    result_type res = simplex_loop(tableau, result);
    *result *= tableau->minimize;
    return res;
}

result_type simplex_first_phase(tableau_format* const tableau) {
    if (does_not_need_first_phase(tableau)) return NORMAL_SOLUTION;
    printf("\n\n Fase 1 \n\n");
    double backup_objective_function[TOTAL_VARIABLES + 1];

    for (size_t j = 0; j <= TOTAL_VARIABLES; j++) {
        backup_objective_function[j] = tableau->table[0][j];
        tableau->table[0][j] =
            tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE ? -1 : 0;
    }

    pivot_on_all_base_variables(tableau);
    double result;
    simplex_loop(tableau, &result);

    if (!IS_ZERO(result)) return NO_SOLUTION;
    exclude_all_artificial_variables_from_base(tableau);
    for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
        tableau->table[0][j] = backup_objective_function[j];
    pivot_on_all_base_variables(tableau);
    return NORMAL_SOLUTION;
}

result_type simplex_loop(tableau_format* const tableau, double* const result) {
    result_type type_of_result = NO_SOLUTION;

    while (type_of_result == NO_SOLUTION) {
        type_of_result = NORMAL_SOLUTION;
        for (size_t j = 1; j <= TOTAL_VARIABLES; j++) {
            if (tableau->table[0][j] <= TOLERANCE || tableau->is_variable_in_base[j]) continue;
            size_t minimum_index = minimum_ratio(tableau, j);
            type_of_result = UNBOUNDED_SOLUTION;

            if (!minimum_index) continue;
            type_of_result = NO_SOLUTION;

            printf("Cost: %24.14lf.\t", tableau->table[0][0]);
            pivot(tableau, minimum_index, j);
            break;
        }
    }

    *result = tableau->table[0][0];
    return type_of_result;
}

size_t minimum_ratio(tableau_format* const tableau, const size_t j) {
    size_t minimum_index = 0;
    double minimum;

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        if (tableau->table[i][j] <= TOLERANCE) continue;
        double ratio = tableau->table[i][0] / tableau->table[i][j];
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
        if (IS_ZERO(ratio - minimum)) {
            for (size_t k = 1; k <= TOTAL_VARIABLES; k++) {
                if (tableau->is_variable_in_base[k] == i) {
                    minimum = ratio;
                    minimum_index = i;
                    break;
                }
            }
        } else if (ratio < minimum - TOLERANCE) {
            minimum = ratio;
            minimum_index = i;
        }
#elif 0
        if (ratio < minimum) {
            minimum = ratio;
            minimum_index = i;
        } else if (IS_ZERO(ratio - minimum)) {
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
    double coefficient = 1.0;
    printf("Pivot on (%zu, %zu)\n", i, j);

    int found = 0;
    for (size_t current_j = 1; current_j <= TOTAL_VARIABLES; current_j++) {
        if (tableau->is_variable_in_base[current_j] == i) {
            tableau->is_variable_in_base[current_j] = 0;
            tableau->is_variable_in_base[j] = i;
            found = 1;
            break;
        }
    }
    assert(found == 1);

    if (!IS_ZERO(tableau->table[i][j] - 1.0)) {
        coefficient = tableau->table[i][j];
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[i][current_j] /= coefficient;
        }
    }

    for (size_t current_i = 0; current_i <= tableau->number_of_costraints; current_i++) {
        coefficient = -tableau->table[current_i][j];
        if (IS_ZERO(coefficient) || current_i == i)  continue;
        for (size_t current_j = 0; current_j <= TOTAL_VARIABLES; current_j++) {
            tableau->table[current_i][current_j] += coefficient * tableau->table[i][current_j];
        }
        if (current_i) {
            assert(tableau->table[current_i][0] >= -TOLERANCE);
        } 
    }
}

void restore_zeroes(tableau_format* const tableau) {
    for (size_t i = 0; i <= tableau->number_of_costraints; i++)
        for (size_t j = 0; j <= TOTAL_VARIABLES; j++)
            if (IS_ZERO(tableau->table[i][j]))  tableau->table[i][j] = 0.0;
}

int does_not_need_first_phase(tableau_format* const tableau) {
    for (size_t j = 1; j <= TOTAL_VARIABLES; j++)
        if (tableau->type_of_variable[j] == ARTIFICIAL_VARIABLE)  return 0;
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

void print_variables(tableau_format* const tableau) {
    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        if (tableau->is_variable_in_base[j])
            printf("x(%zu) = %lf\n", j, tableau->table[tableau->is_variable_in_base[j]][0]);
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

    if ((fdata = fopen(file_name, "r")) == NULL) return 0;

    fscanf(fdata, "%d %d", &(tableau->number_of_original_variables), &(tableau->number_of_costraints));

    int typeOfEquation[tableau->number_of_costraints];
    for (size_t i = 0; i <= tableau->number_of_costraints; i++)
        for (size_t j = 0; j <= tableau->number_of_original_variables; j++)
            tableau->table[i][j] =0.0;

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        fscanf(fdata,"%lf", &(tableau->table[i][0]));
    }

    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        int segno;
        tableau->type_of_variable[i] = NORMAL_VARIABLE;
        fscanf(fdata,"%d", &segno);
        typeOfEquation[i - 1] = segno;
    }

    for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
        double cost;
        fscanf(fdata,"%lf", &cost);
        tableau->table[0][j] = -cost;

        int no;
        fscanf(fdata,"%d", &no);

        int lastVariable = 0;
        for (size_t k = 1; k <= no; k++) {
            fscanf(fdata, "%d", &lastVariable);
            fscanf(fdata, "%lf", &(tableau->table[lastVariable][j]));
        }
    }
    fclose(fdata);
  
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
            scanf("%lf", &(tableau->table[i][j]));
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
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"<=\" costraint number %zu: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
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
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\"=\" costraint number %zu: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
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
                scanf("%lf", &(tableau->table[i][k]));
            }
            printf("Enter the well-known term for the " \
                   "\">=\" costraint number %zu: ", j + 1);
            scanf("%lf", &(tableau->table[i][0]));
            i++;
        }

        tableau->number_of_costraints = lessEqualCostraints + equalCostraints +
            greaterEqualCostraints;
        printf("\n\nTHIS IS THE PROBLEM: \n");
        printf("%s ", tableau->minimize == 1 ? "Min" : "Max");
        for (size_t i = 0; i <= tableau->number_of_costraints; i++) {
            for (size_t j = 1; j <= tableau->number_of_original_variables; j++) {
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
    for (size_t j = 0; j <= tableau->number_of_original_variables; j++) {
        tableau->table[0][j] *= -1 * tableau->minimize;
        tableau->is_variable_in_base[j] = 0;
    }
    tableau->type_of_variable[0] = NO_VARIABLE;
    int needsArtificialVariables[tableau->number_of_costraints];
    for (size_t i = 1; i <= tableau->number_of_costraints; i++) {
        needsArtificialVariables[i] = 0;
        if (i <= lessEqualCostraints) {
            tableau->number_of_slack_variables++;
            for (size_t y = 0; y <= tableau->number_of_costraints; y++) {
                tableau->table[y][tableau->number_of_original_variables +
                    tableau->number_of_slack_variables] = i == y ? 1 : 0;
            }
            if (tableau->table[i][0] < 0) {
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

