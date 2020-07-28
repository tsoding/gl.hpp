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

bool xmlstr_has_prefix(const xmlChar *s, const xmlChar *prefix)
{
    while (*s && *prefix && *s == *prefix) { 
        s += 1;
        prefix += 1;
    };
    return *prefix == 0;
}

template <typename Node>
Node find_node(Node node, const xmlChar *name)
{
    while (node && xmlStrcmp(node->name, name) != 0) {
        node = node->next;
    }
    return node;
}

template <typename Parent>
auto find_child(Parent parent, const xmlChar *name)
{
    auto child = parent->children;
    while (child && xmlStrcmp(child->name, name) != 0) {
        child = child->next;
    }
    return child;
}

template <typename Node>
void print_children(Node node)
{
    auto child = node->children;
    while (child) {
        println(stdout, child->name);
        child = child->next;
    }
}

void generate_enum_from_group(FILE *stream, xmlNodePtr groupNode)
{
    println(stream, "enum class ", groupNode->properties->children->content, " {");
    xmlNodePtr iter = groupNode->children;
    while (iter) {
        if (xmlStrcmp(iter->name, "enum"_xml) == 0) {
            println(stream, "    ", iter->properties->children->content + 3, " = ", iter->properties->children->content, ",");
        }
        iter = iter->next;
    }
    println(stream, "};");
}

void generate_groups(FILE *stream, xmlNodePtr groupsNode)
{
    xmlNodePtr iter = groupsNode->children;
    while (iter) {
        if (xmlStrcmp(iter->name, "group"_xml) == 0) {
            generate_enum_from_group(stream, iter);
        }
        iter = iter->next;
    }
}

void generate_function_from_command(FILE *stream, xmlNodePtr iter)
{
    // TODO: generate_function_from_command is not implemented
}

void generate_commands(FILE *stream, xmlNodePtr commandsNode)
{
    xmlNodePtr iter = commandsNode->children;
    while (iter) {
        if (xmlStrcmp(iter->name, "command"_xml) == 0) {
            generate_function_from_command(stream, iter);
        }
        iter = iter->next;
    }
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

void gen_subcommand(xmlDocPtr doc)
{
    xmlNodePtr groupsNode = find_child(doc->children, "groups"_xml);
    xmlNodePtr commandsNode = find_child(doc->children, "commands"_xml);

    print_header(stdout);
    generate_groups(stdout, groupsNode);
    generate_commands(stdout, commandsNode);
    print_footer(stdout);
}

void commentg_subcommand(xmlDocPtr doc)
{
    xmlNodePtr groupsNode = find_child(doc->children, "groups"_xml);
    auto iter = groupsNode->children;
    while (iter) {
        if (xmlStrcmp(iter->name, "group"_xml) == 0) {
            auto name = find_node(iter->properties, "name"_xml);
            auto comment = find_node(iter->properties, "comment"_xml);
            println(stdout, name->children->content, " -> ", comment->children->content);
        }
        iter = iter->next;
    }
}

struct Subcommand
{
    String_View name;
    void (*run)(xmlDocPtr doc);
    String_View help;
};

Subcommand subcommands[] = {
    {"gen"_sv, gen_subcommand, "Generate the gl.hpp from <spec.xml>"_sv},
    {"commentg"_sv, commentg_subcommand, "Print group names and their comments"_sv},
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
            subcommands[i].run(doc);
            return 0;
        }
    }

    println(stderr, "Subcommand `", subcommand, "` does not exist");
    usage(stderr);

    return 1;
}
