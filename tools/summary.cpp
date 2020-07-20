#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>

struct String_View
{
    const char *data;
    size_t count;

    void chop_left(size_t n)
    {
        if (n <= count) {
            data += n;
            count -= n;
        }
    }

    void chop_right(size_t n)
    {
        if (n <= count) {
            count -= n;
        }
    }

    void grow(size_t n)
    {
        count += n;
    }

    String_View chop_by_delim(char delim)
    {
        String_View result = {};
        result.data = data;

        while (count > 0 && *data != delim) {
            chop_left(1);
            result.grow(1);
        }

        if (count > 0 && *data == delim) {
            chop_left(1);
        }

        return result;
    }

    void println(FILE *stream)
    {
        fwrite(data, 1, count, stream);
        putc('\n', stream);
    }

    String_View trim_left() const
    {
        String_View result = *this;
        while (result.count > 0 && isspace(*result.data)) {
            result.chop_left(1);
        }
        return result;
    }

    String_View trim_right() const
    {
        String_View result = *this;
        while (result.count > 0 && isspace(*(result.data + result.count - 1))) {
            result.chop_right(1);
        }
        return result;
    }

    String_View trim() const
    {
        return trim_left().trim_right();
    };

    bool operator==(const String_View &that) const
    {
        if (this->count != that.count) return false;
        return memcmp(this->data, that.data, this->count) == 0;
    }

    bool operator!=(const String_View &that) const
    {
        return !(*this == that);
    }
};

String_View operator ""_sv(const char *data, size_t count)
{
    return {data, count};
}

struct String_Buffer
{
    char *data;
    size_t capacity;
    size_t count;

    void clean()
    {
        count = 0;
    }

    void push(char x)
    {
        assert(count < capacity);
        data[count++] = x;
    }

    void println(FILE *stream)
    {
        fwrite(data, 1, count, stream);
        putc('\n', stream);
    }

    String_View to_view()
    {
        return {data, count};
    }
};

String_View read_whole_file(const char *file_path)
{
    FILE *f = fopen(file_path, "rb");
    assert(f);

    fseek(f, 0, SEEK_END);
    long m = ftell(f);
    char *result = (char *) malloc(m);
    assert(result);
    fseek(f, 0, SEEK_SET);
    fread(result, 1, m, f);

    fclose(f);

    return {result, (size_t) m};
}

void chop_csv_field(String_View *line, char delim, char esc,
                    String_Buffer *field)
{
    while (line->count > 0) {
        if (*line->data == delim) {
            line->chop_left(1);
            return;
        } else if (*line->data == esc && line->count > 1) {
            line->chop_left(1);
            field->push(*line->data);
            line->chop_left(1);
        } else {
            field->push(*line->data);
            line->chop_left(1);
        }
    }
}

template <typename T>
struct Maybe
{
    bool has_value;
    T unwrap;
};

struct Csv
{
    String_View input;
    String_View line;
    String_Buffer buffer;

    bool next_row()
    {
        if (input.count > 0) {
            line = input.chop_by_delim('\n');
            return true;
        }

        return false;
    }

    // TODO: Csv::next_field() invalidates the previous field
    Maybe<String_View> next_field()
    {
        if (line.count > 0) {
            buffer.clean();
            chop_csv_field(&line, ',', '\\', &buffer);
            return {true, buffer.to_view().trim()};
        }

        return {};
    }
};

Csv csv_from_file(const char *file_path)
{
    Csv csv = {};
    csv.input = read_whole_file(file_path);
    csv.buffer.data = (char*) malloc(csv.input.count);
    csv.buffer.capacity = csv.input.count;
    return csv;
}

void usage(FILE *stream)
{
    fprintf(stream,
            "Usage: ./summary <subcommand> <todo-file>\n"
            "Subcommands:\n"
            "    summary      Print the summary of the implementation coverage\n"
            "    done         Print the list of implemented functions\n"
            "    todo         Print the list of not implemented functions\n");
}

String_View string_view_from_cstr(const char *cstr)
{
    return {cstr, strlen(cstr)};
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(stderr);
        return -1;
    }

    String_View subcommand = string_view_from_cstr(argv[1]);
    const char *file_path = argv[2];

    if (subcommand == "summary"_sv) {
        Csv csv = csv_from_file(file_path);

        size_t implemented = 0;
        size_t total = 0;

        while (csv.next_row()) {
            auto field = csv.next_field();
            if (field.unwrap == "+"_sv) implemented += 1;
            total += 1;
        }

        printf("Implemented: %ld/%ld (%.2f%%)\n",
               implemented, total,
               total > 0 ? (float) implemented / (float) total * 100.0 : 100.0);

    } else if (subcommand == "done"_sv) {
        Csv csv = csv_from_file(file_path);

        while (csv.next_row()) {
            auto implemented = csv.next_field();
            assert(implemented.has_value);

            if (implemented.unwrap == "+"_sv) {
                auto name = csv.next_field();
                assert(name.has_value);
                name.unwrap.println(stdout);
            }
        }
    } else if (subcommand == "todo"_sv) {
        Csv csv = csv_from_file(file_path);

        while (csv.next_row()) {
            auto implemented = csv.next_field();
            assert(implemented.has_value);

            if (implemented.unwrap != "+"_sv) {
                auto name = csv.next_field();
                assert(name.has_value);
                name.unwrap.println(stdout);
            }
        }
    }


    return 0;
}
