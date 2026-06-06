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
    size_t content_size = 0;
    char *content = slurp_file(input_file_path, &content_size);
    if (content == NULL) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n",
                input_file_path, strerror(errno));
        exit(1);
    }

    String_View input = {
        .data = content,
        .count = content_size
    };

    size_t rows, cols;
    estimate_table_size(input, &rows, &cols);
    printf("Size of the table: %zux%zu\n", rows, cols);
    Table table = table_alloc(rows, cols);
    parse_table_from_content(&table, input);

    for (size_t row = 0; row < table.rows; ++row) {
        for (size_t col = 0; col < table.cols; ++col) {
            printf(SV_Fmt"|", SV_Arg(table_cell_at(&table, row, col)->as.text));
        }
        printf("\n");
    }

    Table col2 = table_get_column(&table, 2);
    for (size_t row = 0; row < col2.rows; ++row) {
        printf(SV_Fmt"\n", SV_Arg(table_cell_at(&col2, row, 0)->as.text));
    }
    print_headers(&table);
    printf("----------------------------------------------\n");
    Table col_by_name;
    bool name_was_found = table_get_column_by_name(&table, sv_from_cstr("props"), &col_by_name); 
    if (name_was_found) {
        for (size_t row = 0; row < col_by_name.rows; ++row) {
            printf(SV_Fmt"\n", SV_Arg(table_cell_at(&col_by_name, row, 0)->as.text));
        }
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
            for (size_t row = 0; row < pos_table_seq.rows; ++row) {
                printf(SV_Fmt"\n", SV_Arg(table_cell_at(&pos_table_seq, row, seq_col_idx)->as.text));
            }
        }
    }

    return 0;
}
