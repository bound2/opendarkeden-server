//////////////////////////////////////////////////////////////////////////////
// Filename    : xml_dump.cpp
// Description : Canonical dump of an XMLTree parsed through SXml.
//
// Prints every node, attribute and text run in document order with all bytes
// escaped explicitly, so two parser backends can be compared byte-for-byte:
//
//     ./xml_dump data/TravelWay.xml > before.txt   # built against xerces
//     ./xml_dump data/TravelWay.xml > after.txt    # built against tinyxml2
//     diff before.txt after.txt
//
// Escaping is deliberate: the data files declare iso-8859-1 but hold EUC-KR
// bytes, so the interesting differences are invisible in a terminal. Any byte
// outside printable ASCII is emitted as \xNN.
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string>

#include "SXml.h"

// Render a byte string with non-printable-ASCII bytes as \xNN so that an
// encoding change shows up as a textual diff rather than as mojibake.
static std::string escapeBytes(const std::string& s) {
    static const char* kHex = "0123456789abcdef";
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '\\') {
            out += "\\\\";
        } else if (c >= 0x20 && c < 0x7f) {
            out += static_cast<char>(c);
        } else {
            out += "\\x";
            out += kHex[(c >> 4) & 0xf];
            out += kHex[c & 0xf];
        }
    }
    return out;
}

static void dumpNode(const XMLTree* pNode, const std::string& path, size_t depth) {
    if (pNode == NULL)
        return;

    // GetAttribute/GetChild are non-const on the legacy API in places; the
    // walk only reads, so cast once here rather than touching the header.
    XMLTree* pTree = const_cast<XMLTree*>(pNode);

    const std::string here = path + "/" + pTree->GetName();

    printf("%*sNODE %s\n", static_cast<int>(depth * 2), "", escapeBytes(here).c_str());

    const size_t attrCount = pTree->GetAttributeCount();
    for (size_t i = 0; i < attrCount; ++i) {
        const XMLAttribute* pAttr = pTree->GetAttribute(i);
        if (pAttr == NULL)
            continue;
        // Attribute order is the document order kept by m_AttributesVector,
        // not the hash-map order, so this is stable across runs.
        printf("%*s  ATTR %s = \"%s\"\n", static_cast<int>(depth * 2), "",
               escapeBytes(const_cast<XMLAttribute*>(pAttr)->GetName()).c_str(),
               escapeBytes(const_cast<XMLAttribute*>(pAttr)->ToString()).c_str());
    }

    const std::string& text = pTree->GetText();
    if (!text.empty()) {
        printf("%*s  TEXT \"%s\"\n", static_cast<int>(depth * 2), "", escapeBytes(text).c_str());
    }

    const size_t childCount = pTree->GetChildCount();
    printf("%*s  CHILDREN %zu\n", static_cast<int>(depth * 2), "", childCount);

    for (size_t i = 0; i < childCount; ++i) {
        dumpNode(pTree->GetChild(i), here, depth + 1);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: xml_dump <file.xml> [more.xml ...]\n");
        return 2;
    }

    for (int i = 1; i < argc; ++i) {
        printf("===== FILE %s =====\n", argv[i]);

        XMLTree tree;
        tree.LoadFromFile(argv[i]);
        dumpNode(&tree, "", 0);

        printf("\n");
    }

    return 0;
}
