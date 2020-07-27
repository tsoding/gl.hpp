#include "aids.hpp"
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace aids;

void print1(FILE *stream, const xmlChar *s)
{
    fwrite(s, 1, strlen((const char *)s), stream);
}

xmlNodePtr find_child(xmlNodePtr node, const xmlChar *name)
{
    auto child = node->children;
    while (child) {
        if (xmlStrcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next;
    }
    return 0;
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
        if (xmlStrcmp(iter->name, (const xmlChar*)"enum") == 0) {
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
        if (xmlStrcmp(iter->name, (const xmlChar*)"group") == 0) {
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
        if (xmlStrcmp(iter->name, (const xmlChar*)"command") == 0) {
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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        println(stderr, "Not enough arguments provided");
        println(stderr, "Usage: spec <spec.xml>");
        exit(1);
    }

    const char *filepath = argv[1];

    auto maybeContent = read_file_as_string_view(filepath);
    if (!maybeContent.has_value) {
        println(stderr, "Could not read file `", filepath, "`: ",
                strerror(errno));
        exit(1);
    }
    auto content = maybeContent.unwrap;

    LIBXML_TEST_VERSION;

    xmlDocPtr doc = xmlReadMemory(
        content.data, content.count,
        "noname.xml", NULL, 0);

    xmlNodePtr groupsNode = find_child(doc->children, (const xmlChar*)"groups");
    xmlNodePtr commandsNode = find_child(doc->children, (const xmlChar*)"commands");

    print_header(stdout);
    generate_groups(stdout, groupsNode);
    generate_commands(stdout, commandsNode);
    print_footer(stdout);


    xmlCleanupParser();
    xmlMemoryDump();

    return 0;
}
