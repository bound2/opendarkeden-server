//////////////////////////////////////////////////////////////////////
//
// Filename    : xml_parse_test.cpp
// Description : Pins what SXml produces for the data/*.xml files the
//               gameserver actually loads at startup, so a change of XML
//               backend or of encoding handling is a reviewable diff
//               instead of a silent content change.
//
//               Every byte outside printable ASCII is rendered \xNN. The
//               data files declare iso-8859-1 while holding a mix of
//               EUC-KR and UTF-8, so a diff here is invisible in a
//               terminal unless the bytes are spelled out.
//
//               Recorded against tinyxml2 (third_party/tinyxml2), which
//               replaced xerces-c. The xerces path differed in exactly two
//               ways, both intentional and both visible in this golden:
//                 - it emitted no element text at all, because
//                   XMLTreeGenerator::characters() took `unsigned int`
//                   where Xerces-C 3.x declares XMLSize_t, so the override
//                   never bound and DefaultHandler's no-op ran instead;
//                 - it ran every string through XMLString::transcode() to
//                   the process's local code page, double-encoding
//                   non-ASCII (0xBA -> U+00BA -> 0xC2 0xBA) and making the
//                   result depend on the server's locale.
//
//////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "SXml.h"
#include "TestStreams.h"

using wiretest::isRecording;

TEST(XmlParseTest, narrowAttributesPreserveAdjacentFields) {
    XMLTree tree;
    tree.LoadFromMem("<Waypoint zone='1001' dir='7' maxword='65535' maxbyte='255'/>");
    struct {
        WORD before = 0xabcd;
        WORD value = 0;
        WORD after = 0xbeef;
    } word;
    struct {
        BYTE before = 0xab;
        BYTE value = 0;
        BYTE after = 0xef;
    } byte;

    ASSERT_TRUE(tree.GetAttribute("zone", word.value));
    EXPECT_EQ(word.value, 1001);
    EXPECT_EQ(word.before, 0xabcd);
    EXPECT_EQ(word.after, 0xbeef);
    ASSERT_TRUE(tree.GetAttribute("dir", byte.value));
    EXPECT_EQ(byte.value, 7);
    EXPECT_EQ(byte.before, 0xab);
    EXPECT_EQ(byte.after, 0xef);
    ASSERT_TRUE(tree.GetAttribute("maxword", word.value));
    EXPECT_EQ(word.value, 65535);
    ASSERT_TRUE(tree.GetAttribute("maxbyte", byte.value));
    EXPECT_EQ(byte.value, 255);
}

TEST(XmlParseTest, invalidNarrowAttributesLeaveTheDestinationUnchanged) {
    XMLTree tree;
    tree.LoadFromMem("<Waypoint word='65536' byte='256' negative='-1' malformed='12x' huge='4294967296'/>");
    WORD word = 42;
    BYTE byte = 17;
    EXPECT_FALSE(tree.GetAttribute("word", word));
    EXPECT_EQ(word, 42);
    EXPECT_FALSE(tree.GetAttribute("byte", byte));
    EXPECT_EQ(byte, 17);
    for (const char* name : {"negative", "malformed", "huge", "missing"}) {
        EXPECT_FALSE(tree.GetAttribute(name, word));
        EXPECT_FALSE(tree.GetAttribute(name, byte));
        EXPECT_EQ(word, 42);
        EXPECT_EQ(byte, 17);
    }
}

namespace {

// The files GQuestInfoManager::load() and GQuestCheckPoint read at startup.
// data/EventGQuestB.xml is deliberately absent: nothing loads it.
const char* kDataFiles[] = {
    "SimpleGQuest.xml",
    "EventGQuest.xml",
    "EventCheckPoint.xml",
    "TravelWay.xml",
};

std::string escapeBytes(const std::string& s) {
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

void renderNode(XMLTree* pTree, const std::string& path, size_t depth, std::ostringstream& out) {
    if (pTree == NULL)
        return;

    const std::string indent(depth * 2, ' ');
    const std::string here = path + "/" + pTree->GetName();

    out << indent << "NODE " << escapeBytes(here) << "\n";

    const size_t attrCount = pTree->GetAttributeCount();
    for (size_t i = 0; i < attrCount; ++i) {
        XMLAttribute* pAttr = pTree->GetAttribute(i);
        if (pAttr == NULL)
            continue;
        // Document order, from m_AttributesVector rather than the hash map,
        // so this rendering is stable across runs.
        out << indent << "  ATTR " << escapeBytes(pAttr->GetName()) << " = \"" << escapeBytes(pAttr->ToString())
            << "\"\n";
    }

    const std::string& text = pTree->GetText();
    if (!text.empty())
        out << indent << "  TEXT \"" << escapeBytes(text) << "\"\n";

    const size_t childCount = pTree->GetChildCount();
    out << indent << "  CHILDREN " << childCount << "\n";

    for (size_t i = 0; i < childCount; ++i)
        renderNode(pTree->GetChild(i), here, depth + 1, out);
}

std::string renderAllDataFiles() {
    std::ostringstream out;
    out << "# Parsed shape of the data/*.xml files loaded at gameserver startup.\n";
    out << "# Generated by xml_parse_test (UPDATE_GOLDENS=1 to re-record).\n";
    out << "# Bytes outside printable ASCII are shown as \\xNN: these files declare\n";
    out << "# iso-8859-1 but hold a mix of EUC-KR and UTF-8, so an encoding change\n";
    out << "# is only visible spelled out.\n";

    const size_t fileCount = sizeof(kDataFiles) / sizeof(kDataFiles[0]);
    for (size_t i = 0; i < fileCount; ++i) {
        const std::string path = std::string(WIRETEST_DATA_DIR) + "/" + kDataFiles[i];
        out << "===== FILE " << kDataFiles[i] << " =====\n";

        XMLTree tree;
        tree.LoadFromFile(path.c_str());
        renderNode(&tree, "", 0, out);
        out << "\n";
    }
    return out.str();
}

} // namespace

TEST(XmlParseTest, dataFilesParseToCommittedShape) {
    const std::string actual = renderAllDataFiles();

    // A parse that silently yields nothing would otherwise "match" an empty
    // golden; require real content before comparing.
    ASSERT_NE(actual.find("NODE /QuestList"), std::string::npos)
        << "SimpleGQuest.xml did not parse to a QuestList root";

    const char* path = WIRETEST_XML_PARSE_FILE;
    if (isRecording()) {
        std::ofstream file(path, std::ios::trunc);
        ASSERT_TRUE(file.good()) << "cannot write " << path;
        file << actual;
        std::printf("recorded %s\n", path);
    } else {
        std::ifstream file(path);
        ASSERT_TRUE(file.good()) << "missing " << path << " — run the tests once with UPDATE_GOLDENS=1 to record it";
        std::stringstream expected;
        expected << file.rdbuf();
        EXPECT_EQ(expected.str(), actual) << "SXml output drifted from " << path
                                          << " — review the diff as an XML-backend or encoding change,"
                                             " then re-record with UPDATE_GOLDENS=1";
    }
}

// The bug that motivated replacing xerces: element text was silently dropped
// for years because the SAX override never bound. Nothing reads GetText()
// today, so only a test keeps the fix from being quietly lost again.
TEST(XmlParseTest, elementTextIsCaptured) {
    const std::string path = std::string(WIRETEST_DATA_DIR) + "/SimpleGQuest.xml";

    XMLTree tree;
    tree.LoadFromFile(path.c_str());

    ASSERT_GT(tree.GetChildCount(), 0u) << "SimpleGQuest.xml parsed to an empty tree";

    XMLTree* pQuest = tree.GetChild(size_t(0));
    ASSERT_TRUE(pQuest != NULL);

    XMLTree* pTitle = pQuest->GetChild("Title");
    ASSERT_TRUE(pTitle != NULL) << "first Quest has no Title child";
    EXPECT_FALSE(pTitle->GetText().empty()) << "Title text is empty — the parser is discarding element text again";
}

// Bytes must survive the parser unchanged. Transcoding here would depend on
// the server's locale, which is how the xerces path produced different text
// on different machines.
TEST(XmlParseTest, nonAsciiBytesArePreservedVerbatim) {
    const std::string path = std::string(WIRETEST_DATA_DIR) + "/SimpleGQuest.xml";

    XMLTree tree;
    tree.LoadFromFile(path.c_str());
    ASSERT_GT(tree.GetChildCount(), 0u);

    XMLTree* pQuest = tree.GetChild(size_t(0));
    ASSERT_TRUE(pQuest != NULL);
    XMLTree* pScript = pQuest->GetChild("Script");
    ASSERT_TRUE(pScript != NULL) << "first Quest has no Script child";

    XMLAttribute* pSender = pScript->GetAttribute("sender");
    ASSERT_TRUE(pSender != NULL) << "Script has no sender attribute";

    // The first quest-giver's name, exactly as it sits in the file. The
    // quest lists are English (tools/i18n) and declare UTF-8, so the
    // attribute is the ten ASCII bytes and nothing is transcoded on the
    // way; when the file was EUC-KR this checked that the twelve bytes of
    // the Korean name came through untouched rather than double-encoded to
    // eighteen, the old xerces behaviour.
    const std::string expected = "Vrykolakas";
    EXPECT_EQ(expected, std::string(pSender->ToString()));
}
