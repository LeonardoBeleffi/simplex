#ifndef __SIMPLEX_H__
#define __SIMPLEX_H__

#define MAX 10000

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
void print_variables(tableau_format* const tableau);
void free_tableau(tableau_format* const tableau);
int create_tableau_from_file(tableau_format* const tableau, const char* file_name);
int create_tableau_from_input(tableau_format* const tableau);

#endif /* __SIMPLEX_H__ */

