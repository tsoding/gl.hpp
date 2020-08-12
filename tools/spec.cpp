#include <set>
#include <map>
#include "aids.hpp"
#include <libxml/parser.h>
#include <libxml/tree.h>

#define ARRAY_LEN(xs) (sizeof(xs) / sizeof(xs[0]))

using namespace aids;

const xmlChar *operator""_xml(const char *s, size_t)
{
    return (const xmlChar *)s;
}

void print1(FILE *stream, const xmlChar *s)
{
    fwrite(s, 1, strlen((const char *)s), stream);
}

template <typename Node>
Node find_node(Node node, const xmlChar *name)
{
    while (node && xmlStrcmp(node->name, name) != 0) {
        node = node->next;
    }
    return node;
}

void print_header(FILE *stream)
{
    println(stream, "#ifndef GL_HPP_");
    println(stream, "#define GL_HPP_");
    println(stream);
    println(stream, "namespace gl {");
}

void print_footer(FILE *stream)
{
    println(stream, "}  // namespace gl");
    println(stream, "#endif  // GL_HPP_");
}

String_View xmlstr_as_string_view(const xmlChar *xmlstr)
{
    return cstr_as_string_view((const char *)xmlstr);
}

void gen_subcommand(const char *filepath, xmlDocPtr doc)
{
    std::map<String_View, std::map<String_View, String_View>> groups;

    auto registry = doc->children;
    auto enums = registry->children;
    while (enums) {
        if (xmlStrcmp(enums->name, "enums"_xml) == 0) {
            auto enoom = enums->children;
            while (enoom) {
                if (xmlStrcmp(enoom->name, "enum"_xml) == 0) {
                    auto group_prop = find_node(enoom->properties, "group"_xml);
                    auto name_prop = find_node(enoom->properties, "name"_xml);
                    if (!name_prop) {
                        println(stdout, filepath, ":", enoom->line, ": enum without name found");
                        fflush(stdout);
                        abort();
                    }
                    auto name_text = xmlstr_as_string_view(name_prop->children->content);
                    
                    auto value_prop = find_node(enoom->properties, "value"_xml);
                    if (!value_prop) {
                        println(stdout, filepath, ":", enoom->line, ": enum without value found");
                        fflush(stdout);
                        abort();
                    }
                    auto value_text = xmlstr_as_string_view(value_prop->children->content);

                    if (group_prop) {
                        auto group_text = xmlstr_as_string_view(group_prop->children->content);
                        
                        while (group_text.count) {
                            auto group = group_text.chop_by_delim(',');
                            if (groups[group][name_text].count > 0) {
                                println(stdout, filepath, ":", enoom->line, " enum value ", name_text, " is redefined");
                                fflush(stdout);
                                abort();
                            }
                            groups[group][name_text] = value_text;
                        }
                    }
                }
                enoom = enoom->next;
            }
        } 
        enums = enums->next;
    }

    print_header(stdout);
    for (const auto &group : groups) {
        println(stdout, "enum class ", group.first, " {");
        for (const auto &enoom: group.second) {
            println(stdout, "  ", enoom.first.subview(3, enoom.first.count - 3), " = ", enoom.second, ",");
        }
        println(stdout, "};");
    }
    print_footer(stdout);
}

struct Subcommand
{
    String_View name;
    void (*run)(const char *filepath, xmlDocPtr doc);
    String_View help;
};

Subcommand subcommands[] = {
    {"gen"_sv, gen_subcommand, "Generate the gl.hpp from <spec.xml>"_sv}
};

void usage(FILE *stream)
{
    println(stream, "Not enough arguments provided");
    println(stream, "Usage: spec <spec.xml> <subcommand>");
    println(stream, "Subcommands:");

    const size_t WIDTH = 10;

    for (size_t i = 0; i < ARRAY_LEN(subcommands); ++i) {
        const size_t name_width = subcommands[i].name.count;
        println(stream, 
                "    ", 
                subcommands[i].name, 
                Pad {name_width > WIDTH ? 0 : WIDTH - name_width, ' '}, 
                subcommands[i].help);
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
        println(stderr, "[ERROR] Spec XML file is not provided");
        usage(stderr);
        exit(1);
    }

    const char *filepath = args.pop();

    auto maybeContent = read_file_as_string_view(filepath);
    if (!maybeContent.has_value) {
        println(stderr, "Could not read file `", filepath, "`: ",
                strerror(errno));
        exit(1);
    }
    auto content = maybeContent.unwrap;

    LIBXML_TEST_VERSION;

    xmlDocPtr doc = xmlReadMemory(content.data, content.count, "noname.xml", NULL, 0);
    defer(xmlCleanupParser());
    defer(xmlMemoryDump());

    if (args.empty()) {
        println(stderr, "[ERROR] subcommand is not provided");
        usage(stderr);
        exit(1);
    }

    const char *subcommand = args.pop();

    for (size_t i = 0; i < ARRAY_LEN(subcommands); ++i) {
        if (subcommands[i].name == cstr_as_string_view(subcommand)) {
            subcommands[i].run(filepath, doc);
            return 0;
        }
    }

    println(stderr, "Subcommand `", subcommand, "` does not exist");
    usage(stderr);
    return 1;
}
