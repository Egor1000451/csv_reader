#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <errno.h>

#define SV_IMPLEMENTATION
#include "./sv.h"

#define CSV_DELIM ','

typedef enum {
    CELL_KIND_TEXT = 0,
    CELL_KIND_NUMBER
} Cell_Kind;

typedef union {
    String_View text;
    double number;
} Cell_As;

typedef struct {
    Cell_Kind kind;
    Cell_As as;
} Cell;

typedef struct {
    Cell *cells;
    size_t rows;
    size_t cols;
    Cell *headers;
} Table;

void usage(FILE *stream) {
    fprintf(stream, "Usage: ./csv_reader <input.csv>\n"); 
}

Table table_alloc(size_t rows, size_t cols) {
    Table table = {0};
    table.rows = rows;
    table.cols = cols;
    table.cells = malloc(sizeof(Cell) * rows * cols);
    table.headers = malloc(sizeof(Cell) * cols);
    if (table.cells == NULL) {
        fprintf(stderr, "ERROR: cannot allocate memory for the table\n");
        exit(1);
    }
    memset(table.cells, 0, sizeof(Cell) * rows * cols);
    memset(table.headers, 0, sizeof(Cell) * cols);
    return table;
}

Cell *table_cell_at(Table *table, size_t row, size_t col) {
    assert(row < table->rows);
    assert(col < table->cols);
    return &table->cells[row * table->cols + col];
}

Cell *table_header_at(Table *table, size_t col) {
    assert(col < table->cols);
    return &table->headers[col];
}

char *slurp_file(const char *file_path, size_t *size) {
    // ftell - для однократного выделения памяти для buffer
    // fread - для переноса содержимого файла в buffer
    char *buffer = NULL;
    FILE *f = fopen(file_path, "rb");
    if (f == NULL)
        goto error;

    if (fseek(f, 0, SEEK_END) < 0) 
        goto error;

    long m = ftell(f);
    if (m < 0)
        goto error;

    buffer = malloc(sizeof(char) * m);
    if (buffer == NULL)
        goto error;
    if (fseek(f, 0, SEEK_SET) < 0) 
        goto error;

    size_t n = fread(buffer, 1, m, f);
    assert(n == (size_t) m);

    if (ferror(f))
        goto error;
    if (size)
        *size = n;
    fclose (f);
    return buffer;

error:
    if (f)
        fclose(f);
    if (buffer)
        free(buffer);
    return NULL;
}

void parse_table_from_content(Table *table, String_View content) {
    bool is_header = false;
    for (size_t row = 0; content.count > 0; ++row) {
        if (row == 0) {
            is_header = true;
        } else {
            is_header = false;
        }
        String_View line = sv_chop_by_delim(&content, '\n');
        for (size_t col = 0; line.count > 0; ++col) {
            String_View cell = sv_trim(sv_chop_by_delim(&line, CSV_DELIM));
            table_cell_at(table, row, col)->as.text = cell;
            if (is_header) {
                table_header_at(table, col)->as.text = cell;
            }
        }
    }
}

void estimate_table_size(String_View content, size_t *out_rows, size_t *out_cols) {
    size_t rows = 0;
    size_t cols = 0;
    for (; content.count > 0; ++rows) {
        String_View line = sv_chop_by_delim(&content, '\n');
        size_t col = 0;
        for (; line.count > 0; ++col) {
            sv_chop_by_delim(&line, CSV_DELIM);
        }
        if (cols < col) {
            cols = col;
        }
    }

    if (out_rows)
        *out_rows = rows;
    if (out_cols)
        *out_cols = cols;
}

Table create_table(const char *file_path) {
    size_t content_size = 0;
    char *content = slurp_file(file_path, &content_size);
    if (content == NULL) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    String_View input = {
        .data = content,
        .count = content_size
    };

    size_t rows, cols;
    estimate_table_size(input, &rows, &cols);
    Table table = table_alloc(rows, cols);
    parse_table_from_content(&table, input);
    return table;
}

void table_print(Table *table) {
    for (size_t row = 0; row < table->rows; ++row) {
        for (size_t col = 0; col < table->cols; ++col) {
            printf(SV_Fmt"|", SV_Arg(table_cell_at(table, row, col)->as.text));
        }
        printf("\n");
    }
}

Table table_get_column(Table *table, size_t col) {
    Table taken_col = table_alloc(table->rows, 1);
    for (size_t row = 0; row < table->rows; ++row) {
        taken_col.cells[row] = *table_cell_at(table, row, col);
    }
    return taken_col;
}

Table table_get_row(Table *table, size_t row) {
    Table taken_row = table_alloc(1, table->cols);
    for (size_t col = 0; col < table->cols; ++col) {
        taken_row.cells[col] = *table_cell_at(table, row, col);
    }
    return taken_row;
}

String_View* get_headers(Table *table) {
    String_View *headers = malloc(table->cols * sizeof(String_View));
    for (size_t col = 0; col < table->cols; ++col) {
        headers[col] = table_header_at(table, col)->as.text;
    }
    return headers;
}

void print_headers(Table *table) {
    String_View *headers = get_headers(table); 
    for (size_t col = 0; col < table->cols; ++col) {
        printf("(%zu) "SV_Fmt"\n", col, SV_Arg(headers[col]));
    }
}

bool table_find_header_index(Table *table, String_View name, size_t *out_result) {
    for (size_t i = 0; i < table->cols; ++i) {
        if (table->headers[i].kind == CELL_KIND_TEXT) {
            if (sv_eq(table->headers[i].as.text, name)) {
                if (out_result) *out_result = i;
                return true;
            }
        }
    }
    return false;
}

bool table_get_column_by_name(Table *table, String_View name, Table *out_table) {
    size_t idx;
    bool found = table_find_header_index(table, name, &idx);
    assert(found);
    if (found) {
        *out_table = table_get_column(table, idx);
        return true;
    }
    out_table->cells = NULL;
    out_table->headers = NULL;
    out_table->rows = 0;
    out_table->cols = 0;
    return false;
}

bool select_rows_by_col_val(Table *table, String_View col_name, String_View value, Table *out_table) {
    if (!table || !out_table) return false;

    size_t target_col_idx;
    if (!table_find_header_index(table, col_name, &target_col_idx)) {
        return false;
    }
    printf("target col idx: %zu\n", target_col_idx);

    size_t match_count = 0;
    for (size_t i = 0; i < table->rows; ++i) {
        Cell *cell = table_cell_at(table, i, target_col_idx);
        if (sv_eq(cell->as.text, value)) {
            match_count++;
        }
    }
    printf("Match count: %zu\n", match_count);

    out_table->cols = table->cols;
    out_table->rows = match_count;

    if (match_count == 0) {
        out_table->cells = NULL;
        out_table->headers = NULL;
        return true;
    }

    out_table->cells = calloc(match_count * table->cols, sizeof(Cell));
    out_table->headers = calloc( table->cols, sizeof(Cell));

    if (!out_table->cells || !out_table->headers) {
        free(out_table->cells);
        free(out_table->headers);
        out_table->cells = NULL;
        out_table->headers = NULL;
        return false;
    }

    for (size_t c = 0; c < table->cols; ++c) {
        out_table->headers[c]  = table->headers[c];
    }

    size_t out_row = 0;
    for (size_t i = 0; i < table->rows; ++i) {
        if (sv_eq(table_cell_at(table, i, target_col_idx)->as.text, value)) {
            for (size_t c = 0; c < table->cols; ++c) {
                *table_cell_at(out_table, out_row, c) = *table_cell_at(table, i, c);
            }
            out_row++;
        }
    }

    return true;
}

