#include "aids.hpp"

using namespace aids;

struct String_Buffer
{
    size_t capacity;
    size_t count;
    char *data;

    void clean()
    {
        count = 0;
    }

    bool push(char x)
    {
        if (count < capacity) {
            data[count++] = x;
            return true;
        }

        return false;
    }

    String_View to_view()
    {
        return {count, data};
    }

    char *end()
    {
        return data + count;
    }
};

String_View chop_csv_field(String_View *line, char delim, char esc,
                           String_Buffer *buffer)
{
    String_View field = {0, buffer->end()};

    while (line->count > 0) {
        if (*line->data == delim) {
            line->chop(1);
            return field;
        } else if (*line->data == esc && line->count > 1) {
            line->chop(1);
            { bool ok = buffer->push(*line->data); assert(ok); }
            field.grow(1);
            line->chop(1);
        } else {
            { bool ok = buffer->push(*line->data); assert(ok); }
            field.grow(1);
            line->chop(1);
        }
    }

    return field;
}

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
            return {true, chop_csv_field(&line, ',', '\\', &buffer).trim()};
        }

        return {};
    }
};

Csv csv_from_file(const char *file_path)
{
    auto file_content = read_file_as_string_view(file_path);
    assert(file_content.has_value);

    Csv csv = {};
    csv.input = file_content.unwrap;
    csv.buffer.data = (char*) malloc(csv.input.count);
    assert(csv.buffer.data);
    csv.buffer.capacity = csv.input.count;
    return csv;
}

void usage(FILE *stream)
{
    println(stream,
            "Usage: ./coverage <subcommand> <subcommand-args>\n"
            "Subcommands:\n"
            "    summary <todo-filepath>          Print the summary of the implementation coverage.\n"
            "    done <todo-filepath>             Print the list of implemented functions.\n"
            "    todo <todo-filepath>             Print the list of not implemented functions.\n"
            "    help                             Print print this help text to stdout.");
}

void summary_subcommand(const char *file_path)
{
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
}

void done_subcommand(const char *file_path)
{
    Csv csv = csv_from_file(file_path);

    for (size_t line = 1; csv.next_row(); ++line) {
        auto implemented = csv.next_field();
        auto name = csv.next_field();
        assert(implemented.has_value);
        assert(name.has_value);

        if (implemented.unwrap == "+"_sv) {
            println(stdout, file_path, ":", line, ": ", name.unwrap);
        }
    }
}

void todo_subcommand(const char *file_path)
{
    Csv csv = csv_from_file(file_path);

    for (size_t line = 1; csv.next_row(); ++line) {
        auto implemented = csv.next_field();
        auto name = csv.next_field();
        assert(implemented.has_value);
        assert(name.has_value);

        if (implemented.unwrap != "+"_sv) {
            println(stdout, file_path, ":", line, ": ", name.unwrap);
        }
    }
}

struct Args
{
    int argc;
    char **argv;

    bool empty()
    {
        return argc == 0;
    }

    const char *pop()
    {
        assert(!empty());
        const char *result = *argv;
        argc -= 1;
        argv += 1;
        return result;
    }
};

int main(int argc, char *argv[])
{
    Args args = {argc, argv};
    args.pop();

    if (args.empty()) {
        println(stderr, "[ERROR] subcommand is not provided");
        usage(stderr);
        return -1;
    }

    String_View subcommand = cstr_as_string_view(args.pop());

    if (subcommand == "summary"_sv) {
        if (args.empty()) {
            println(stderr, "[ERROR] filepath is not provided for todo subcommand");
            usage(stderr);
            return -1;
        }

        summary_subcommand(args.pop());
    } else if (subcommand == "done"_sv) {
        if (args.empty()) {
            println(stderr, "[ERROR] filepath is not provided for done subcommand");
            usage(stderr);
            return -1;
        }

        done_subcommand(args.pop());
    } else if (subcommand == "todo"_sv) {
        if (args.empty()) {
            println(stderr, "[ERROR] filepath is not provided for todo subcommand");
            usage(stderr);
            return -1;
        }

        todo_subcommand(args.pop());
    } else if (subcommand == "help"_sv){
        usage(stdout);
    } else {
        println(stderr, "[ERROR] unknown subcommand `", subcommand, "`");
        usage(stderr);
        return -1;
    }

    return 0;
}
