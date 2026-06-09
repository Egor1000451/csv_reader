#include <stdio.h>
#include <time.h>

#include "./csv_reader.c"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: input file is not provided\n");
        usage(stderr);
        exit(1);
    }

    const char *input_file_path = argv[1];
    Table table = create_table(input_file_path);

    table_print(&table);
    printf("----------------------------------------------\n");

    Table col2 = table_get_column(&table, 2);
    table_print(&col2);
    printf("----------------------------------------------\n");

    print_headers(&table);
    printf("----------------------------------------------\n");
    Table col_by_name;
    bool name_was_found = table_get_column_by_name(&table, sv_from_cstr("props"), &col_by_name); 
    if (name_was_found) {
        table_print(&col_by_name);
    }

    printf("----------------------------------------------\n");
    Table pos_table_seq;
    bool pos_seq_r = select_rows_by_col_val(&table, sv_from_cstr("target"),
                                            sv_from_cstr("1"),
                                            &pos_table_seq);
    if (pos_seq_r) {
        printf("%zu\n", pos_table_seq.rows);
        size_t seq_col_idx;
        if(table_find_header_index(&table, sv_from_cstr("props"), &seq_col_idx)) {
            // table_print(&pos_table_seq);
            for (size_t row = 0; row < pos_table_seq.rows; ++row) {
                printf(SV_Fmt"\n", SV_Arg(table_cell_at(&pos_table_seq, row, seq_col_idx)->as.text));
            }
        }
    }

    return 0;
}
