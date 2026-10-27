# Day 4 — 분류·기본 필터·정렬·출력 테스트

실행: `make test-4`

전체 출력 연결 후: `make test-4 DAY4_NM=/absolute/path/to/ft_nm`
별도 실행: `DAY4_NM=/absolute/path/to/ft_nm bash tests/day4/test_day4.sh`
후보 실행 파일은 직접 새로 빌드해야 한다. 테스트가 main을 대신 구현하거나
필터·정렬·출력을 자체 구현하여 통과시키지 않는다.

관리자 권한이 없어 sanitizer 런타임을 설치할 수 없다면 다음처럼 실행한다.
기본값은 기존과 같은 `address,undefined`이며 선택한 설정을 실행 시 표시한다.

```bash
# UBSan 없이 ASan만 사용
DAY4_SANITIZERS=address ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make test-4 DAY4_NM="$PWD/ft_nm"
# ASan도 사용할 수 없다면 sanitizer 없이 기능 검사
DAY4_SANITIZERS=none make test-4 DAY4_NM="$PWD/ft_nm"
```

어느 설정이든 분류 74개와 출력 비교 2개는 동일하게 실행한다.
`address`는 UBSan 검사를, `none`은 ASan/UBSan 검사를 수행하지 않는다.
이 설정에서 통과해도 생략한 sanitizer 검사를 통과한 것으로 해석하지 않는다.

## 분류 검사: 37개 × ELF32/ELF64 = 74개

`day4_type_test.c`는 실제 `symbol_type()`과 실제 section reader를 연결한다.
테스트용 섹션 테이블은 검증 완료된 테이블을 받은 상황을 표현한다.
헤더 파서나 심볼 수집의 완전성 검사는 Day 2/3의 책임이다.
네이티브 프로세스에서 두 ELF 형식의 구조체를 읽으며 32비트 ABI 테스트는 아니다.

- U, A/a, C; weak object V/v, function W/w, NOTYPE W/w.
- 일반 심볼 T/t, D/d, B/b, R/r; section index 2를 사용해 STB_WEAK와 혼동하는 회귀 검사.
- STT_NOTYPE text, merge/string 추가 플래그, group 추가 플래그, TLS data/BSS.
- DYNAMIC·INIT_ARRAY·FINI_ARRAY·PREINIT_ARRAY의 global/local D/d 및 NOTE의 R/r.
- non-alloc NOTE는 기존 ? 동작 유지, writable text는 T로 실행 속성 우선. non-alloc NOTE의 GNU nm 일치 여부는 이 합성 검사로 보장하지 않는다.
- 잘못된 일반 index 두 종류는 -1 반환.
- 성공 시 정확한 문자와 반환값, 모든 경우 원래 metadata의 보존.
- 특수 index는 테이블에 존재하지 않아도 분류 성공해야 한다.
- ASan/UBSan, 개별 실행 timeout, 구현 빌드 실패 시 런타임 미실행 명시.

## 기본 필터·정렬·출력: 실제 .o 두 개

`day4_fixture.s`를 각각 -m32/-m64로 어셈블한다. libc/multilib 링크는 불필요하다.
이름과 섹션 순서를 섞고, 서로 다른 주소, local/global, undefined, weak,
absolute, common, TLS, merge strings와 STT_FILE을 포함한다.
INIT_ARRAY·FINI_ARRAY·PREINIT_ARRAY·할당된 NOTE 섹션의 실제 심볼도 포함한다.
STT_SECTION은 assembler가 생성하는 경우 함께 비교한다.

`day4_compare.py`는 양쪽 LC_ALL=C에서 옵션 없는 시스템 nm과 후보를 실행한다.
stdout 바이트를 그대로 비교해 필터, 레코드 정렬, 값·문자·이름 결합,
8/16자리 값 폭, undefined 값 공백을 검사한다. stderr와 종료 상태는 별도로 검사한다.
파일명 차이나 공백·정렬 차이를 정규화하지 않는다.

정렬/필터/출력 함수의 공개 API가 아직 없으므로 개별 함수 테스트는 만들지 않았다.
CLI를 연결한 뒤 위 비교를 실행해야 Day 4 전체 범위를 검증할 수 있다.
실행 파일·공유 라이브러리·복수 인자·기본 a.out은 Day 5 이후 범위다.

## 결과 해석

- 종료 0: 분류 74개 및 실제 출력 2개 통과.
- 종료 1: 빌드 실패 또는 검사 실패. 빌드 실패는 74개 런타임 실패로 세지 않는다.
- 종료 2: 실패는 없지만 도구/후보 미제공으로 미실행 항목 존재.
- UNAVAILABLE는 PASS가 아니며 Day 4 완료를 뜻하지 않는다.

테스트 자체의 comparator 검증에는 시스템 nm을 후보로 사용할 수 있으나,
그 결과는 ft_nm 구현 검증으로 계산하면 안 된다.

## 2026-10-27 분류 확장 결과

기존 분류는 일반 심볼에서 PROGBITS/NOBITS만 허용해 DYNAMIC·배열·NOTE
심볼을 ?로 남겼다. 할당된 섹션은 실행 속성이 있으면 T/t, 그 외 NOBITS는
B/b, 쓰기 가능하면 D/d, 나머지는 R/r로 분류한다. local binding에는 소문자를
적용하며 undefined/absolute/common/weak의 기존 선행 처리는 유지한다.

새 검사로 수정 전 54 PASS / 22 FAIL을 재현했다. 수정 후 ASan(누수 제외)에서
**76 PASS / 0 FAIL / 0 UNAVAILABLE**를 확인했다. 분류 74개와 실제 ELF32/64
출력 비교 2개다. 별도 Day5 통합 비교에서도 정상 형식 50건 및 nm이 수용한
공식 입력 6건이 모두 일치했다. UBSan과 누수 검사는 환경 제약으로 제외했다.
이 결과는 GNU unique·IFUNC 등 모든 심볼 종류의 완전 지원을 뜻하지 않는다.
