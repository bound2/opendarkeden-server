# DarkEden 게임서버 - 프로젝트 분석 개요

> 분석 완료 날짜: 2026-08-11  
> 분석 대상: `dk_server` (DarkEden MMORPG 서버 오픈소스)

---

## 목차

| 번호 | 항목 | 상태 |
|------|------|------|
| 01 | [프로젝트 기본 정보](01.%20프로젝트%20기본%20정보.md) | ✅ |
| 02 | [서버 아키텍처](02.%20서버%20아키텍잘.md) | ✅ |
| 03 | [빌드 시스템](03.%20빌드%20시스템.md) | ✅ |
| 04 | [소스 코드 구조](04.%20소스%20코드%20구조.md) | ✅ |
| 05 | [패킷 시스템](05.%20패킷%20시스템.md) | ✅ |
| 06 | [Core 라이브러리](06.%20Core%20라이브러리.md) | ✅ |
| 07 | [게임서버 모듈 구성](07.%20게임서버%20모듈%20구성.md) | ✅ |
| 08 | [로그인서버](08.%20로그인서버.md) | ✅ |
| 09 | [공유서버](09.%20공유서버.md) | ✅ |
| 10 | [데이터베이스 레이어](10.%20데이터베이스%20레이어.md) | ✅ |
| 11 | [게임 데이터 파일](11.%20게임%20데이터%20파일.md) | ✅ |
| 12 | [Lua 스크립팅](12.%20Lua%20스크립팅.md) | ✅ |
| 13 | [주요 클래스 계층 구조](13.%20주요%20클래스%20계층%20구조.md) | ✅ |
| 14 | [게임 콘텐츠 시스템 목록](14.%20게임%20콘텐츠%20시스템%20목록.md) | ✅ |
| 15 | [설정 파일 및 인프라](15.%20설정%20파일%20및%20인프라.md) | ✅ |
| 16 | [CI/CD 및 코드 품질 관리](16.%20CI-CD%20및%20코드%20품질%20관리.md) | ✅ |

---

## 분석 제외 파일

→ [분석제외.md](분석제외.md)

---

# 분석 제외 파일 목록

분석하지 않아도 되는 파일들을 확장자/종류별로 정리.

---

## 바이너리 게임 데이터 (data/)

```
data/
├── *.smp         - 게임 맵/스폰 배치 데이터 (바이너리)
├── *.ssi         - 씬/섹션 정보 데이터 (바이너리)
├── *.bin         - 몬스터·보스·퀘스트 스탯 데이터 (바이너리)
├── *.idx         - 인덱스 파일 (바이너리)
└── *.tab         - 테이블 파일 (바이너리)
```

---

## 컴파일된 Lua 바이트코드 (data/lua/)

```
data/lua/
├── *.luc                         - 컴파일된 Lua 바이트코드 (.lua 원본만 분석)
├── exceptMotor/
│   └── *.luc
└── withMotor/
    └── *.luc
```

---

## 레거시 소스 관리 파일 (VSS)

```
src/**/
└── vssver.scc    - Visual SourceSafe 잔여물 (모든 하위 디렉토리)
```

---

## 백업 / 임시 파일

```
data/
├── EventGQuest.xml.0119          - 날짜 버전 백업
├── EventGQuest.xml.bak           - XML 백업
├── EventGQuest.xml.test          - 테스트용 임시본
├── SimpleGQuest.xml.bak
└── SimpleGQuest.xml.bak2

src/server/gameserver/
├── Zone.cpp.backup               - 소스 백업
└── Vampire_backup.cpp            - 소스 백업
```

---

## 기타 바이너리 / 실행파일

```
src/server/gameguard/
├── CSAuth.idx    - GameGuard 인증 인덱스 (바이너리)
└── CSAuth.tab    - GameGuard 인증 테이블 (바이너리)

data/
├── CSAuth.idx
└── CSAuth.tab

src/server/updateserver/
└── UI.spki       - 알 수 없는 바이너리 파일

data/lua/
└── luaAllCompile - 컴파일 스크립트 (셸)

data/
├── copyChiefBin  - 유틸리티 스크립트
├── copyQuestBin  - 유틸리티 스크립트
└── MakeEmpty.sh  - 유틸리티 스크립트
```

---

## 요약 - 제외 확장자

| 확장자 | 이유 |
|--------|------|
| `.smp` | 바이너리 맵 데이터 |
| `.ssi` | 바이너리 씬 데이터 |
| `.bin` | 바이너리 게임 데이터 |
| `.idx` | 바이너리 인덱스 |
| `.tab` | 바이너리 테이블 |
| `.luc` | 컴파일된 Lua 바이트코드 |
| `.scc` | VSS 소스 관리 잔여물 |
| `.backup` | 소스 백업 |
| `.bak`, `.bak2` | 백업 파일 |
| `.0119`, `.test` | 날짜/버전 임시 백업 |
| `.spki` | 알 수 없는 바이너리 |

---

## 1. 프로젝트 기본 정보

| 항목 | 내용 |
|------|------|
| 프로젝트명 | DarkEden Game Server |
| 장르 | MMORPG (3진영 PvP: Slayer / Vampire / Ousters) |
| 언어 | C++11 |
| 빌드 시스템 | CMake 3.16+ + Makefile wrapper |
| 프로젝트 버전 | 1.0 (CMakeLists.txt 기준) |
| 대상 OS | Linux (Ubuntu 20.04+), macOS (Apple Silicon 포함), Docker |
| 오픈소스 여부 | 오픈소스 (`upstream: github.com/opendarkeden/server`) |

### 외부 의존성

| 라이브러리 | 버전 | 용도 |
|-----------|------|------|
| libmysqlclient | 5.7 (MySQL 5.7 / 8 호환) | 게임 DB 연동 |
| Lua / LuaJIT | 5.1 | 퀘스트 스크립팅 |
| Xerces-C | 3.2.3 | XML 파싱 (게임 데이터) |
| pthreads | OS 기본 | 멀티스레딩 |

### 데이터베이스

| DB 이름 | 용도 |
|---------|------|
| `DARKEDEN` | 게임 메인 DB (캐릭터, 아이템, 길드 등) |
| `USERINFO` | 계정/인증 DB |

### 실행 바이너리 (빌드 산출물)

| 바이너리 | 설명 |
|---------|------|
| `bin/loginserver` | 로그인·캐릭터 선택 서버 |
| `bin/sharedserver` | 공유 데이터 서버 (길드 등) |
| `bin/gameserver` | 메인 게임 서버 |

---

---

## 2. 서버 아키텍처 (다중 프로세스 구조)

### 프로세스 구성 및 기동 순서

```
[Client]
   │ TCP:9999
   ▼
[loginserver]  ──UDP:9996/9997──  [gameserver]  ──TCP:9977──  [sharedserver]
   │ DB                               │ DB
   ▼                                  ▼
DARKEDEN + USERINFO             DARKEDEN + USERINFO
```

서버는 반드시 **loginserver → sharedserver → gameserver** 순서로 기동해야 한다.

### 각 서버 역할

| 서버 | 포트 | 주요 역할 |
|------|------|-----------|
| **loginserver** | TCP 9999, UDP 9996 | 계정 인증, 캐릭터 선택, 서버 목록 제공 |
| **sharedserver** | TCP 9977 | 길드 등 여러 gameserver 간 공유 데이터 관리 |
| **gameserver** | TCP 9998, UDP 9997 | 실제 게임 로직 전담 (존, 전투, 아이템, NPC 등) |

> 추가로 `theoneserver`(미사용 추정), `updateserver`(클라이언트 패치용) 소스가 존재하나 빌드 대상에 포함되지 않음

### gameserver 내부 구조 (주요 매니저)

```
GameServer
├── DatabaseManager        - DB 커넥션 풀
├── ObjectManager          - Zone/ZoneGroup/NPC/아이템 등 월드 객체 로드
├── ThreadManager          - ZoneGroupThread 풀 관리 (Zone별 병렬 처리)
├── PacketFactoryManager   - 패킷 생성 팩토리
├── PacketValidator        - 패킷 유효성 검증
├── LoginServerManager     - 로그인서버와 UDP 통신 전담 쓰레드
├── SharedServerManager    - 공유서버와 TCP 통신 전담 쓰레드
├── GameServerInfoManager  - 서버 메타정보 관리
├── BillingPlayerManager   - 과금 시스템 연동 (조건부 컴파일)
└── ClientManager          - 클라이언트 TCP 접속 수락 무한루프 (메인 루프)
```

### 쓰레딩 모델

| 쓰레드 | 역할 |
|--------|------|
| `ClientManager` (메인루프) | select/accept 기반 클라이언트 I/O 처리 |
| `ZoneGroupThread` (N개) | ZoneGroup 하나당 1개 쓰레드, 순차 게임 로직 실행 |
| `LoginServerManager` | UDP 수신 전담 (블로킹 기반) |
| `SharedServerManager` | SharedServer TCP 재연결 및 수신 전담 |

### 서버 간 통신 프로토콜

| 통신 경로 | 프로토콜 | 비고 |
|-----------|----------|------|
| Client ↔ GameServer | TCP | 게임 패킷 (GC/CG 계열) |
| Client ↔ LoginServer | TCP | 로그인 패킷 (LC/CL 계열) |
| GameServer ↔ LoginServer | UDP | DatagramSocket 기반 (GL/LG 계열) |
| GameServer ↔ SharedServer | TCP | Socket 기반, 재연결 로직 포함 (GS/SG 계열) |

---

## 3. 빌드 시스템 (CMake + Makefile)

### CMake 디렉토리 계층

```
CMakeLists.txt (root)
├── src/Core/CMakeLists.txt         → 정적 라이브러리 Core + 패킷 라이브러리 3종
└── src/server/CMakeLists.txt
    ├── ServerCore (서버 공통 lib)
    ├── database/CMakeLists.txt      → GameServerDatabase lib
    ├── chinabilling/CMakeLists.txt  → GameServerCBilling lib
    ├── gameserver/CMakeLists.txt    → gameserver 실행파일
    │   ├── skill/    → Skill lib
    │   ├── item/     → Items lib
    │   ├── billing/  → GameServerBilling lib
    │   ├── war/      → War lib
    │   ├── couple/   → Couple lib
    │   ├── mission/  → Mission lib
    │   ├── ctf/      → CTF lib
    │   ├── quest/    → Quest + LuaScript lib
    │   └── mofus/    → Mofus lib
    ├── loginserver/CMakeLists.txt   → loginserver 실행파일
    └── sharedserver/CMakeLists.txt  → sharedserver 실행파일
```

빌드 산출물: `bin/` (실행파일), `lib/` (정적 라이브러리 `.a`)

### 핵심 정적 라이브러리

| 라이브러리 | 위치 | 역할 |
|-----------|------|------|
| `Core` | src/Core | 소켓, 데이터그램, 패킷 데이터 구조, 플레이어 정보 등 공유 유틸리티 |
| `ServerCore` | src/server | 쓰레드, 뮤텍스, CondVar, LogClient, SystemAPI 등 서버 공통 |
| `GameServerPackets` | src/Core | GC/CG/GL/LG/GS/SG/GG/GM 패킷 (게임서버용) |
| `LoginServerPackets` | src/Core | CL/LC/GL/LG/GM 패킷 (로그인서버용) |
| `SharedServerPackets` | src/Core | GS/SG/LG 패킷 (공유서버용) |
| `GameServerDatabase` | src/server/database | DB 커넥션/쿼리 레이어 |
| `Items`, `Skill`, `Quest` | gameserver 서브모듈 | 게임 콘텐츠 모듈 라이브러리 |
| `War`, `Couple`, `Mission`, `CTF`, `Mofus` | gameserver 서브모듈 | 추가 콘텐츠 모듈 |

### 프리프로세서 매크로 (컴파일 시 서버 타입 분기)

| 매크로 | 적용 대상 | 역할 |
|--------|----------|------|
| `__GAME_SERVER__` | gameserver + GameServerPackets | 게임서버 전용 코드 활성화 |
| `__COMBAT__` | gameserver + GameServerPackets | 전투 관련 코드 활성화 |
| `__LOGIN_SERVER__` | loginserver + LoginServerPackets | 로그인서버 전용 코드 |
| `__SHARED_SERVER__` | sharedserver + SharedServerPackets | 공유서버 전용 코드 |
| `__LINUX__` / `__APPLE__` | 전체 | OS 분기 |
| `__CONNECT_BILLING_SYSTEM__` | gameserver (조건부) | 과금 시스템 연동 |
| `__MOFUS__` | gameserver (조건부) | Mofus 이벤트 시스템 연동 |

### Makefile 명령어 (CMake 래퍼)

| 명령어 | 동작 |
|--------|------|
| `make` (= `make debug`) | CMake Debug 빌드 (`-g`) |
| `make release` | CMake Release 빌드 (`-O2 -DNDEBUG`) |
| `make clean` | `build/`, `bin/`, `lib/` 삭제 |
| `make fmt` | clang-format으로 src/ 전체 포맷 적용 |
| `make fmt-check` | git diff 기준 변경 파일만 포맷 검사 (빠름) |
| `make fmt-check-all` | src/ 전체 파일 포맷 검사 (느림) |

### 코드 포맷 규칙 (.clang-format)

- 기반 스타일: **LLVM**
- 들여쓰기: 4스페이스 (탭 금지)
- 최대 줄 길이: **120자**
- 포인터 정렬: 좌측 (`int* p`)
- `#include` 자동 정렬 및 그룹화 (`SortIncludes: true`)

### CI/CD (GitHub Actions)

- **트리거**: PR → `master` 브랜치
- **동작**: 변경된 `.cpp/.h/.hpp` 파일에 대해 clang-format 검사
- **실패 시**: `make fmt` 실행 권고 메시지 출력 후 빌드 차단

---

## 4. 소스 코드 구조 (src/)

### 전체 규모

| 구분 | 파일 수 |
|------|---------|
| `.cpp` | 2,346개 |
| `.h` | 1,925개 |
| **합계** | **4,271개** |

### 최상위 구조

```
src/
├── Core/                  # 패킷 + 공유 유틸리티 (1,599개 파일)
│   ├── types/             # 게임 공용 타입 정의 헤더 (12개 .h)
│   ├── Rpackets/          # R계열 패킷 (레거시/미사용 추정)
│   ├── TOpackets/         # TO계열 패킷
│   └── Upackets/          # 업데이트 관련 패킷 (CU*, GUO* 등)
└── server/
    ├── database/          # DB 커넥션 레이어 (15개 파일)
    ├── chinabilling/      # 중국 과금 시스템 (조건부 컴파일)
    ├── gameserver/        # 메인 게임서버 (673개 + 서브모듈)
    │   ├── skill/         # 스킬 시스템 ★ 최대 모듈 (1,038개)
    │   ├── item/          # 아이템 시스템 (189개)
    │   ├── quest/         # 퀘스트 + Lua 스크립트 (315개)
    │   ├── mission/       # 미션 시스템 (60개)
    │   ├── mofus/         # 게임 이벤트 (42개)
    │   ├── war/           # 전쟁 시스템 (37개)
    │   ├── billing/       # 과금 연동 (15개)
    │   ├── couple/        # 커플/파티 (11개)
    │   ├── ctf/           # 깃발전쟁 (10개)
    │   ├── exchange/      # 거래소 (4개)
    │   ├── gameguard/     # 안티치트 (4개)
    │   └── test/          # 테스트 코드
    ├── loginserver/       # 로그인서버 (32개)
    ├── sharedserver/      # 공유서버 (28개)
    ├── theoneserver/      # 미사용 (26개)
    └── updateserver/      # 클라이언트 패치 서버 (15개)
```

### Core/ 상세

가장 큰 디렉토리. 세 가지 역할이 혼재한다.

**① 패킷 정의 (대부분)**  
GC, CG, CL, LC, GL, LG, GS, SG, GG, GM 계열 패킷 클래스 + 핸들러.  
CMake에서 서버 타입별로 골라 3개 라이브러리로 따로 컴파일.

**② 소켓/네트워크 유틸리티**  
`Socket`, `ServerSocket`, `DatagramSocket`, `SocketInputStream`, `SocketOutputStream`,  
`SocketEncryptInputStream`, `SocketEncryptOutputStream`, `Encrypter`

**③ 공유 데이터 구조**  
플레이어 정보 (`PCSlayerInfo`, `PCVampireInfo`, `PCOustersInfo` 각 1~3버전),  
인벤토리/기어/아이템 정보, 길드/스킬/이펙트 정보, `Properties`(설정 파서), `GameTime`

**types/ 서브디렉토리**  
게임 전반의 enum/typedef 정의 헤더 모음. 서버-클라이언트 공유용.

| 파일 | 주요 내용 |
|------|-----------|
| `CreatureTypes.h` | PCType, 능력치(STR/DEX/INT/HP/MP), 방향, 시야, 스킬도메인, 진영 정렬 등 |
| `ItemTypes.h` | 아이템 ID, 등급, 슬롯 타입 등 |
| `GuildTypes.h` | 길드 관련 타입 |
| `ObjectTypes.h` | 오브젝트 ID 타입 |
| `PetTypes.h` | 펫 관련 타입 |

### server/ 상세

**database/**  
`DatabaseManager` — 쓰레드 ID를 키로 하는 `unordered_map<int, Connection*>` 풀.  
ZoneGroupThread마다 독립 커넥션을 가져 lock 없이 DB 접근.  
DARKEDEN DB 연결, USERINFO DB 연결, CBilling DB 연결을 각각 분리 관리.

**gameserver/ 서브모듈 규모 비교**

| 모듈 | 파일 수 | 비고 |
|------|---------|------|
| `skill/` | 1,038 | Slayer·Vampire·Ousters 3진영 스킬 전체 구현 |
| `quest/` | 315 | XML 퀘스트 파싱 + Lua 스크립트 엔진 |
| `item/` | 189 | 무기·방어구·소모품 등 아이템 타입별 클래스 |
| `mission/` | 60 | 미션/의뢰 시스템 |
| `mofus/` | 42 | 이벤트 외부 연동 시스템 |
| `war/` | 37 | 종족전·길드전·레벨전 |
| `billing/` | 15 | 과금 시스템 |
| `couple/` | 11 | 커플 링·파트너 시스템 |
| `ctf/` | 10 | 깃발 전쟁 (Capture The Flag) |
| `exchange/` | 4 | 거래소 (최근 추가, WIP) |
| `gameguard/` | 4 | GameGuard 안티치트 헤더만 |

---

## 5. 패킷 시스템 (프로토콜 설계)

### 패킷 헤더 구조 (TCP)

```
[ PacketID : 2 bytes (ushort) ]
[ PacketSize : 4 bytes (uint)  ]  → 바디 크기 (헤더 제외)
[ Sequence : 1 byte  (BYTE)   ]  → 현재 "0" 고정 (미사용)
[ Body : PacketSize bytes      ]
```
헤더 합계: **7 bytes** (`szPacketHeader = szPacketID + szPacketSize + szSequenceSize`)

### 패킷 ID 전체 범위 (PACKET_MAX = 501)

| 범위 | 계열 | 방향 | 개수 |
|------|------|------|------|
| 0 ~ 146 | CG | Client → GameServer | 147 |
| 147 ~ 162 | CL | Client → LoginServer | 16 |
| 163 ~ 171 | CR, CU | 레거시/업데이트 | 9 |
| 172 ~ 421 | GC | GameServer → Client | 250 |
| 422 ~ 424 | GG | GameServer → GameServer | 3 |
| 425 ~ 428 | GL | GameServer → LoginServer | 4 |
| 429 | GM | Monitor | 1 |
| 430 ~ 438 | GS | GameServer → SharedServer | 9 |
| 440 ~ 457 | LC | LoginServer → Client | 18 |
| 458 ~ 461 | LG | LoginServer → GameServer | 4 |
| 462 ~ 468 | RC | 레거시 관리자 채널 | 7 |
| 469 ~ 479 | SG | SharedServer → GameServer | 11 |
| 480 ~ 482 | UC | UpdateServer → Client | 3 |
| 483 ~ 500 | CG/GC Exchange | 거래소 (최근 추가) | 18 |

### 패킷 클래스 구조 (3-in-1 패턴)

각 패킷 `.h` 파일에 3개 클래스가 함께 선언된다. `GCMove.h` 예시:

```cpp
// ① 데이터 컨테이너 (Packet 상속)
class GCMove : public Packet {
    void read(SocketInputStream&);         // 역직렬화
    void write(SocketOutputStream&) const; // 직렬화
    void execute(Player* pPlayer);         // 핸들러 위임
    PacketID_t getPacketID() const { return PACKET_GC_MOVE; }
    PacketSize_t getPacketSize() const { return szObjectID + szCoord*2 + szDir; }
private:
    ObjectID_t m_ObjectID;
    Coord_t m_X, m_Y;
    Dir_t m_Dir;
};

// ② 팩토리 (PacketFactory 상속)
class GCMoveFactory : public PacketFactory {
    Packet* createPacket() { return new GCMove(); }
    PacketID_t getPacketID() const { return Packet::PACKET_GC_MOVE; }
};

// ③ 핸들러 (별도 .cpp)
class GCMoveHandler {
    static void execute(GCMove* pPacket, Player* pPlayer);
};
```

### TCP vs UDP 패킷 베이스 클래스

| 클래스 | 전송 방식 | 사용 계열 |
|--------|----------|-----------|
| `Packet` | TCP (SocketInputStream/OutputStream) | GC, CG, LC, CL, GS, SG 등 대부분 |
| `DatagramPacket` | UDP (Datagram 객체) | GL, LG (게임서버↔로그인서버) |

> DatagramPacket을 TCP 소켓으로 읽으려 하면 즉시 `ProtocolException` 발생 (혼용 방지)

### 직렬화 방식

- **바이너리 직렬화** (텍스트/JSON 아님) — raw byte 복사
- `SocketInputStream`: **링 버퍼** (기본 80KB, `m_Head/m_Tail` 인덱스)
- 템플릿 `read<T>()` 로 타입 크기만큼 버퍼에서 읽음
- **암호화**: `EncryptKey(WORD) + HashTable(BYTE*)` 기반 XOR 암호화 (2008년 추가)

### PacketFactory 관리

`PacketFactoryManager` — PacketID를 인덱스로 하는 배열(`PacketFactory** m_Factories`)  
서버 기동 시 `init()`에서 서버 타입에 맞는 팩토리들을 등록.  
패킷 수신 시 `createPacket(packetID)` → 팩토리가 new로 패킷 객체 생성.

### PacketValidator (상태 기반 패킷 검증)

`PlayerStatus`별로 허용 PacketID 집합을 관리.  
클라이언트가 잘못된 상태에서 패킷을 보내면 드롭하여 비정상 처리 방지.

**게임서버 플레이어 상태 흐름:**
```
GPS_BEGIN_SESSION
  → (CGConnect 수신) → GPS_WAITING_FOR_CG_READY
  → (CGReady 수신)   → GPS_NORMAL          ← 게임 중 모든 패킷 허용
  → (로그아웃)       → GPS_END_SESSION
```

**로그인서버 플레이어 상태 흐름:**
```
LPS_BEGIN_SESSION
  → (CLLogin)        → LPS_WAITING_FOR_CL_GET_PC_LIST
  → (CLGetPCList)    → LPS_PC_MANAGEMENT
  → (CLSelectPC)     → LPS_AFTER_SENDING_LG_INCOMING_CONNECTION
  → (GL응답)         → LPS_END_SESSION
```

---

## 6. Core 라이브러리

`src/Core/`는 패킷 정의(1,024 .cpp)와 공유 유틸리티(570 .h)가 한 디렉토리에 공존한다.  
서버 타입(Game/Login/Shared)에 무관하게 공유되는 모든 코드의 출발점.

---

### 6-1. 타입 시스템

`Types.h` 가 `types/` 서브디렉토리의 헤더를 전부 include.

| 파일 | 주요 내용 |
|------|-----------|
| `SystemTypes.h` | `BYTE/WORD/DWORD/uint/ulong` 재정의, `BUILD_NUMBER=40518`, OS 분기 |
| `CreatureTypes.h` | PCType(진영), 능력치(STR/DEX/INT/HP/MP), 방향, 시야, 스킬도메인, 정렬 등 |
| `ItemTypes.h` | 아이템 ID·등급·슬롯 |
| `GuildTypes.h` | 길드 타입 |
| `ObjectTypes.h` | ObjectID 타입 |
| `ZoneTypes.h` | Zone ID·타입 |
| `WarTypes.h` | 전쟁 타입 |
| `ShopTypes.h` | 상점 타입 |
| `QuestTypes.h` | 퀘스트 타입 |
| `PlayerTypes.h` | 플레이어 타입 |
| `ServerType.h` | ServerStatus, WorldStatus enum |

---

### 6-2. 소켓 / 네트워크 계층

```
SocketImpl (플랫폼 추상화: POSIX/Windows)
    │
    ├── Socket           TCP 클라이언트 소켓
    ├── ServerSocket     TCP 서버 소켓 (accept)
    └── DatagramSocket   UDP 소켓

SocketInputStream        링 버퍼(80KB, m_Head/m_Tail) + 암호화 키
SocketOutputStream       송신 버퍼
SocketEncryptInputStream / SocketEncryptOutputStream   암호화 스트림 래퍼

Datagram / DatagramPacket / SerialDatagram   UDP 패킷 처리
```

- `Socket`은 `SocketImpl`에 구현을 위임 (Bridge 패턴)
- `SocketInputStream::read<T>()` 템플릿으로 타입 크기만큼 링 버퍼에서 읽음
- 링 버퍼가 역순(`m_Head > m_Tail`)일 때 `memcpy` 두 번으로 처리

---

### 6-3. 예외 계층

Java 스타일 예외 계층. `__BEGIN_TRY`/`__END_CATCH` 매크로로 스택 추적.

```
Throwable (std::exception 상속)
├── Exception
│   ├── IOException
│   │   ├── SocketException → BindException, ConnectException
│   │   ├── ProtocolException → IdleException, InvalidProtocolException
│   │   │                      InsufficientDataException, DisconnectException
│   │   │                      IgnorePacketException
│   │   ├── EOFException
│   │   └── TimeoutException
│   ├── RuntimeException
│   │   ├── InvalidArgumentException
│   │   ├── OutOfBoundException
│   │   ├── NoSuchElementException
│   │   └── DuplicatedException
│   ├── SQLException → SQLConnectException, SQLQueryException
│   └── GameException → PortalException, EmptyTileNotExistException
│                       InventoryFullException
└── Error
    ├── GameError
    ├── AssertionError
    ├── UnsupportedError
    ├── LogError
    └── UnknownError
```

**주요 매크로:**

| 매크로 | 역할 |
|--------|------|
| `__BEGIN_TRY` / `__END_CATCH` | Debug 빌드에서 `__PRETTY_FUNCTION__`을 예외 스택에 추가 후 rethrow |
| `__ENTER_CRITICAL_SECTION(mutex)` | `mutex.lock()` + try |
| `__LEAVE_CRITICAL_SECTION(mutex)` | 예외 시 `mutex.unlock()` + rethrow |

> Release(`NDEBUG`) 빌드에서는 `__BEGIN_TRY`/`__END_CATCH`가 `((void)0)`으로 대체되어 오버헤드 없음

---

### 6-4. 게임 데이터 구조 (Info 클래스)

패킷에 담겨 전송되는 게임 오브젝트의 스냅샷 구조체.  
모두 `read(SocketInputStream&)` / `write(SocketOutputStream&)` 메서드를 구현해 직렬화 지원.

| 분류 | 클래스 |
|------|--------|
| 플레이어 | `PCSlayerInfo` / `PCVampireInfo` / `PCOustersInfo` (각 1~3 버전) |
| 아이템·장비 | `GearInfo`, `InventoryInfo`, `PCItemInfo`, `SubItemInfo`, `GearSlotInfo` |
| 스킬 | `PCSkillInfo`, `SlayerSkillInfo`, `VampireSkillInfo`, `OustersSkillInfo` |
| 길드 | `GuildInfo`, `GuildMemberInfo`, `GuildWarInfo`, `LevelWarInfo`, `RaceWarInfo` |
| 퀘스트 | `QuestStatusInfo` |
| 펫 | `PetInfo` |
| 기타 | `NPCInfo`, `BloodBibleBonusInfo`, `EffectInfo`, `ModifyInfo`, `StoreInfo` |

---

### 6-5. 유틸리티

| 클래스 | 역할 |
|--------|------|
| `Properties` | `.conf` 파일 key=value 파서. `g_pConfig` 전역으로 서버 전체에서 사용 |
| `SXml` (XMLTree) | Xerces-C 래퍼. XML 게임 데이터 파일 로딩 (`LoadFromFile`) |
| `GameTime` | 게임 내 시간 (Year/Month/Day/Hour/Min/Sec, 7 bytes). 패킷 직렬화 지원 |
| `StringStream` | 문자열 빌더 (cout 스타일 `<<` 연산자) |
| `FileAPI` | 파일 시스템 유틸리티 |
| `Geometry` | 2D 좌표/방향 계산 |
| `TimeChecker` | 경과 시간 측정 |
| `Encrypter`, `EncryptUtility` | 패킷 XOR 암호화 (EncryptKey + HashTable) |
| `md5` | MD5 해시 (비밀번호 등) |
| `HashMap` | 커스텀 해시맵 (STL 이전 시대 유산) |

---

### 6-6. 서브디렉토리 패킷

| 디렉토리 | 내용 |
|----------|------|
| `Rpackets/` | R계열 패킷 (GM 관리자 채널, `RC_*` 패킷) |
| `TOpackets/` | TO계열 패킷 (`GTO_ACKNOWLEDGEMENT` 등, theoneserver 통신) |
| `Upackets/` | U계열 패킷 (`CU_*/UC_*`, 업데이트 서버 통신) |

---

## 7. 게임서버 모듈 구성

### 7-1. 게임 오브젝트 클래스 계층

```
Object  (ObjectID, ObjectClass, ObjectPriority)
├── Creature  (위치 x/y/dir, Zone*, MoveMode, Sight, EffectManager, Resist)
│   ├── PlayerCreature  (공통 PC 로직: 인벤토리, 스킬, 길드, 파티 등)
│   │   ├── Slayer       슬레이어 플레이어
│   │   ├── Vampire      뱀파이어 플레이어
│   │   └── Ousters      아우스터즈 플레이어
│   ├── NPC              상점/퀘스트 NPC
│   └── Monster          몬스터 (AI 포함)
├── Item       (아이템 - ConcreteItem으로 구체화)
├── Effect     (시간 기반 효과 - 크리처/타일 양쪽에 부착 가능)
├── Portal     (존 이동 포털)
└── Obstacle   (충돌 장애물)
```

**ObjectPriority** (Tile의 `forward_list` 정렬 기준):  
`WALKING > FLYING > BURROWING > EFFECT > ITEM > PORTAL > OBSTACLE`

**이동 모드 (MoveMode)**: `WALKING` / `FLYING` / `BURROWING` (잠행)

---

### 7-2. 월드 구조 (ZoneGroup → Zone → Tile)

```
ZoneGroupManager
└── ZoneGroup  (GameTime 보유, processPlayers() + heartbeat() 루프)
    └── Zone   (2D 타일 맵 - 너비·높이, 타입, 접근모드)
        ├── Tile[][]  (WORD 비트플래그로 상태 관리)
        │   └── forward_list<Object*>  (타일 위 오브젝트 목록)
        └── Sector[][] (타일 묶음 - 몬스터 AI 구역 최적화)
```

**ZoneType** 종류:

| 값 | 설명 |
|----|------|
| `ZONE_NORMAL_FIELD` | 일반 필드 |
| `ZONE_NORMAL_DUNGEON` | 일반 던전 |
| `ZONE_SLAYER_GUILD` | 슬레이어 길드 존 |
| `ZONE_PC_VAMPIRE_LAIR` | 뱀파이어 플레이어 레어 |
| `ZONE_NPC_VAMPIRE_LAIR` | NPC 뱀파이어 레어 |
| `ZONE_CASTLE` | 성 (전쟁 거점) |
| `ZONE_RANDOM_MAP` | 랜덤 맵 |

**Zone 속성 플래그**: `isPKZone`, `isHolyLand`, `isCastle`, `isMasterLair`, `isPremiumZone`, `isPayPlay`, `isNoPortalZone`, `isDynamicZone`  
**ZoneAccessMode**: `PUBLIC` / `PRIVATE` (길드존·레어는 PRIVATE)  
**DarkLevel / LightLevel**: 시야 영향 (어둠 마법 등)  
**Timeband**: 낮/밤 시간대

---

### 7-3. Zone 내부 매니저 구성

```
Zone
├── PCManager          - 현재 존의 플레이어 관리
├── NPCManager         - NPC 관리
├── MonsterManager     - 몬스터 스폰·AI 관리
├── EffectManager      - 존·크리처 이펙트 관리
├── EffectScheduleManager - 이펙트 예약 실행
├── WeatherManager     - 날씨 (맑음/비/눈 등)
├── LocalPartyManager  - 파티 관리
├── PartyInviteInfoManager - 파티 초대 상태
├── TradeManager       - 플레이어 간 거래
├── WarScheduler       - 전쟁 스케줄 관리
├── LevelWarManager    - 레벨 전쟁
├── MasterLairManager  - 마스터 레어 (보스 던전)
└── VampirePortalManager - 뱀파이어 포털 이펙트
```

---

### 7-4. Effect 시스템

`Effect`는 크리처 또는 타일에 부착되는 시간 기반 상태 객체.  
크리처에는 `bitset<EFFECT_CLASS_MAX>`로 활성 Effect 플래그를 관리.  
Effect 하위 클래스가 90개 이상 존재.

| 분류 | 예시 클래스 |
|------|-------------|
| 회복 | `EffectHPRecovery`, `EffectMPRecovery` |
| 이동/변신 | `EffectRideMotorcycle`, `EffectGhost` |
| 전투 | `EffectAutoTurret`, `EffectTurretLaser`, `EffectCastingTrap` |
| 상태이상 | `EffectComa`, `EffectMute`, `EffectLoud` |
| 아이템/이동 | `EffectDecayItem`, `EffectTransportItem`, `EffectAddMonster` |
| 릭/성전 | `EffectRelic`, `EffectHasBloodBible`, `EffectShrineGuard` |
| 유틸 | `EffectKickOut`, `EffectShutDown`, `EffectRegenZone` |

---

### 7-5. DynamicZone (인스턴스 던전)

```
DynamicZone (base)
├── DynamicZoneAlterOfBlood       - 피의 제단
├── DynamicZoneGateOfAlter        - 제단의 문
├── DynamicZoneSlayerMirrorOfAbyss   - 심연의 거울 (슬레이어)
├── DynamicZonVampireMirrorOfAbyss   - 심연의 거울 (뱀파이어)
└── DynamicZoneOustersMirrorOfAbyss  - 심연의 거울 (아우스터즈)
```

---

### 7-6. GQuest 시스템 (글로벌 퀘스트)

Composite 패턴으로 퀘스트 조건 트리 구성. `GQuestElement` 하위 클래스 30개 이상.

```
GQuestElement (base)
├── GQuestKillMonsterElement    - 몬스터 처치
├── GQuestGiveItemElement       - 아이템 지급
├── GQuestWarpElement           - 위치 이동
├── GQuestEnterDynamicZoneElement - 인스턴스 던전 입장
├── GQuestRandomElement         - 랜덤 분기
├── GQuestORElement / GQuestNOTElement - 논리 연산
└── ...
```

---

### 7-7. Event / 서버 스케줄러

일회성/반복 서버 이벤트를 `EventManager`가 큐에 넣어 처리.

| 이벤트 | 역할 |
|--------|------|
| `EventSave` | 주기적 DB 저장 |
| `EventRegeneration` | 몬스터/NPC 재생성 |
| `EventResurrect` | 플레이어 부활 처리 |
| `EventTransport` | 존 간 이동 처리 |
| `EventShutdown` | 서버 종료 절차 |
| `EventSystemMessage` | 전체 공지 발송 |
| `EventRefreshHolyLandPlayer` | 성지 보너스 갱신 |
| `EventReloadInfo` | 런타임 데이터 리로드 |

---

### 7-8. 몬스터 AI

`FiniteStateMachine` 기반 상태 기계.  
`MonsterAI`가 `Zone::heartbeat()`에서 호출.  
`Sector` 단위로 시야 검색 범위를 제한해 성능 최적화.

---

## 08. 로그인서버

### 역할

클라이언트의 최초 접속 지점. 계정 인증 → 캐릭터 선택 → 게임 서버 연결 안내를 담당한다.
게임서버·공유서버보다 **먼저 기동**되어야 한다.

---

### 매니저 구성 (LoginServer 생성자 순서)

| 매니저 | 역할 |
|--------|------|
| `DatabaseManager` | DB 연결 관리 |
| `GameServerInfoManager` | DB에서 읽은 게임 서버 목록 |
| `GameServerGroupInfoManager` | 서버 그룹(WorldID + GroupID + GroupName) 정보 |
| `ZoneInfoManager` | Zone 정보 |
| `ZoneGroupInfoManager` | ZoneGroup → ServerID 매핑 |
| `PacketFactoryManager` | 패킷 역직렬화 팩토리 |
| `PacketValidator` | LPS_* 상태머신으로 패킷 순서 검증 |
| `GameServerManager` | 게임서버와 UDP 통신 (별도 Thread) |
| `ClientManager` | TCP 클라이언트 수용 (메인 스레드 무한루프) |
| `ItemDestroyer` | 특정 계정 소유 아이템 일괄 삭제 |
| `UserInfoManager` | 현재 접속 중인 유저 정보 (WorldID + ServerGroupID + UserNum) |
| `GameWorldInfoManager` | 게임 월드 목록 정보 |

#### 조건부 매니저

| 매크로 | 매니저 | 용도 |
|--------|--------|------|
| `__CONNECT_CBILLING_SYSTEM__` | `CBillingPlayerManager` | 중국 빌링 연동 |
| `__THAILAND_SERVER__` | `TimeChecker` | 태국 청소년 보호(ChildGuard) |

---

### 스레드 모델

```
[메인 스레드]
  LoginServer::start()
    → GameServerManager::start()   // 별도 Thread 생성 (UDP 수신 루프)
    → ClientManager::start()       // 메인 스레드에서 직접 무한루프 실행 (블로킹)
```

- `ClientManager::run()`이 **리턴하지 않는** 무한루프이므로, 이후 코드는 실행되지 않는다.
- `LoginServer::stop()`은 `UnsupportedError`를 던지도록 구현되어 있어 정상 종료가 지원되지 않는다.

---

### LoginPlayer 클래스

```
Player  PaySystem  BillingPlayerInfo  CBillingPlayerInfo
   └──────────────────────────────────────────┘
                  LoginPlayer
```

| 멤버 | 설명 |
|------|------|
| `m_PlayerStatus` (PlayerStatus) | LPS_* 상태머신 현재 상태 |
| `m_PacketHistory` (deque, 최대 10개) | 최근 수신 패킷 이력 |
| `m_FailureCount` (maxFailure=3) | 인증 실패 횟수, 초과 시 강제 끊김 |
| `m_WorldID`, `m_ServerGroupID`, `m_LastSlot` | 접속할 게임서버 정보 |
| `m_LastCharacterName` | 마지막으로 선택한 캐릭터 이름 |
| `m_SSN`, `m_Zipcode` | 한국 주민번호·우편번호 (본인인증) |
| `m_bFreePass` | 외부 인증(넷마블 등)을 이미 통과한 경우 |
| `m_bWebLogin` | 웹 로그인 경로 여부 |
| `m_gameServerIP` | 클라이언트에게 알려줄 게임서버 IP |
| `m_KickCharacterCount`, `m_ExpireTimeForKickCharacter` | 중복 접속 시 기존 캐릭터 강제 로그아웃 대기 |

---

### 로그인 흐름

```
[Client]                [LoginServer]              [GameServer]
   │─── CL연결 ─────────→ ClientManager                │
   │─── CLLogin ────────→ LoginPlayer.processCommand   │
   │                       DB 계정 검증                │
   │←── LCLoginOK ─────────                           │
   │─── CLSelectPC ─────→ 캐릭터 선택                  │
   │                       ───── LGKickCharacter ──── →│  (중복 접속 처리)
   │←── LCSelectOK ─────── gameServerIP 전달           │
   │─── (TCP 끊김) ──────                              │
   │══════════════════════════════════════════════════ │
   │─── CG 접속 ─────────────────────────────────────→│
```

---

### 서버 간 전송 보안 (ReconnectLoginInfo)

게임서버 간 캐릭터 이동(서버 점프) 시 로그인 서버가 임시 인증 토큰을 발행한다.

```
ReconnectLoginInfo {
    ClientIP   : string    // 유효한 클라이언트 IP
    PlayerID   : string    // 계정 ID
    Key        : DWORD     // 무작위 검증 키
    ExpireTime : Timeval   // 만료 시각
}
```

- `priority_queue<..., CompareReconnectLoginInfo>` 구조로 만료 시각 오름차순 정렬 → 빠른 만료 항목 우선 제거
- 게임서버는 이 Key를 검증한 뒤에만 재접속을 허용한다.

---

### Info 클래스 (DB 로드)

| 클래스 | DB 테이블 | 역할 |
|--------|-----------|------|
| `GameServerGroupInfo` | `GameServerGroupInfo` | WorldID + GroupID + GroupName + Stat |
| `ZoneGroupInfo` | `ZoneGroupInfo` | ZoneGroupID → ServerID 매핑 |
| `ZoneInfo` | `ZoneInfo` | 개별 Zone 정보 |
| `UserInfo` | 런타임 관리 | 현재 접속 유저 위치 (WorldID + ServerGroupID + UserNum) |

---

## 09. 공유서버 (Shared Server)

### 역할

여러 게임서버 인스턴스가 **공유해야 하는 데이터**(주로 길드 정보)를 중앙에서 관리한다.
게임서버들은 TCP로 공유서버에 접속해 길드 데이터를 읽고 쓴다.

---

### 매니저 구성 (SharedServer 생성자 순서)

| 매니저 | 역할 |
|--------|------|
| `DatabaseManager` | DB 연결 관리 |
| `GuildManager` | 길드 데이터 인메모리 관리 (핵심) |
| `GameServerInfoManager` | 게임서버 정보 목록 |
| `GameServerGroupInfoManager` | 게임서버 그룹 정보 |
| `PacketFactoryManager` | 패킷 역직렬화 팩토리 |
| `PacketValidator` | 패킷 순서 검증 |
| `GameServerManager` | 게임서버 TCP 연결 수용 (별도 Thread) |
| `HeartbeatManager` | 메인 스레드 루프 (heartbeat 주기 처리) |
| `GameWorldInfoManager` | 게임 월드 정보 |
| `ResurrectLocationManager` | Zone별 부활 위치 (Slayer/Vampire 분리) |
| `StringPool` | DB 로드 다국어 문자열 (10종, 길드 알림용) |

#### 조건부 매니저

| 매크로 | 매니저 | 상태 |
|--------|--------|------|
| `__NETMARBLE_SERVER__` | `NetmarbleGuildRegisterThread` | 주석 처리됨 (비활성) |

---

### 스레드 모델

```
[별도 Thread]  GameServerManager::run()
                → select() 기반 TCP 다중 접속 처리
                → 최대 100개 게임서버 동시 연결 (nMaxGameServers=100)
                → GameServerPlayer 배열 (소켓 FD를 인덱스로 사용)

[메인 스레드]  HeartbeatManager::start()  ← 블로킹 무한루프
```

---

### GameServerManager 구조

로그인서버의 `GameServerManager`(UDP)와 달리 **TCP ServerSocket** 기반이다.

```cpp
class GameServerManager : public Thread {
    ServerSocket*      m_pServerSocket;       // TCP 수신 소켓
    fd_set             m_ReadFDs[2];          // select() FD 집합 (저장/작업 분리)
    fd_set             m_WriteFDs[2];
    fd_set             m_ExceptFDs[2];
    Timeval            m_Timeout[2];
    GameServerPlayer*  m_pGameServerPlayers[100]; // FD → Player 매핑
    Mutex              m_Mutex;
};
```

- `m_ReadFDs[0]`은 마스터 집합(보존용), `[1]`은 `select()` 인자용으로 매 루프마다 `[0]→[1]` 복사
- `broadcast(Packet*)` 로 모든 연결된 게임서버에 일괄 전송 가능

---

### GuildManager (핵심 컴포넌트)

```cpp
unordered_map<GuildID_t, Guild*>  m_Guilds;   // 전체 길드 인메모리 맵
Timeval m_WaitMemberClearTime;                 // Wait 상태 멤버 정리 타이머
Mutex   m_Mutex;                               // 다중 게임서버 동시 접근 보호
```

| 제공 기능 | 설명 |
|-----------|------|
| `addGuild` / `deleteGuild` / `getGuild` | 기본 CRUD (뮤텍스 보호) |
| `addGuild_NOBLOCKED` / `getGuild_NOBLOCKED` | 이미 락을 잡은 컨텍스트에서 호출 |
| `heartbeat()` | Wait 상태 멤버 만료 처리 등 주기적 유지관리 |
| `isGuildMaster()` | 길드 마스터 여부 확인 |
| `hasCastle()` | 성 점령 여부 + 위치(ServerID, ZoneID) 반환 |
| `hasWarSchedule()` | 전쟁 예약 여부 |
| `hasActiveWar()` | 현재 전쟁 진행 여부 |
| `makeWaitGuildList()` / `makeActiveGuildList()` | 클라이언트용 길드 목록 패킷 생성 |
| `makeSGGuildInfo()` | 게임서버 요청에 대한 길드 정보 직렬화 (`__SHARED_SERVER__` 전용) |

---

### ResurrectLocationManager

플레이어 사망 시 부활 위치를 Zone 단위로 관리한다.

```cpp
unordered_map<ZoneID_t, ZONE_COORD>  m_SlayerPosition;   // Slayer 부활 좌표
unordered_map<ZoneID_t, ZONE_COORD>  m_VampirePosition;  // Vampire 부활 좌표
```

- `init()` → `load()` 순서로 DB에서 좌표 로드
- Ousters 부활 위치는 소스 레벨 하드코딩 (`Resurrect.cpp`)

---

### StringPool

DB에서 로드하는 게임 내 메시지 문자열 풀.

```cpp
enum StringID {
    STRID_TEAM_REGISTRATION_ACCEPT,    // 팀 등록 수락
    STRID_CLAN_REGISTRATION_ACCEPT,    // 클랜 등록 수락
    STRID_TEAM_JOIN_ACCEPT,            // 팀 가입 수락
    STRID_CLAN_JOIN_ACCEPT,            // 클랜 가입 수락
    STRID_TEAM_BROKEN, STRID_CLAN_BROKEN,   // 해체
    STRID_TEAM_CANCEL, STRID_CLAN_CANCEL,   // 취소
    // ... (STRID_MAX = 10)
};
```

---

### 로그인서버와의 차이점

| 항목 | 로그인서버 | 공유서버 |
|------|----------|---------|
| 클라이언트 대상 | 게임 클라이언트 (다수) | 게임서버 프로세스 (최대 100개) |
| 네트워크 프로토콜 | TCP (클라이언트) + UDP (게임서버) | TCP만 사용 |
| 메인 루프 클래스 | `ClientManager` | `HeartbeatManager` |
| 핵심 데이터 | 계정/캐릭터 인증 정보 | 길드 정보 |
| 정상 종료 | `UnsupportedError` (미구현) | `UnsupportedError` (미구현) |

---

## 10. 데이터베이스 레이어

### 구성 파일

```
src/server/database/
├── DB.h             - 편의 매크로 모음 (BEGIN_DB, END_DB, NEW_STMT)
├── Connection.h/cpp - MySQL 연결 래퍼
├── Statement.h/cpp  - SQL 실행
├── Result.h/cpp     - 쿼리 결과 커서
└── DatabaseManager.h/cpp - 스레드별 연결 관리자
```

---

### 클래스 관계

```
DatabaseManager
  ├── m_pDefaultConnection       (Connection)  ← DARKEDEN DB
  ├── m_pUserInfoConnection      (Connection)  ← USERINFO DB
  ├── m_Connections              (TID → Connection)  ← 스레드별 DARKEDEN
  ├── m_WorldConnections         (WorldID → Connection)
  └── m_CBillingConnections      (TID → Connection)  ← 중국 빌링

Connection ─── createStatement() ──→ Statement ─── executeQuery() ──→ Result
```

---

### Connection

MySQL C API의 `MYSQL` 구조체를 직접 래핑한다.

```cpp
class Connection {
    MYSQL  m_Mysql;          // libmysqlclient 핵심 구조체
    bool   m_bConnected;
    bool   m_bBusy;          // 사용 중 여부
    Mutex  m_Mutex;          // 동시 쿼리 방지
    // host, db, user, password, port ...
};
```

- `createStatement()` → `Statement*` 반환 (호출자가 `delete` 책임)
- `getBusy()` / `setBusy()` : 연결 풀 구현에 활용

---

### Statement & Result

```cpp
// 사용 패턴
Statement* pStmt = pConnection->createStatement();
Result* pResult = pStmt->executeQuery("SELECT ... FROM ...");
while (pResult->next()) {
    int  val  = pResult->getInt(1);
    BYTE b    = pResult->getBYTE(2);
    const char* s = pResult->getString(3);
}
delete pStmt;  // Result는 Statement 소멸 시 같이 해제됨
```

| Result 메서드 | 반환 타입 |
|--------------|----------|
| `getField(i)` | `char*` (raw) |
| `getInt(i)` | `int` (atoi) |
| `getUInt(i)` | `uint` |
| `getBYTE(i)` | `BYTE` |
| `getWORD(i)` | `WORD` |
| `getDWORD(i)` | `DWORD` (strtoul) |
| `getString(i)` | `const char*` |

---

### DatabaseManager 스레드 모델

**핵심 설계**: 쿼리를 실행하는 각 스레드가 **독립적인 `Connection`** 을 보유한다.

```cpp
// getConnection()의 동작
Connection* DatabaseManager::getConnection(const string&) {
    auto itr = m_Connections.find(Thread::self());  // 현재 스레드 ID 검색
    if (itr == m_Connections.end())
        return m_pDefaultConnection;  // 스레드 미등록 시 기본 연결 사용
    return itr->second;
}
```

- `Thread::self()` = pthread_self() 캐스팅 값 (int)
- 스레드 시작 시 `addConnection(TID, new Connection(...))` 호출로 등록
- Mutex 없이 각 스레드가 전용 커넥션 사용 → 락 없는 쿼리 가능

---

### init() 동작 순서

```
1. DB_HOST / DB_DB / DB_USER / DB_PASSWORD 로 m_pDefaultConnection 생성
2. UI_DB_HOST / ... 로 m_pUserInfoConnection 생성 (USERINFO DB)
3. 시간 검증: SELECT unix_timestamp()
   → 서버 시간과 DB 시간 차이 > 3600초면 Error("Time Check Error") 발생
4. WorldDBInfo 쿼리:
   - 로그인서버: 전체 행 로드 → m_WorldConnections[WorldID] 등록
   - 게임서버:   WHERE WorldID=0 만 로드 → m_pWorldDefaultConnection
```

---

### DB.h 편의 매크로

```cpp
// 스테이트먼트 생성 (DARKEDEN DB, 현재 스레드 연결 사용)
#define NEW_STMT  g_pDatabaseManager->getConnection("DARKEDEN")->createStatement()

// 예외 처리 블록
#define BEGIN_DB  try
#define END_DB(STMT)                            \
    catch (SQLQueryException& sqe) {            \
        delete STMT;                            \
        filelog("DBError.log", "%s", ...);      \
        throw(msg.c_str());                     \
    }

// 일반 사용 패턴
Statement* pStmt = NULL;
BEGIN_DB {
    pStmt = NEW_STMT;
    Result* pResult = pStmt->executeQuery("SELECT ...");
    // ... 처리
    delete pStmt;
}
END_DB(pStmt)
```

---

### 연결 종류 요약

| 연결 변수 | DB | 용도 | 서버 |
|-----------|-----|------|------|
| `m_pDefaultConnection` | DARKEDEN | 초기화·단일 쿼리 | 전체 |
| `m_pUserInfoConnection` | USERINFO | 사용자 통계 | 전체 |
| `m_Connections[TID]` | DARKEDEN | 스레드별 게임 쿼리 | 게임서버 |
| `m_WorldConnections[WorldID]` | DARKEDEN | 멀티월드 | 로그인서버 |
| `m_CBillingConnections[TID]` | 중국 빌링 DB | 중국 빌링 로그 | 조건부 |
| `m_pDistConnection` | (주석 처리) | 분산 DB | 미사용 |

---

### 에러 처리

- DB 오류 → `SQLQueryException` → `END_DB` 매크로가 `DBError.log` 파일에 기록 후 `string` 예외로 재던짐
- 연결 오류 → `SQLConnectException` → `DatabaseManager::init()`에서 `Error`로 변환
- `executeDummyQuery()` (`SELECT 1`) : 장시간 미사용 연결의 timeout 방지

---

## 11. 게임 데이터 파일 (data/)

### 디렉토리 구성

```
data/             - 총 581개 파일
├── *.bin         - NPC/보스 퀘스트 데이터 (249개)
├── *.smp         - 존 맵 섹터 데이터 (142개)
├── *.ssi         - 존 맵 섹터 인덱스 (142개)
├── *.xml         - 이벤트·퀘스트 설정 (5개)
├── lua/          - Lua 이벤트 스크립트 (15개)
│   ├── *.lua     - Lua 원본
│   └── *.luc     - Lua 컴파일 바이트코드
├── CSAuth.idx    - 인증 인덱스
├── CSAuth.tab    - 인증 테이블
└── shrinename    - 신전 이름 목록 (한글)
```

---

### .bin 파일 - NPC/몬스터 퀘스트 데이터

```
명명 규칙: {NPC이름}.{race}.bin
예시: Boss Ash Balog.slayer.bin
      Chief Blood Warlock.vampire.bin
      Bathory.ousters.bin
```

- 83개 고유 NPC/보스 이름 × 3종족(slayer/vampire/ousters) = 249개
- 주요 보스: `Boss Ash Balog`, `Boss Crimson Slaughter`, `Boss Dark Berith`, `Boss Lord Chaos`, `Boss Turning Dead`
- 주요 치프: `Chief Alcan`, `Chief Big Fang`, `Chief Blood Warlock`, `Chief Chaos Knight` 등
- NPC와 대화하거나 처치할 때 진행되는 NPC 퀘스트 데이터 (바이너리)
- 실행 파일 경로: `copyChiefBin`, `copyQuestBin` 스크립트로 복사

---

### .smp / .ssi 파일 - 존 맵 데이터

```
명명 규칙: {zone_name}.smp  +  {zone_name}.ssi  (항상 쌍으로 존재)
예시: adam_c.smp / adam_c.ssi
      bathory_dungeon_b1f.smp / bathory_dungeon_b1f.ssi
```

- 142개 존 쌍 (smp=섹터 맵 데이터, ssi=섹터 공간 인덱스 추정)
- 클라이언트와 동일한 맵 바이너리 파일 (타일/섹터 구조)

| 지역 유형 | 예시 존 |
|-----------|---------|
| 일반 필드 | `adam_c`, `bathory_lair`, `perona_ne`, `vranco_sw` |
| 던전 | `bathory_dungeon_b1f`, `limbo_dungeon`, `hexserius_dungeon1f` |
| 성/전쟁 | `castle_hexserius`, `siege_warfare`, `rasen_battlezone` |
| 길드 본부 | `guild_army_1f`, `clan_hdqrs`, `team_hdqrs` |
| PK 구역 | `freepk`, `slayerpk`, `vampirepk` |
| 종족 마을 | `ousters_village`, `vampire_village` |
| 튜토리얼 | `tutorial_n`, `tutorial_s` |
| GDR (던전 레이드) | `gdr_illusion_01`, `gdr_lair_01`, `gdr_lair_hard` |
| 통로 | `tunnel_ghorgova`, `under_pass_1f`, `under_pass_2f` |

---

### XML 파일 - 이벤트·퀘스트 설정

| 파일 | 내용 |
|------|------|
| `TravelWay.xml` | Ousters 종족 이동 경로 허용 (종족 등급 A/B/C별 Way ID 목록) |
| `EventGQuest.xml` | 이벤트 퀘스트 정의 (id, 제목, 발생 조건, 완료 조건, 보상) |
| `EventGQuestB.xml` | 이벤트 퀘스트 추가 세트 |
| `SimpleGQuest.xml` | 단순 퀘스트 정의 |
| `EventCheckPoint.xml` | 이벤트 체크포인트 좌표 (type, zoneid, x, y) |

EventGQuest.xml 구조 예시:
```xml
<Quest id="1010">
  <Title>진화의 고리</Title>
  <Script sender="테리">...</Script>
  <Happen replay="false">
    <Race race="SLAYER"/>
    <Level min="150"/>
    <NOT><AdvancementClassLevel min="1"/></NOT>
  </Happen>
  <Complete>
    <SayNPC name="테리" target="302" index="1"/>
    <GiveQuestItem id="21"/>
    <AddEffect effectclass="462"/>
  </Complete>
</Quest>
```

---

### Lua 스크립트 - 이벤트 보상

```
data/lua/
├── xmasEventCommon.lua     - 아이템 클래스·옵션 ID 정의 공통 테이블
├── xmasEventSlayer.lua     - 슬레이어용 크리스마스 이벤트 보상
├── xmasEventVampire.lua    - 뱀파이어용 크리스마스 이벤트 보상
├── exceptMotor/            - 오토바이 없는 버전
├── withMotor/              - 오토바이 포함 버전
├── premiumEventItem*.lua   - 프리미엄 이벤트 아이템
└── TestServerReward*.lua   - 테스트 서버 보상
```

- 공통 파일에는 아이템 클래스 ID, 옵션 ID 매핑 테이블 정의
- `selectOne(t)` 함수로 무작위 선택 구현
- `.luc` 파일은 `lua5.1`으로 미리 컴파일된 바이트코드
- `lua/luaAllCompile` 스크립트로 일괄 컴파일

---

### initdb/ - 데이터베이스 스키마

| 파일 | 크기 | 내용 |
|------|------|------|
| `DARKEDEN.sql` | 4MB | 게임 메인 DB (374개 테이블) |
| `USERINFO.sql` | 32KB | 사용자 계정 DB (7개 테이블) |
| `a-setup.sql` | 0KB | 초기 설정 |

**DARKEDEN 주요 테이블 분류 (374개)**

| 분류 | 테이블 예시 |
|------|------------|
| 플레이어 | `Player`, `Slayer`, `Vampire`, `Ousters` |
| 아이템 Info/Object | `SwordInfo`/`SwordObject`, `RingInfo`/`RingObject` 등 80+ 쌍 |
| 스킬 | `SkillSave`, `SkillDomainInfo`, `SkillBalance`, `SkillTreeInfo` |
| 길드 | `GuildInfo`, `GuildMember`, `GuildWarHistory`, `GuildUnionInfo` |
| 존·세계 | `ZoneInfo`, `ZoneGroupInfo`, `WorldDBInfo`, `WorldInfo` |
| 퀘스트 | `GQuestSave`, `EventQuestStatus`, `MeetNPCQuestInfo`, `MonsterKillQuestInfo` |
| 이펙트(DB 저장) | `EffectPoison`, `EffectParalysis`, `EffectBloodDrain` 등 |
| 전쟁 | `FlagWarHistory`, `LevelWarHistory`, `RaceWarHistory`, `WarScheduleInfo` |
| 교환소 | `ExchangeListing`, `ExchangeOrder` |
| 로그 | `ItemTraceLog`, `MoneyTraceLog`, `MonsterKillLog`, `TradeLog` |
| 관리 | `IPBlockInfo`, `MACBlockInfo`, `SpeedHackPlayer`, `IllegalPlayerInfo` |
| 이벤트 | `Event200501*`, `EventGiftBox*`, `EventItemCount`, `EventLotto` |

**USERINFO 테이블 (7개)**

| 테이블 | 역할 |
|--------|------|
| `LoginData` | 로그인 기록 |
| `LoginPlayerData` | 로그인 플레이어 정보 |
| `LogoutPlayerData` | 로그아웃 정보 |
| `UserStat` | 사용자 통계 |
| `UserStatus` | 현재 상태 |
| `AccountPoint` | 계정 포인트 |
| `PointLedger` | 포인트 장부 |

---

## 12. Lua 스크립팅 (퀘스트 시스템)

### 전체 구조 개요

퀘스트/NPC 시스템은 **Trigger → Condition → Action** 패턴의 C++ 엔진이 핵심이며,
Lua는 이벤트 아이템 무작위 선택이라는 제한적인 역할만 담당한다.

---

### Trigger 시스템

#### Trigger 클래스

```cpp
class Trigger {
    TriggerType m_TriggerType;      // NPC_TRIGGER | QUEST_TRIGGER | MONSTER_TRIGGER
    TriggerID_t m_TriggerID;
    QuestID_t   m_QuestID;
    ConditionSet m_ConditionSet;    // bitset<CONDITION_MAX> - 빠른 타입 조회
    vector<Condition*> m_Conditions;
    vector<Action*>    m_Actions;
    vector<Action*>    m_CounterActions;
};
```

| 타입 | 설명 |
|------|------|
| `NPC_TRIGGER` | NPC와 대화 시 발동 |
| `QUEST_TRIGGER` | 퀘스트 진행 중 조건 충족 시 발동 |
| `MONSTER_TRIGGER` | 몬스터 AI 행동 (공격, 순찰 등) |

| 모드 | 설명 |
|------|------|
| `ACTIVE_TRIGGER` | ZoneGroupThread 루프에서 주기적으로 체크 |
| `PASSIVE_TRIGGER` | 패킷 핸들러 호출 시에만 체크 |
| `NEUTRAL_TRIGGER` | 양쪽 모두 사용 가능 |

#### TriggerManager / TriggerParser

- `TriggerParser`: DB의 `Triggers` 테이블에서 직렬화된 Condition/Action 문자열을 파싱
- `TriggerManager`: 존 단위로 트리거 목록 관리, NPC/Monster 객체에 할당
- `PropertyBuffer`: 트리거 데이터 전달용 키-값 버퍼

---

### Condition (47종, CONDITION_MAX)

```cpp
// ConditionSet = bitset<CONDITION_MAX>  ← 빠른 존재 여부 확인
```

| 카테고리 | 조건 예시 |
|----------|----------|
| **Active** (주기 체크) | AT_FIRST, AT_TIME, FROM_TIME_TO_TIME, IDLE, EVERY_TIME |
| **Passive** (이벤트 기반) | TALKED_BY, ANSWERED_BY, BLOOD_DRAINED, ATTACKED_BY, DIED_BY |
| **상태 조건** | FLAG_ON/OFF, ATTR_COMP, RACE_COMP, SAME_CLAN |
| **PC 관련** | PC_HAS_SKILL, PC_HAS_ITEM, PC_DONE_QUEST, PC_UNDER_QUEST |
| **존 입장** | ENTER_CASTLE, ENTER_HOLY_LAND, CAN_ENTER_PAY_ZONE, CAN_ENTER_GDR_LAIR |
| **전쟁** | SIEGE_DEFENDER_SIDE, SIEGE_ATTACKER_SIDE, EXIST_REINFORCE |
| **길드** | IS_GUILD_MEMBER, NOT_GUILD_MEMBER |
| **기타** | PAY_PLAY, CAN_PET_QUEST, EFFECT_FLAG |

---

### Action (ACTION_MAX ≈ 100종)

```cpp
class Action {
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL) = 0;
};
```

| 카테고리 | 액션 예시 |
|----------|----------|
| **대화** | SAY, RANDOM_SAY, ASK, ASK_DYNAMIC, QUIT_DIALOGUE |
| **아이템** | GIVE_ITEM, GIVE_QUEST_ITEM, GIVE_EVENT_ITEM, GIVE_NEWBIE_ITEM |
| **상점** | PREPARE_SHOP, SELL, BUY, REPAIR, REGEN_SHOP |
| **스킬** | TEACH_SKILL, DOWN_SKILL |
| **이동·워프** | WARP_IN_ZONE, SET_POSITION, WARP_TO_RESURRECT_POSITION, WARP_LEVEL_WAR_ZONE |
| **존 입장** | ENTER_PK_ZONE, ENTER_EVENT_ZONE, ENTER_GDR_LAIR, ENTER_CASTLE_WITH_FEE |
| **퀘스트** | SELECT_QUEST, QUEST_REWARD, CANCEL_QUEST, INIT_EVENT_QUEST, REWARD_EVENT_QUEST |
| **길드** | CREATE_GUILD, DESTROY_GUILD, SHOW_GUILD_DIALOG |
| **커플** | ACCEPT_COUPLE_REQUEST, FORCE_APART_COUPLE |
| **전쟁** | WAR_REGISTRATION, REGISTER_SIEGE, RECALL_SIEGE, ENTER_SIEGE |
| **몬스터 AI** | ATTACK, RETREAT, MOVE, ATTACK_MOVE, PATROL, HOLD_POSITION |
| **기타** | HEAL, RESTORE(뱀파이어→슬레이어 변환), TUTORIAL, MINI_GAME |

---

### Simple Quest 서브시스템 (Squest/)

```
Squest/
├── Quest / SimpleQuest        - 퀘스트 기본 클래스
├── MonsterKillQuest           - 몬스터 처치형 퀘스트
├── MonsterSelector            - 대상 몬스터 선택기
├── QuestBoard                 - 퀘스트 게시판
├── QuestFactoryManager        - 퀘스트 종류별 팩토리
├── QuestManager               - 퀘스트 인메모리 관리
├── QuestPrice/Reward/Penalty  - 보상·패널티 계산
└── SimpleQuestLoader          - SimpleGQuest.xml 파싱
```

---

### Lua 스크립팅 (제한적 역할)

#### 사용 목적

Lua는 C++ Trigger 엔진의 **이벤트 아이템 선택** 부분에만 사용된다.  
전체 퀘스트 로직은 C++으로 구현되며, Lua는 "어떤 아이템을 줄 것인가"의 무작위 결정에만 개입한다.

#### Lua 통합 클래스 계층

```
LuaState
  └── lua_State*  (Lua 5.1 / LuaJIT 2.1)
       ├── base lib
       ├── math lib
       ├── string lib
       └── io lib

LuaScript (abstract)
  ├── LuaSelectItem             - 아이템 클래스·타입·옵션 선택
  ├── LuaTradeEventSlayerItem   - 슬레이어용 이벤트 아이템 교환
  └── LuaTradeEventVampireItem  - 뱀파이어용 이벤트 아이템 교환
```

#### LuaSelectItem 동작

```cpp
class LuaSelectItem : public LuaScript {
    Item::ItemClass m_ItemClass;  // Lua 실행 후 결과값
    ItemType_t      m_ItemType;
    OptionType_t    m_OptionType;
    OptionType_t    m_OptionType2;
};
```

1. `executeFile(filename)` → Lua 파일 실행
2. Lua 스크립트 내 `selectOne(t)` 함수로 무작위 아이템 선택
3. 결과값(ItemClass, ItemType, Option)을 C++으로 반환

#### Lua 스크립트 구조 예시 (xmasEventCommon.lua)

```lua
-- 아이템 클래스 ID 테이블
ItemClassInfo = {
    ["ITEM_CLASS_RING"] = 8,
    ["ITEM_CLASS_VAMPIRE_RING"] = 30,
    -- ...
}
-- 옵션 ID 테이블
OptionInfo = {
    ["STR+2"] = 2, ["DEX+3"] = 8, ["DAM+1"] = 48, ...
}
-- 무작위 선택 헬퍼
function selectOne(t)
    return t[ random(1, getn(t)) ]
end
```

#### 컴파일

```bash
# luaAllCompile 스크립트로 .lua → .luc 변환
lua5.1 -b xmasEventCommon.lua xmasEventCommon.luc
```

- `.luc` 파일은 Lua 바이트코드 (lua5.1 컴파일)
- macOS에서는 LuaJIT 2.1을 대신 사용 (`__APPLE__` 조건부 포함)

---

### 데이터 흐름 요약

```
DB(Triggers 테이블)
    → TriggerParser.parse()
    → Trigger { Conditions + Actions } 생성
    → NPC / Monster에 할당

플레이어 상호작용(패킷 수신)
    → Trigger.isAllSatisfied() 검사
    → Trigger.activate()
        → Action.execute()
            → (일부 액션) LuaScript.executeFile()
                → Lua가 아이템 클래스·타입 반환
                → C++에서 아이템 생성 후 플레이어에게 지급
```

---

## 13. 주요 클래스 계층 구조

### Object 계층 (공통 최상위)

```
Object  (ObjectID, toString, getObjectClass, getObjectPriority)
├── Creature
│   ├── PlayerCreature  (abstract: Slayer/Vampire/Ousters 공통 기반)
│   │   ├── Slayer       (CREATURE_CLASS_SLAYER)
│   │   ├── Vampire      (CREATURE_CLASS_VAMPIRE)
│   │   └── Ousters      (CREATURE_CLASS_OUSTERS)
│   ├── Monster          (CREATURE_CLASS_MONSTER)
│   └── NPC              (CREATURE_CLASS_NPC)
├── Item
├── Effect
├── Portal
└── Obstacle
```

---

### Creature

```cpp
class Creature : public Object {
    enum CreatureClass { SLAYER, VAMPIRE, NPC, MONSTER, OUSTERS, MAX };
    enum MoveMode      { WALKING=0, FLYING, BURROWING, MAX };

    // 핵심 상태
    Zone*        m_pZone;
    ZONE_COORD   m_Coord;
    MoveMode     m_MoveMode;
    int          m_Sight;       // 기본 13 타일
    EffectManager m_EffectManager;  // bitset<EFFECT_CLASS_MAX> + Effect* 목록
    Player*      m_pPlayer;     // PC 여부 판별 (NULL이면 NPC/Monster)
};
```

| 상수 | 값 |
|------|-----|
| DEFAULT_SIGHT | 13 타일 |
| DARKNESS_SIGHT | 0 타일 |
| YELLOW_POISON_SIGHT | 3 타일 |

---

### PlayerCreature (abstract)

Slayer, Vampire, Ousters의 공통 기반. 직접 인스턴스화 불가.

| 구성 요소 | 클래스 | 설명 |
|-----------|--------|------|
| 인벤토리 | `Inventory` | 아이템 슬롯 관리 |
| 보관함 | `Stash` | 창고 아이템 |
| 상품 인벤토리 | `GoodsInventory` | 상점 재고 |
| 퀘스트 관리 | `QuestManager`, `GQuestManager` | 진행 중인 퀘스트 |
| 시간제한 아이템 | `TimeLimitItemManager` | 시간 만료 처리 |
| 펫 | `PetInfo`, `PetItem`, `Pet` | 펫 시스템 |
| 랭크 보너스 | `HashMapRankBonus` | DWORD→RankBonus* 맵 |
| 플래그 | `FlagSet` | 퀘스트 진행 상태 플래그 |
| 닉네임 | `NicknameBook` | 친구/적 목록 |
| SMS 주소록 | `SMSAddressBook` | SMS 연동 |
| 혈서 | `BloodBibleSignInfo` | 피의 성서 서명 |

---

### Slayer

```cpp
class Slayer : public PlayerCreature {
    // 능력치 상수 (기본 서버)
    SLAYER_MAX_LEVEL  = 150
    SLAYER_MAX_ATTR   = 315
    SLAYER_MAX_RANK   = 50
};
```

- 중국 서버(`__CHINA_SERVER__`) 별도 능력치 상수 존재
- BOUND_LEVEL(100) 전: 능력치 총합 330 제한, 이후 완화

---

### Vampire

```cpp
class Vampire : public PlayerCreature {
    // 시간대별 능력 배율
    VampireTimebandFactor[4] = {125, 100, 125, 150};  // 새벽/낮/저녁/밤
};
```

---

### Ousters

```cpp
class Ousters : public PlayerCreature {};
```

---

### Monster / NPC

```cpp
class Monster : public Creature {
    // FSM 기반 AI
    // Trigger/Condition/Action 시스템
    // Sector 기반 시야 최적화
};

class NPC : public Creature {
    // Trigger/Condition/Action 시스템
    // 대화, 상점, 퀘스트 제공
};
```

---

### Item 계층

```cpp
class Item : public Object {
    enum ItemClass { ... };  // 106종 (0~105)
};
```

**아이템 클래스 분류 (총 106종)**

| ID 범위 | 카테고리 | 예시 |
|---------|---------|------|
| 0 | 탈 것 | MOTORCYCLE |
| 1~6 | 소모품/기타 | POTION, WATER, HOLYWATER, MAGAZINE, ETC |
| 7 | 열쇠 | KEY |
| 8~19 | 슬레이어 장비 | RING, BRACELET, NECKLACE, COAT, TROUSER, SHOES, SWORD, BLADE, SHIELD, CROSS, GLOVE, HELM |
| 20~25 | 총기류 | SG(산탄총), SMG(기관단총), AR(돌격소총), SR(저격소총), BOMB, MINE |
| 26~28 | 기타 | BELT, LEARNINGITEM, MONEY |
| 29 | 시체 | CORPSE |
| 30~45 | 뱀파이어 장비 | VAMPIRE_RING, SKULL, MACE, SERUM, VAMPIRE_ETC, EARRING, AMULET ... |
| 46 | 퀘스트 아이템 | QUEST_ITEM |
| 49 | 피의 성서 | BLOOD_BIBLE |
| 50 | 성의 상징 | CASTLE_SYMBOL |
| 51~52 | 커플 반지 | COUPLE_RING |
| 57~69 | Ousters 장비 | ARMSBAND, BOOTS, CHAKRAM, CIRCLET, COAT, PENDENT, RING, STONE, WRISTLET, LARVA, PUPA, COMPOS_MEI, SUMMON_ITEM |
| 70~83 | 시스템 아이템 | EFFECT_ITEM, CODE_SHEET, MOON_CARD, SWEEPER, PET_ITEM, PET_FOOD, PET_ENCHANT, LUCKY_BAG, SMS_ITEM, CORE_ZAP, GQUEST_ITEM, TRAP_ITEM, BLOOD_BIBLE_SIGN, WAR_ITEM |
| 94~105 | 후기 추가 | CARRYING_RECEIVER, SHOULDER_ARMOR, DERMIS, PERSONA, FASCIA, MITTEN |

---

### Skill 시스템

```cpp
enum SkillTypes {
    // ... 0~396
    SKILL_MAX  // = 397
};
```

**스킬 도메인 분류 (종족별)**

| 도메인 | ID 범위 | 슬레이어 스킬 예시 |
|--------|---------|-----------------|
| 근접 공격 | 5~17 | DOUBLE_IMPACT, DANCING_SWORD, CROSS_COUNTER, HEAVENS_SWORD |
| 블레이드 | 18~30 | SINGLE_BLOW, GHOST_BLADE, SHADOW_WALK, ARMAGEDDON_SLASHER |
| 총기 | 31~45 | FAST_RELOAD, QUICK_FIRE, SMG_MASTERY, HEAD_SHOT, SNIPING |
| 마법(Cleric) | 46~78 | DETECT_HIDDEN, BLESS, CURE_POISON, RESURRECTION, MASS_HEAL |
| 뱀파이어 | 79~115 | BLOOD_DRAIN, POISONOUS_HANDS, ACID_TOUCH, PARALYZE, BLOODY_NAIL, HIDE, TRANSFORM_TO_WOLF, SUMMON_WOLF |
| Ousters | 116~160+ | SEDUCTION, WIND_DIVIDER, EARTHQUAKE, THUNDER_BOLT, BERSERKER, TYPHOON, PSYCHOKINESIS |

- 총 397종 스킬 (SKILL_MAX = 397)
- 시간대별 보정: 뱀파이어 밤에 150%, 낮에 100%; 몬스터 밤에 100%, 낮에 50%

---

### 주요 매니저 클래스

| 클래스 | 관리 대상 |
|--------|----------|
| `ObjectRegistry` | ZoneID별 ObjectID 할당·관리 |
| `EffectManager` | Creature별 활성 이펙트 목록 (`bitset<EFFECT_CLASS_MAX>`) |
| `InventorySlot` | 아이템 슬롯 배열 |
| `TriggerManager` | 존별 NPC/Monster Trigger 목록 |
| `SkillSlot` | PC별 스킬 슬롯 |
| `LocalPartyManager` | 파티 관리 |
| `GQuestManager` | GQuest (복합 퀘스트) 진행 상태 |
| `TimeLimitItemManager` | 시간제한 아이템 만료 추적 |

---

## 14. 게임 콘텐츠 시스템 목록

### 모듈 구성

```
src/server/gameserver/
├── skill/        (1,031파일) - 스킬 시스템
├── quest/         (311파일) - Trigger/Action/Condition 퀘스트 엔진
├── item/          (183파일) - 아이템 처리
├── mission/        (57파일) - 미션 퀘스트 (kill/NPC/gather/minigame)
├── mofus/          (39파일) - 외부 이벤트 서버 연동 (모퍼스, __METRO_SERVER__)
├── war/            (34파일) - 전쟁 시스템
├── ctf/             (7파일) - 깃발 전쟁 (Capture the Flag)
├── couple/          (8파일) - 커플 시스템
├── billing/        (12파일) - 빌링(유료) 시스템
├── exchange/        (4파일) - 교환소 (경매장) 시스템
└── gameguard/       (1파일) - GameGuard 연동
```

---

### 스킬 시스템 (skill/, 1,031파일)

- SKILL_MAX = 397종 스킬
- 종족별 도메인 구조:
  - 슬레이어: 근접(Sword/Blade) · 총기(Gun/Bomb/Mine) · 마법(Cleric/Magic)
  - 뱀파이어: 흡혈/독/산성/혈마법 · 변신(늑대/박쥐) · 소환 · 은신
  - Ousters: 바람/지진/번개/정신지배 등
- 시간대 보정 (`GameTime` 연동):
  - 뱀파이어: 밤 150%, 낮 100%
  - 몬스터: 밤 100%, 낮 50%
- 스킬 도메인 EXP 저장: `DOMAIN_EXP_SAVE_DIVIDER = 100` (DB 저장값 × 100 = 실제값)

---

### 미션 퀘스트 시스템 (mission/)

```
QuestClass {
    QUEST_CLASS_MONSTER_KILL = 0,  // 몬스터 처치
    QUEST_CLASS_MEET_NPC,          // NPC 만나기
    QUEST_CLASS_GATHER_ITEM,       // 아이템 수집
    QUEST_CLASS_MINI_GAME,         // 미니게임
}
```

| 클래스 | 역할 |
|--------|------|
| `QuestInfo` / `QuestStatus` | 퀘스트 기본 정보·진행 상태 (abstract) |
| `MonsterKillQuestInfo/Status` | 몬스터 처치 수 추적 |
| `MeetNPCQuestInfo/Status` | NPC 대화 완료 추적 |
| `GatherItemQuestInfo/Status` | 아이템 수집 추적 |
| `MiniGameQuestInfo/Status` | 미니게임 완료 추적 |
| `EventQuestInfoManager` | 이벤트 퀘스트 목록 관리 |
| `EventQuestRewardManager` | 이벤트 보상 처리 |
| `EventQuestLootingManager` | 이벤트 전리품 관리 |
| `QuestInfoManager` | 전체 퀘스트 정보 인메모리 관리 |
| `QuestManager` | 플레이어별 진행 상태 관리 |
| `RewardClass` / `RewardInfo` | 보상 클래스 (RandomRewardClass, SlayerWeaponRewardClass 등) |
| `SimpleQuestInfoManager` | SimpleGQuest.xml 로드 퀘스트 관리 |

---

### 전쟁 시스템 (war/)

```
Work (abstract)
└── War (abstract) - WarState: WAIT/CURRENT/END/CANCEL
    ├── GuildWar   - 길드 vs 길드 전쟁
    ├── RaceWar    - 종족 전쟁 (슬레이어 vs 뱀파이어)
    └── SiegeWar   - 공성전 (성 점령)

FlagWar : public Work  - 깃발 전쟁 (CTF 모듈)
```

| 클래스 | 역할 |
|--------|------|
| `WarSystem` | 전쟁 전체 관리 |
| `WarSchedule` / `WarScheduler` | 전쟁 예약·일정 관리 |
| `Schedule` / `Scheduler` | 일반 스케줄 (시간 기반 이벤트) |
| `DragonEyeManager` | 용의 눈 (전쟁 관련 특수 아이템) |
| `RaceWarLimiter` | 종족 전쟁 참가 인원 제한 |
| `*ItemPosition` | 아이템 드롭 위치 추상화 (Zone/Inventory/Corpse/Mouse/SubInventory) |

---

### CTF - 깃발 전쟁 (ctf/)

```cpp
class FlagWar : public Work {};
class FlagManager {};       // 깃발 상태 관리
class FlagWarManager {};    // 전쟁 인스턴스 관리
class NewbieFlagWar {};     // 신규자용 CTF
```

---

### 커플 시스템 (couple/)

| 클래스 | 역할 |
|--------|------|
| `CoupleManager` | 커플 매칭 관리 |
| `PartnerWaitingManager` | 매칭 대기 목록 |
| `WaitForMeet` | 결합 대기 상태 |
| `WaitForApart` | 이별 대기 상태 |

- DB 테이블: `CoupleInfo`, `CoupleRingInfo`/`VampireCoupleRingInfo`

---

### 빌링 시스템 (billing/)

외부 빌링 서버(유료 서비스)와 TCP로 연결.

```
BillingPlayer : public Player  ← 빌링 서버를 Player처럼 취급
BillingPlayerManager           ← 빌링 서버 연결 관리
BillingPlayerInfo              ← 플레이어별 빌링 정보
CommonBillingPacket            ← 빌링 전용 패킷
```

- 로그인서버에서는 빌링 비활성화 (주석 처리됨, 2003년 애드빌 요청)
- 중국 빌링: `cbilling/CBillingPlayerManager` (`__CONNECT_CBILLING_SYSTEM__`)
- PaySystem: LoginPlayer가 믹스인으로 상속

---

### 교환소 시스템 (exchange/, PR #142)

가장 최근에 추가된 모듈 (트레이딩 시스템).

```cpp
enum ExchangeResult {
    EXCHANGE_SUCCESS,
    EXCHANGE_FAIL_ITEM_NOT_FOUND,
    EXCHANGE_FAIL_ITEM_OWNERSHIP,
    EXCHANGE_FAIL_ITEM_TRADEABLE,
    EXCHANGE_FAIL_INSUFFICIENT_POINTS,
    EXCHANGE_FAIL_LISTING_NOT_FOUND,
    EXCHANGE_FAIL_INVENTORY_FULL,
    EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT,
    // ...
};
```

| 클래스 | 역할 |
|--------|------|
| `ExchangeService` | 교환소 비즈니스 로직 (거래 등록/구매/취소/수령) |
| `ExchangeDB` | DB 쿼리 추상화 |

- DB 테이블: `ExchangeListing`, `ExchangeOrder`
- `EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT`: 멱등성 처리로 중복 거래 방지

---

### 모퍼스 시스템 (mofus/)

`__METRO_SERVER__` 매크로가 정의될 때만 활성화되는 **외부 이벤트 서버 연동 모듈**.

```
MPlayer        - 모퍼스 서버를 Player처럼 취급
MPlayerManager - 모퍼스 서버 연결 관리
MPacketManager - 모퍼스 전용 패킷 처리
PKT* 클래스들  - 모퍼스 프로토콜 패킷
```

- 게임 내 이벤트 포인트(MofusPowerPoint) 연동
- 별도 로그 파일: `mofus_error.txt`, `mofus_log.txt`, `mofus_packet.txt`

---

### 아이템 시스템 (item/, 183파일)

- 106종 아이템 클래스 각각에 대한 로더/저장 로직
- `ItemLoaderManager`: 아이템 클래스별 로더 팩토리
- 인벤토리 슬롯 관리, 시간제한 아이템, 펫 아이템, 혼합 아이템(MixingItem) 등 포함

---

### 기타 시스템 (gameserver/ 루트)

| 시스템 | 주요 클래스 |
|--------|------------|
| 파티 | `Party`, `LocalPartyManager` |
| 길드 | `GuildManager` (공유서버와 동기화) |
| 펫 | `PetInfo`, `PetItem`, `Pet` |
| 동적 존(인스턴스 던전) | `DynamicZone` (5종) |
| 날씨 | `WeatherManager` |
| 채팅 | 일반/길드/파티/속삭임 채널 |
| 랭킹 | `RankBonus`, `RankBonusData` |
| 혈서 | `BloodBibleSignInfo`, `BloodBibleInfo` |
| PC방 연동 | `PCRoomIPInfo`, `PCRoomInfo`, `PCRoomPayList` |

---

## 15. 설정 파일 및 인프라 (conf/, Docker)

### 설정 파일 형식

`Properties` 클래스(`g_pConfig`)로 로드하는 `key : value` 형식의 텍스트 파일.
주석은 `#`으로 시작한다.

---

### conf/ 디렉토리 구성

```
conf/
├── gameserver.conf       - 게임서버 설정
├── loginserver.conf      - 로그인서버 설정
├── sharedserver.conf     - 공유서버 설정
├── updateserver.conf     - 업데이트서버 설정
├── excel96-*.conf        - 개발자별 로컬 설정 (excel96)
└── backup/               - 이전 설정 백업
```

---

### gameserver.conf 주요 항목

| 키 | 값 예시 | 설명 |
|----|---------|------|
| `User` | `excel96` | 운영 계정 |
| `HomePath` | `/home/.../server` | 서버 루트 경로 |
| `TCPPort` | `9998` | 클라이언트 TCP 수신 포트 |
| `LoginServerIP` | `192.168.0.16` | 로그인서버 IP |
| `LoginServerUDPPort` | `9996` | 로그인↔게임 UDP 포트 |
| `GameServerUDPPort` | `9997` | 게임서버 UDP 응답 포트 |
| `SharedServerIP` | `192.168.0.16` | 공유서버 IP |
| `SharedServerPort` | `9977` | 공유서버 TCP 포트 |
| `BillingServerIP/Port` | `111.111.111.111:1111` | 빌링서버 (미설정 시 비활성) |
| `BaseGameTime` | `1996-1-1` | 게임 내 기준 날짜 |
| `BaseRealTime` | `2000-5-1` | 현실 기준 날짜 (GameTime 계산) |
| `DB_HOST` | `192.168.0.16` | DARKEDEN DB 호스트 |
| `DB_PORT` | `3306` | MySQL 포트 |
| `DB_DB` | `DARKEDEN` | 데이터베이스 이름 |
| `DB_USER` | `elcastle` | DB 사용자 |
| `UI_DB_HOST` | `192.168.0.16` | USERINFO DB 호스트 |
| `LogServerIP/Port` | (별도 로그서버) | 로그 서버 연결 |
| `LogLevel` | `2999` | 로그 수준 |
| `WorldID` | `1` | 월드 식별자 |
| `ServerID` | `0` | 게임서버 식별자 |
| `Dimension` | `1` | 차원(서버 세트) 번호 |
| `ActiveRaceWar` | `0` | 종족 전쟁 활성화 여부 |
| `ActiveGuildWar` | `0` | 길드 전쟁 활성화 여부 |
| `ActiveFlagWar` | `0` | 깃발 전쟁 활성화 여부 |
| `ActiveLevelWar` | `0` | 레벨 전쟁 활성화 여부 |
| `IsNetMarble` | `0` | 넷마블 연동 여부 |

---

### loginserver.conf 주요 항목

| 키 | 설명 |
|----|------|
| `LoginServerPort` | `9999` TCP 클라이언트 수신 포트 |
| `LoginServerUDPPort` | `9996` 게임서버용 UDP 포트 |
| `LoginServerID` | `0` 로그인서버 식별자 |
| `LoginServerBaseID` | `100` 기본 ID 시작값 |
| `FreePlaySlayerSum` | `30` 무료 체험 슬레이어 능력치 합 |
| `FreePlayVampireLevel` | `30` 무료 체험 뱀파이어 레벨 |
| `ServerInfoReloadTime` | `1` WorldInfo/GameServerGroupInfo 재로드 주기(분) |
| `BillingServerIP/Port` | 빌링서버 |

---

### sharedserver.conf 주요 항목

| 키 | 설명 |
|----|------|
| `TCPPort` | `9977` 게임서버의 TCP 연결을 받는 포트 |
| `WorldID` | `1` |
| `Dimension` | `1` |

---

### 포트 구조 요약

```
[클라이언트]
  │─── TCP 9999 ──→ [로그인서버]
  │                   │─── UDP 9996/9997 ──→ [게임서버]
  │─── TCP 9998 ──→ [게임서버]
  │                   │─── TCP 9977 ──────→ [공유서버]

[모니터링]
  UDP 9876 ──→ MonitorClient (GM/관리 툴)
```

---

### Docker 인프라

#### 두 가지 Dockerfile

| 파일 | 기반 이미지 | 용도 |
|------|-----------|------|
| `Dockerfile.dev` | Ubuntu 20.04 | 개발환경 (빌드 도구 포함: gcc, build-essential) |
| `Dockerfile.pub` | Ubuntu 20.04 | 배포환경 (런타임 라이브러리만, 빌드 결과물 COPY) |

**Dockerfile.dev** 설치 패키지:
```
gcc, build-essential
libxerces-c-dev   (XML 파싱)
libmysqlclient-dev (MySQL 클라이언트)
liblua5.1-dev     (Lua 5.1 스크립팅)
xutils-dev        (X11 유틸리티)
psmisc            (killall 등)
```

**Dockerfile.pub** 파일 구조:
```
/home/darkeden/vs/
├── bin/      ← 컴파일된 바이너리 (loginserver, sharedserver, gameserver)
├── data/     ← 게임 데이터 파일
└── conf/     ← docker/conf/ 의 설정 파일
```

#### docker-compose.yml

```yaml
services:
  odk-mysql:
    image: mysql/mysql-server:5.7
    volumes:
      - ../initdb:/docker-entrypoint-initdb.d/   # 스키마 자동 초기화
    command: mysqld --sql_mode="ONLY_FULL_GROUP_BY,..."
    # NO_ZERO_DATE, STRICT_TRANS_TABLES 제거 필수

  odk-server:
    image: tiancaiamao/darkeden:latest
    ports:
      - "9999:9999"   # loginserver TCP
      - "9998:9998"   # gameserver TCP
      - "9997:9997"   # gameserver UDP
      - "9997:9997/udp"

networks:
  odk-network:
```

#### 주의사항

- MySQL **5.7** 필수 (`sql_mode` 문제로 8.x 일부 호환 문제 가능)
- `NO_ZERO_DATE` 와 `STRICT_TRANS_TABLES` 는 반드시 SQL 모드에서 제거해야 함 (레거시 날짜 데이터 때문)
- 게임 서버와 DB 서버 간 시간 차이가 **3600초 초과 시 서버 시작 실패** (`DatabaseManager::init()` 시간 검증)
- `WorldDBInfo` 테이블과 설정 파일의 `DB_HOST`/`DB_DB`가 일치해야 함
- `HomePath` 는 서버가 실제로 실행되는 위치의 **절대 경로**여야 함 (데이터 파일 로드 기준점)

---

### 서버 시작 순서

```bash
./bin/loginserver  -f ./conf/loginserver.conf
./bin/sharedserver -f ./conf/sharedserver.conf
./bin/gameserver   -f ./conf/gameserver.conf
```

공유서버 → 로그인서버 → 게임서버 순으로 기동해야 한다.
(게임서버가 공유서버에 TCP 연결, 로그인서버와 UDP 교환)

---

## 16. CI/CD 및 코드 품질 관리

### GitHub Actions - 포맷 체크

```
.github/workflows/format-check.yml
```

**트리거 조건**: `master` 브랜치 대상 PR이 열리거나 변경될 때

**동작 순서**:
```yaml
1. actions/checkout@v4 (fetch-depth: 0, 전체 히스토리)
2. clang-format 설치
3. git diff --name-only origin/master HEAD | grep '\.(cpp|h|hpp)$'
   → 변경된 C++ 파일 목록 추출
4. 각 파일에 대해:
   clang-format FILE | diff -q FILE -
   → 포맷 불일치 시 [FAIL] 출력
5. 하나라도 실패 시 exit 1 → PR 머지 차단
```

- **변경 파일만 검사** (전체 파일 아님) → 빠른 속도
- 실패 시 메시지: `"Please run 'make fmt' to fix formatting issues."`

---

### clang-format 설정 (.clang-format)

`BasedOnStyle: LLVM`을 기반으로 커스터마이즈.

| 설정 항목 | 값 | 설명 |
|-----------|-----|------|
| `IndentWidth` | 4 | 들여쓰기 4칸 |
| `TabWidth` | 4 | 탭 너비 4칸 |
| `UseTab` | Never | 탭 문자 금지, 공백 사용 |
| `ColumnLimit` | 120 | 줄 길이 최대 120자 |
| `PointerAlignment` | Left | `int* p` (포인터 왼쪽 정렬) |
| `BreakBeforeBraces` | Attach | `if (x) {` (K&R 스타일) |
| `AllowShortFunctionsOnASingleLine` | Empty | 빈 함수만 한 줄 허용 |
| `AllowShortIfStatementsOnASingleLine` | false | if 한 줄 금지 |
| `AllowShortLoopsOnASingleLine` | false | 루프 한 줄 금지 |
| `SortIncludes` | true | `#include` 자동 정렬 |
| `IncludeBlocks` | Regroup | 인클루드 블록 재그룹화 |
| `MaxEmptyLinesToKeep` | 2 | 최대 빈 줄 2개 |
| `AccessModifierOffset` | -4 | `public:` 들여쓰기 제거 |
| `AlignTrailingComments` | true | 후행 주석 정렬 |
| `ReflowComments` | false | 주석 자동 줄바꿈 금지 |

**인클루드 우선순위**:
```
1. <system.h>       (C 시스템 헤더)
2. <cpplib>         (C++ STL)
3. <other libs>     (기타 <>)
4. "project.h"      (프로젝트 헤더)
```

---

### Makefile 빌드 명령

| 명령 | 동작 |
|------|------|
| `make` / `make debug` | CMake Debug 빌드 (기본) |
| `make release` | CMake Release 빌드 |
| `make fmt` | 전체 소스 clang-format 적용 |
| `make fmt-check` | 변경된 파일만 포맷 검사 (빠름) |
| `make fmt-check-all` | 전체 파일 포맷 검사 (느림) |
| `make clean` | `build/`, `bin/`, `lib/` 삭제 |

```bash
# 병렬 빌드 (자동 CPU 수 감지)
cmake --build build -j$(sysctl -n hw.ncpu || nproc || echo 4)
```

---

### 최근 주요 변경 이력 (git log)

| 커밋 | 내용 |
|------|------|
| `3f183364` | 교환소(트레이딩) 시스템 WIP 구현 (#142) |
| `acf4267c` | 빌드 수정 + 소스 인코딩 UTF-8 전환 (#141) |
| `69abbad0` | GitHub Actions 포맷 체크 추가 |
| `91d2a405` | `make fmt` 명령 도입 |
| `0341cf6e` | 빌드 시스템 CMake로 전환 (#140) |
| `3c2054f5` | 길드 채팅 버그 수정 (#131) |
| `d3674cb5` | MySQL `STRICT_TRANS_TABLES` 제거 (#116) |
| `79f368d9` | Effect ID 클라이언트 동기화 (#95) |
| `cd60b527` | Lua 5.1 업그레이드 (#92) |
| `6275b979` | IP_t 크기 수정으로 파티 참가 버그 수정 (#82) |

---

### 코드 품질 현황

| 항목 | 상태 |
|------|------|
| 포맷 체크 | GitHub Actions CI로 PR마다 강제 검사 |
| 빌드 CI | 없음 (빌드 테스트 자동화 미구현) |
| 단위 테스트 | 없음 (test/, testAlone/ 는 수동 테스트 파일) |
| 정적 분석 | 없음 |
| 소스 인코딩 | UTF-8 (PR #141에서 마이그레이션, 레거시 일부 잔존) |
| 주석 언어 | 일부 한국어 → 영어 전환 중 (CLAUDE.md 지침) |

---

### 개발 환경 요건

| 요건 | 내용 |
|------|------|
| C++ 표준 | C++11 |
| CMake | 3.16 이상 |
| 컴파일러 | GCC (Ubuntu 20.04 기준) |
| clang-format | apt-get install clang-format |
| MySQL | 5.7 (sql_mode 조정 필요) |
| Lua | lua5.1-dev 또는 LuaJIT 2.1 (macOS) |
| Xerces-C | libxerces-c-dev 3.2.3 |

```bash
# Ubuntu 의존성 일괄 설치
sudo apt install libxerces-c-dev libmysqlclient-dev liblua5.1-dev \
                 build-essential cmake clang-format
```

---

해당 프로젝트를 둘러보고 어떤 내용인지 
/분석관련/프로젝트 개요.md 파일에 분석한 내용 작성해줘 

1. filelist.txt에 프로젝트 구조를 담았어. 우선 이거를 먼저 읽어보고
   분석하지 않아도 되는 파일 ( 산출물 obj 같은 파일등 ) 확장자가 보이면
   분석 제외.md 파일을 filelist.txt 와 같은 형식으로 작성해줘 

 2. 프로젝트 개요.md에 세세한 항목은 너무 길게 작성하지 말고 
  대 제목만 달아서 나중에 하나하나 분석해보려고해.
