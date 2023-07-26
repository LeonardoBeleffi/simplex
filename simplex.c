#include <stdio.h>

typedef struct tableau_format {
    double **table;
    int number_of_variables, number_of_costraints;
} tableau_format;

int createNewTableauFromFile(const char* file_name, tableau_format* const tableau);

int createNewTableauFromInput(tableau_format* const tableau);

double simplex(tableau_format* const tableau);

void simplex_first_phase(tableau_format* const tableau);

double simplex_second_phase(tableau_format* const tableau);

int doesNotNeedFirstPhase(tableau_format* const tableau);

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
    if (doesNotNeedFirstPhase(tableau)) return;
}

double simplex_second_phase(tableau_format* const tableau) {
    return 0.0;
}

int doesNotNeedFirstPhase(tableau_format* const tableau) {
    return 0;
}

int createNewTableauFromFile(const char* file_name, tableau_format* const tableau) {
    return 0;
}

int createNewTableauFromInput(tableau_format* const tableau) {
    return 0;
}

