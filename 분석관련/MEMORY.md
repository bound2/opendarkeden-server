# DarkEden 서버 프로젝트 메모리

## 프로젝트 개요

- **프로젝트**: DarkEden MMORPG 서버 오픈소스 (`dk_server`)
- **언어**: C++11
- **빌드**: CMake 3.16+ (`make` → Debug, `make release` → Release)
- **포맷**: clang-format (`make fmt`), PR마다 GitHub Actions 검사

## 분석 문서

- **통합 분석 파일**: `분析관련/DarkEden_전체분석.md` (81KB, 16개 섹션 전체 포함)
- **인덱스**: `분析관련/프로젝트 개요.md`
- **개별 파일**: `분析관련/01~16. *.md`
- **분析 제외 목록**: `분析관련/분析제외.md`

→ 자세한 분석 내용은 `분析관련/DarkEden_전체분析.md` 참조

## 핵심 아키텍처

- **3개 서버 프로세스**: loginserver(9999) → sharedserver(9977) → gameserver(9998)
- **패킷**: 7바이트 헤더, PACKET_MAX=501, TCP(GC/CG/LC/CL) + UDP(GL/LG)
- **세계 구조**: ZoneGroup → Zone → Tile → `forward_list<Object*>`
- **크리처 계층**: Object → Creature → PlayerCreature → Slayer/Vampire/Ousters
- **DB**: MySQL 5.7, DARKEDEN(374테이블) + USERINFO(7테이블)

## 파일 경로 주의사항

- `분析관련/` 디렉토리 이름에 한국어 포함 → Read/Write 도구 직접 사용 불가
- **반드시 Python subprocess 사용**: 디렉토리 경로 = `'\ubd84\uc11d\uad00\ub828'`
- 파일 읽기/쓰기 예시:
  ```python
  path = os.path.join(r'C:\Users\USER\Desktop\Github_clone\dk_server', '\ubd84\uc11d\uad00\ub828', 'filename.md')
  ```

## 주요 상수

- `BUILD_NUMBER` = 40518
- `PACKET_MAX` = 501
- `SKILL_MAX` = 397
- `EFFECT_CLASS_MAX` (bitset으로 Effect 관리)
- Item 클래스: 106종 (0~105)
- 존 맵: 142개 (.smp/.ssi 쌍)
- NPC 퀘스트 바이너리: 83종 NPC × 3종족 = 249개 .bin 파일
