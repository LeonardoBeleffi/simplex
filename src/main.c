#include <stdio.h>
#include "simplex.h"

int main(int argc, char *argv[]) {
    tableau_format tableau;
    double result = 0;
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
            printf("\nOptimal cost: %lf\n\nNon zero variables:\n\n", result);
            print_variables(&tableau);
        }
    };
    free_tableau(&tableau);
    return 0;
}

