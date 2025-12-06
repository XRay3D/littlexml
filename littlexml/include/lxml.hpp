#pragma once

#include <cassert>
#include <memory>
#include <print>
#include <ranges>
#include <vector>

#ifdef LXML_INTERFACE_UNIT
#define LXML_BEGIN_MODULE_EXPORT export {
#define LXML_END_MODULE_EXPORT   }
#define LXML_EXPORT              export
#else
#define LXML_BEGIN_MODULE_EXPORT
#define LXML_END_MODULE_EXPORT
#define LXML_EXPORT
#endif

LXML_BEGIN_MODULE_EXPORT
// using namespace std::string_literals;
using namespace std ::string_view_literals;
LXML_END_MODULE_EXPORT

// LXML_BEGIN_MODULE_EXPORT
LXML_EXPORT
namespace XML {

namespace r = std ::ranges;
namespace v = std ::views;
using std ::print;
using std ::println;
using std ::string_view;

// =============== Definitions ===============
struct Attribute {
    string_view key;
    string_view value;
};

using AttributeList = std::vector<Attribute>;
using NodeList = std::vector<struct Node*>;

struct Node : std::vector<std::unique_ptr<Node>> {
    friend class Document;
    string_view tag{};
    string_view text{};
    struct Node* parent{};
    AttributeList attributes{};

    Node(Node* parent = nullptr);
    ~Node() = default;
    NodeList children(string_view tag);
    string_view attrVal(string_view key);
    Attribute& attr(string_view key);

private:
    void setTag(string_view newTag);
};

struct Document {
    std::string buf;
    Node root;
    string_view version;
    string_view encoding;
    int load(string_view path);
    int write(string_view path, int indent);
};

// =============== Implementation ===============

// =============== Node ===============
inline void Node::setTag(string_view newTag) {
    if(tag.size()) return;
    if(newTag.starts_with('<'))
        newTag = newTag.substr(1);
    if(size_t i = newTag.find_first_of("\r\n\t /"sv); i < newTag.size())
        newTag = newTag.substr(0, i);
    tag = newTag;
}

inline Node::Node(Node* parent)
    : parent{parent} {
    if(parent) parent->emplace_back(this);
}

inline NodeList Node::children(string_view tag) {
    NodeList list;
    auto filter = [tag](auto&& child) { return child->tag == tag; };
    list.assign_range(*this | v::filter(filter) | v::transform(&std::unique_ptr<Node>::get));
    return list;
}

inline string_view Node::attrVal(string_view key) {
    for(int i = 0; i < attributes.size(); i++) {
        Attribute attr = attributes /*.data*/[i];
        if(attr.key == key)
            return attr.value;
    }
    return {};
}

inline Attribute& Node::attr(string_view key) {
    static Attribute dummy{};
    auto it = r::find(attributes, key, &Attribute::key);
    if(it != attributes.end()) return *it;
    return dummy;
}
// =============== Document ===============
inline int Document::load(string_view path) {
    {
        std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(// if(times > 0) print(file, "{:{}}", " "sv, indent * times);fp); })>
            file(fopen(std::string{path}.c_str(), "r"), {});

        if(!file) {
            println(stderr, "Could not load file from '{}'", path);
            return false;
        }

        fseek(file.get(), 0, SEEK_END);
        int size = ftell(file.get());
        fseek(file.get(), 0, SEEK_SET);

        buf.resize(size);
        fread(buf.data(), 1, size, file.get());
    }
    string_view buf{this->buf};
    string_view lex;
    size_t i;
    Node* currNode = &root;
    // Remove bom
    if(buf.starts_with("\xEF\xBB\xBF"sv))
        buf = buf.substr(3);

    enum class TagType {
        START,
        INLINE
    };

    static constexpr auto parseAttrs = +[](string_view& buf, Node& node) -> TagType {
        Attribute attr;
        TagType tt;
        size_t i{};
        while(i < buf.size()) {
            i = buf.find_first_of(attr.key.empty() ? " \"=>"sv : "\""sv);
            switch(buf[i]) {
            case ' ': {
                node.setTag(buf.substr(1, i));
                buf = buf.substr(++i);
                continue;
            } break;
            case '"': {
                if(attr.key.empty()) {
                    println(stderr, "Value has no key");
                    return TagType::START;
                }
                i = buf.find_first_of('"');
                attr.value = buf.substr(0, i);
                buf = buf.substr(++i);
                node.attributes.emplace_back(attr);
                attr.key = {};
                attr.value = {};
                continue;
            } break;
            case '=': {
                attr.key = buf.substr(0, i++);
                buf = buf.substr(++i);
                continue;
            } break;
            case '>': {
                if(buf.data()[i - 1] == '/') {
                    node.setTag(buf.substr(0, i));
                    tt = TagType::INLINE;
                } else {
                    node.setTag(buf.substr(1, i - 1));
                    tt = TagType::START;
                }
                buf = buf.substr(i);
                return tt;
            } break;
            default: break;
            }
        }
        std::unreachable();
        // return TagType::START;
    };

    while((i = buf.find_first_of('<')) < buf.size()) {
        if(buf.front() == '>') lex = buf.substr(1, i - 1);
        if(size_t i = lex.find_first_not_of("\r\n\t "sv); i > lex.size()) lex = {};
        buf = buf.substr(i);
        // Inner text
        if(lex.size()) {
            if(!currNode) {
                println(stderr, "Text outside of document");
                return false;
            }
            currNode->text = lex;
            lex = {};
        }

        switch(buf[1]) {
        case '/': // End of nodelex
            i = buf.find_first_of('>');
            lex = buf.substr(2, i - 2);
            if(!currNode) {
                println(stderr, "Already at the root");
                return false;
            }
            if(size_t i = lex.find(' '); i < lex.size())
                lex = lex.substr(0, i);
            if(currNode->tag != lex) {
                println(stderr, "Mismatched tags ({} != {})", currNode->tag, lex);
                return false;
            }
            currNode = currNode->parent;
            buf = buf.substr(i);
            continue;
        case '!': // Special nodes
                  // Comments
            if(buf.starts_with("<!--"sv)) {
                while(!lex.ends_with("-->"sv))
                    if(i = buf.substr(i).find_first_of('>'); i == ""sv.npos) {
                        println(stderr, "Mismatched end of comment at {}", buf.data() - this->buf.data());
                        return false;
                    } else lex = buf.substr(0, ++i);
                (new Node{currNode})->text = lex;
                buf = buf.substr(i);
                continue;
            }
            std::unreachable();
        case '?': { // Declaration tags
            Node desc;
            parseAttrs(buf, desc);
            version = desc.attrVal("version"sv);
            encoding = desc.attrVal("encoding"sv);
            if(version.empty()) version = "1.0";
            if(encoding.empty()) encoding = "UTF-8";
            continue;
        }
        default: // New node
            currNode = new Node{currNode};
            // Start tag
            if(parseAttrs(buf, *currNode) == TagType::INLINE) {
                currNode = currNode->parent;
                continue;
            }
            // Is tag name if none
            assert(currNode->tag.size());
            // Reset lexer
            lex = {};
            continue;
        }
    }
    return true;
}

inline int Document::write(string_view path, int indent) {
    std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(fp); })>
        file(fopen(std::string{path}.c_str(), "w"), {});

    if(!file) {
        println(stderr, "Could not open file '{}'", path);
        return false;
    }

    println(file.get(), R"(<?xml version="{}" encoding="{}"?>)", version, encoding);
    auto nodeOut = [file = file.get(), indent](this auto&& nodeOut, const Node* node, int times = 0) -> void {
        for(auto&& child: *node) {
            const auto indentStr = v::repeat(' ', indent * times);
            // if(times > 0) print(file, "{:{}}", " "sv, indent * times);
            print(file, "{:s}", indentStr);
            if(child->text.starts_with("<!--"sv)) {
                println(file, "{}", child->text);
                continue;
            }

            print(file, "<{}", child->tag);
            ++times;
            for(Attribute attr: child->attributes) {
                if(attr.value.empty()) continue;
                if(child->attributes.size() > 8)
                    print(file, "\n{:s}", v::repeat(' ', indent * times - 1));
                print(file, R"( {}="{}")", attr.key, attr.value);
            }
            --times;

            if(child->size() == 0 && child->text.empty())
                println(file, "/>");
            else {
                print(file, ">");
                if(child->size() == 0)
                    println(file, "{}</{}>", child->text, child->tag);
                else {
                    println(file, "");
                    nodeOut(child.get(), times + 1);
                    // if(times > 0) print(file, "{:{}}", " "sv, indent * times);
                    print(file, "{:s}", indentStr);
                    println(file, "</{}>", child->tag);
                }
            }
        }
    };

    nodeOut(&root);
    // fclose(file);
    return true;
}
} // namespace XML
// LXML_END_MODULE_EXPORT
