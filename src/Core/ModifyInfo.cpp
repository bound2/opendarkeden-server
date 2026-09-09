//////////////////////////////////////////////////////////////////////////////
// Filename    : ModifyInfo.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ModifyInfo.h"

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
ModifyInfo::ModifyInfo(){__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
ModifyInfo::~ModifyInfo() noexcept {
    m_ShortList.clear();
    m_LongList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ�� �ʱ�ȭ�Ѵ�.
//////////////////////////////////////////////////////////////////////////////
void ModifyInfo::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    SHORTDATA short_data;
    LONGDATA long_data;
    BYTE ShortCount, LongCount;

    clearList();

    iStream.read(ShortCount);

    for (BYTE s = 0; s < ShortCount; s++) {
        iStream.read(short_data.type);
        iStream.read(short_data.value);

        if (short_data.type >= MODIFY_MAX)
            throw InvalidProtocolException("modify type past the last one");

        m_ShortList.push_back(short_data);
    }

    iStream.read(LongCount);

    for (BYTE s = 0; s < LongCount; s++) {
        iStream.read(long_data.type);
        iStream.read(long_data.value);

        if (long_data.type >= MODIFY_MAX)
            throw InvalidProtocolException("modify type past the last one");

        m_LongList.push_back(long_data);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
//////////////////////////////////////////////////////////////////////////////
void ModifyInfo::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_ShortList.size() > kMaxCount || m_LongList.size() > kMaxCount)
        throw InvalidProtocolException("too many modify entries");

    oStream.write((BYTE)m_ShortList.size());
    list<SHORTDATA>::const_iterator short_itr = m_ShortList.begin();
    for (; short_itr != m_ShortList.end(); short_itr++) {
        SHORTDATA short_data = *short_itr;
        oStream.write(short_data.type);
        oStream.write(short_data.value);
    }

    oStream.write((BYTE)m_LongList.size());
    list<LONGDATA>::const_iterator long_itr = m_LongList.begin();
    for (; long_itr != m_LongList.end(); long_itr++) {
        LONGDATA long_data = *long_itr;
        oStream.write(long_data.type);
        oStream.write(long_data.value);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string ModifyInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ModifyInfo[" << "ShortCount:" << (int)m_ShortList.size() << ",ShortListSet(";

    list<SHORTDATA>::const_iterator short_itr = m_ShortList.begin();
    for (; short_itr != m_ShortList.end(); short_itr++) {
        SHORTDATA short_data = *short_itr;
        msg << modifyType2String(short_data.type) << ":" << (int)short_data.value << ",";
    }

    msg << "),LongCount:" << (int)m_LongList.size() << ",LongListSet(";

    list<LONGDATA>::const_iterator long_itr = m_LongList.begin();
    for (; long_itr != m_LongList.end(); long_itr++) {
        LONGDATA long_data = *long_itr;
        msg << modifyType2String(long_data.type) << ":" << (int)long_data.value << ",";
    }

    msg << ")]";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Refuses an entry past the count the BYTE on the wire carries and the
// factory maxima budget.
void ModifyInfo::addShortData(ModifyType type, ushort value) {
    __BEGIN_TRY

    if (type >= MODIFY_MAX)
        throw InvalidProtocolException("modify type past the last one");
    if (m_ShortList.size() >= kMaxCount)
        throw InvalidProtocolException("too many modify entries");

    SHORTDATA short_data;
    short_data.type = type;
    short_data.value = value;

    m_ShortList.push_back(short_data);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ModifyInfo::addLongData(ModifyType type, ulong value) {
    __BEGIN_TRY

    if (type >= MODIFY_MAX)
        throw InvalidProtocolException("modify type past the last one");
    if (m_LongList.size() >= kMaxCount)
        throw InvalidProtocolException("too many modify entries");

    LONGDATA long_data;
    long_data.type = type;
    long_data.value = value;

    m_LongList.push_back(long_data);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ModifyInfo::popShortData(SHORTDATA& rData) {
    __BEGIN_TRY

    SHORTDATA short_data = m_ShortList.front();

    rData.type = short_data.type;
    rData.value = short_data.value;

    m_ShortList.pop_front();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ModifyInfo::popLongData(LONGDATA& rData) {
    __BEGIN_TRY

    LONGDATA long_data = m_LongList.front();

    rData.type = long_data.type;
    rData.value = long_data.value;

    m_LongList.pop_front();

    __END_CATCH
}
