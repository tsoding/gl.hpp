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

    bool operator==(const String_View &that)
    {
        if (this->count != that.count) return false;
        return memcmp(this->data, that.data, this->count) == 0;
    }

    bool operator==(const String_View &that) const
    {
        if (this->count != that.count) return false;
        return memcmp(this->data, that.data, this->count) == 0;
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

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./summary <todo-file>\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(stderr);
        return -1;
    }

    String_View content = read_whole_file(argv[1]);
    String_Buffer buffer = {};
    buffer.data = (char*) malloc(content.count);
    buffer.capacity = content.count;

    size_t implemented = 0;
    size_t total = 0;

    while (content.count > 0) {
        String_View line = content.chop_by_delim('\n');

        buffer.clean();
        chop_csv_field(&line, ',', '\\', &buffer);
        auto field = buffer.to_view().trim();

        if (field == "+"_sv) implemented += 1;

        total += 1;
    }

    if (total > 0) {
        printf("Implemented %.2f%%\n", (float) implemented / (float) total * 100.0);
    } else {
        printf("Implemented 100.00%%\n");
    }

    return 0;
}
