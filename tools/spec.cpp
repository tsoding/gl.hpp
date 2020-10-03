#include <set>
#include <map>
#include <vector>
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

#define FOREACH_CHILD(child, node) \
    for (auto child = node->children, it = child; child; child = child->next, it = child, (void) it)
#define FOREACH_CHILD_NAME(child, node, _name) \
    FOREACH_CHILD(child, node) \
        if (xmlStrcmp(child->name, _name) == 0)

void commands_subcommand(const char *filepath, xmlDocPtr doc)
{
#define DEBUG_IT(...) println(stdout, filepath, ":", it->line, ": ", __VA_ARGS__)
    const auto registry = doc->children;
    FOREACH_CHILD_NAME(commands, registry, "commands"_xml) {
        FOREACH_CHILD_NAME(command, commands, "command"_xml) {
            DEBUG_IT("command");
            FOREACH_CHILD(tag, command) {
                if (xmlStrcmp(it->name, "text"_xml) != 0) {
                    DEBUG_IT(Pad {2, ' '}, it->name);
                    if (xmlStrcmp(it->name, "proto"_xml) == 0) {
                        FOREACH_CHILD(proto_child, tag) {
                            if (xmlStrcmp(it->name, "text"_xml) != 0) {
                                DEBUG_IT(Pad {4, ' '}, it->name);
                                DEBUG_IT(Pad {4, ' '}, it->children->content);
                            }
                        }
                    } else if (xmlStrcmp(it->name, "param"_xml) == 0) {
                        FOREACH_CHILD(params_child, tag) {
                            if (xmlStrcmp(it->name, "text"_xml) != 0) {
                                DEBUG_IT(Pad {4, ' '}, it->name);
                            }
                        }
                    }
                }
            }
        }
    }
#undef DEBUG_IT
}

void gen_subcommand(const char *filepath, xmlDocPtr doc)
{
    std::map<String_View, std::map<String_View, String_View>> groups;

    auto registry = doc->children;
    FOREACH_CHILD_NAME (enums, registry, "enums"_xml) {
        FOREACH_CHILD_NAME (enoom, enums, "enum"_xml) {
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

//#define DEBUG

#ifdef DEBUG
#define DEBUG_TAG(tag, ...) println(stdout, filepath, ":", (tag)->line, ": ", __VA_ARGS__)
#else
#define DEBUG_TAG(tag, ...)
#endif

struct Sig
{
    String_View name;
    String_View type;
};

void print1(FILE *stream, Sig sig)
{
    print(stream, "{ name: \"", sig.name, "\", type: \"", sig.type, "\" }");
}

Sig extract_sig(const char *filepath, size_t level, xmlNodePtr node)
{
    Sig result = {};

    FOREACH_CHILD(child, node) {
        if (xmlStrcmp(child->name, "text"_xml) != 0) {
            DEBUG_TAG(child, Pad{level, ' '}, child->name);
            if (xmlStrcmp(child->name, "name"_xml) == 0) {
                DEBUG_TAG(child, Pad{level + 2, ' '}, child->children->content);
                result.name = xmlstr_as_string_view(child->children->content);
            } else if (xmlStrcmp(child->name, "ptype"_xml) == 0) {
                DEBUG_TAG(child, Pad{level + 2, ' '}, child->children->content);
                result.type = xmlstr_as_string_view(child->children->content);
            }
        }
    }

    return result;
}

void commands_subcommand(const char *filepath, xmlDocPtr doc)
{
    auto registry = doc->children;
    FOREACH_CHILD_NAME(commands, registry, "commands"_xml) {
        FOREACH_CHILD_NAME(command, commands, "command"_xml) {
            DEBUG_TAG(command, "command");
            Maybe<Sig> command_sig = {};
            std::vector<Sig> command_params;
            FOREACH_CHILD(prop, command) {
                if (xmlStrcmp(prop->name, "text"_xml) != 0) {
                    DEBUG_TAG(prop, Pad{2, ' '}, prop->name);
                    if (xmlStrcmp(prop->name, "proto"_xml) == 0) {
                        command_sig = {true, extract_sig(filepath, 4, prop)};
                    } else if (xmlStrcmp(prop->name, "param"_xml) == 0) {
                        command_params.push_back(extract_sig(filepath, 4, prop));
                    }
                }
            }
#ifndef DEBUG
            assert(command_sig.has_value);

            println(stdout, "// ", filepath, ":", command->line, ": ", command_sig.unwrap.name);
            if (command_sig.unwrap.type.count == 0) {
                print(stdout, "void");
            } else {
                print(stdout, command_sig.unwrap.type);
            }
            print(stdout, " ");
            print(stdout, command_sig.unwrap.name);
            print(stdout, "(");
            bool first = true;
            for (const auto &param: command_params) {
                if (!first) {
                    print(stdout, ", ");
                }
                print(stdout, param.type, ' ', param.name);
                first = false;
            }
            print(stdout, ")");
            println(stdout);
#endif
        }
    }
}

#undef DEBUG_TAG

struct Subcommand
{
    String_View name;
    void (*run)(const char *filepath, xmlDocPtr doc);
    String_View help;
};

Subcommand subcommands[] = {
    {"gen"_sv, gen_subcommand, "Generate the gl.hpp from <spec.xml>"_sv},
    {"commands"_sv, commands_subcommand, "Generate OpenGL commands"_sv},
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

int main(int argc, char *argv[])
{
    Args args = {argc, argv};
    args.shift();

    if (args.empty()) {
        println(stderr, "[ERROR] Spec XML file is not provided");
        usage(stderr);
        exit(1);
    }

    const char *filepath = args.shift();

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

    const char *subcommand = args.shift();

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
