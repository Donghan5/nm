# ft_nm 구현 마일스톤

최종 갱신: 2026-10-27. 저장소 소스와 5일 차 통합 테스트 실행 결과 기준. 날짜는 실제 경과 시간이 아니라 구현 순서상의 일차를 뜻한다.

## 현재 위치

**심볼 분류 수정 후 통합 비교 57 PASS / 35 FAIL(총 92건).** 기존 21 PASS / 71 FAIL에서 분류로 인한 실패 36건을 해소했다. 정상 형식 50건과 nm이 수용한 공식 바이너리 6건이 완전 일치한다. 확장한 Day4 회귀는 76 PASS이며 CLI ASan 25건은 누수 검사 없이 오류·충돌·timeout이 없다. 복수 파일 출력·옵션·오류 진단과 잔여 검증이 남아 5일 차는 미완료다.

- `src/main.c`: 파일 처리 → 수집 → 섹션 테이블 준비 → 분류 → 정렬·출력 → 해제를 연결했다. 인자가 없을 때 a.out 처리, 여러 파일 순회와 오류 후 계속 처리를 확인했다. 복수 파일의 파일명 헤더는 누락된다.
- `src/elf/elf_ident.c`, `elf32.c`, `elf64.c`: ELF 식별·헤더·테이블 검증과 reader 구현. 앞선 2일 차 테스트 61개 통과.
- `src/symbol/symbol_collect.c`, `symbol_validate.c`: 심볼 수집·연결 테이블·이름·index 검증. 앞선 3일 차 테스트 46개 통과. st_value와 st_size를 각각 s_addr와 s_size에 보존한다.
- `src/symbol/symbol_type.c`: 특수 index·weak 선행 처리 후 할당된 섹션의 실행·쓰기 속성과 NOBITS 여부로 분류한다. DYNAMIC·INIT/FINI/PREINIT_ARRAY·NOTE 분류 차이를 해소했다.
- `src/sort/sort_symbols.c`, `src/print/print_symbols.c`: 레코드 정렬, 빈 이름·FILE·SECTION 필터, 클래스별 출력 폭, COMMON 크기 출력 구현.
- 전체 프로그램과 새 디렉터리 빌드 성공. Day4 분류와 전체 CLI는 ASan으로 검사했다. UBSan 런타임이 없고 LeakSanitizer는 ptrace 환경 오류로 실행하지 못했다. 출력 비교 대상 CLI는 일반 빌드다.

남은 분량은 **2·3일 차 잔여 항목 + 통합 5일 차(기존 5~7일 차와 보너스 플래그 2개)**다. 하루 집중 일정으로 계획하되, 발견한 결함과 미실행 항목은 완료로 표시하지 않는다. 5일 차 필수 형식·인자·오류 경로를 실행했으나 출력 불일치와 미실행 검사가 남아 완료로 판단하지 않는다.

## 전체 일정과 현재 상태

| 일차 | 구현 목표 | 완료 기준 | 현재 상태 |
|---|---|---|---|
| 1일 차 | PDF로 요구사항 확정, 테스트 빌드 구성, 파일 열기·크기 확인·해제 | 정상·오류 입력의 반환과 정리, 크기 변환 경계값 확인 | 완료: 기본 8 PASS / 1 N/A, 32비트 3 PASS |
| 2일 차 | ELF 헤더와 섹션 테이블 검증 | 필요한 구조를 파일 범위 밖 접근 없이 탐색 | 주요 구현·61개 테스트 통과; 섹션 payload 검증 잔여 |
| 3일 차 | 심볼 테이블과 연결된 문자열 테이블 읽기 | 작은 `.o`에서 심볼 이름·값·속성 추출 | 기본 범위 완료: 46 PASS; SHN_XINDEX 해석 미지원 |
| 4일 차 | 심볼 종류 판정, 기본 필터·정렬·출력 | 작은 입력에서 시스템 `nm`과 출력 일치 | 분류 확장 회귀 완료: 76 PASS / 0 FAIL / 0 UNAVAILABLE |
| 5일 차 — 하루 통합 | 기존 5~7일 차: 필수 형식·인자·오류·메모리 검증, 보너스 -r/-p, 학교 공식 테스트, 제출 재현·설명 | 필수·보너스 출력 비교, 공식 바이너리 검증과 회귀 검사, 새 환경 빌드 및 설명 | 통합 비교 57 PASS / 35 FAIL; 분류 불일치 36건 해소; 출력·옵션·진단 수정 및 제출 점검 잔여 |

## 목표 아키텍처

```text
main.c: 입력 결정 → 파일별 처리 → 최종 종료 상태 집계
  │
  ├─ file.c: 초기화 → open → fstat → mmap → close
  ├─ elf/elf_ident.c: ELF 식별, 클래스·바이트 순서 확인
  │    ├─ elf64.c: ELF64 헤더·섹션 검증 및 해석
  │    └─ elf32.c: ELF32 헤더·섹션 검증 및 해석
  ├─ symbol/symbol_collect.c: 심볼·문자열 연결, 공통 레코드 수집
  ├─ symbol/symbol_type.c: 출력 문자 분류
  ├─ sort/sort_symbols.c: 심볼 레코드 정렬
  ├─ print/print_symbols.c: 값・문자・이름 출력
  └─ 심볼 배열 해제 → file.c의 munmap

공통 기능: 범위 검사, 오류 보고
```

파일 매핑의 소유자는 파일 처리 계층이다. 현재 문자열 getter는 매핑 내부 주소를 반환하고 수집부는 이름을 t_symbol.s_name 배열에 검증 후 복사한다. 반환 주소와 내부 배열은 개별 free하지 않으며, 결과 심볼 배열은 호출자가 해제한다. 소속 섹션을 이용한 후속 분류가 끝날 때까지 매핑을 유지한다. ELF32와 ELF64의 공통 레코드 및 분류·정렬·출력 연결은 구현됐으며, 작은 .o 두 종류에서 전체 흐름을 확인했다.

## 일차별 작업

### 1일 차 — 완료: 파일 처리 기본 범위와 크기 변환 검증

- [x] PDF 원문과 mandatory 범위 대조 완료 — 사용자 확인.
- [x] 실제 파일 처리 코드와 필요한 의존성을 테스트 타깃으로 빌드한다.
- [x] `f_name`을 문자 배열로 정하고 내부 배열의 잘못된 `free()`를 제거한다.
- [x] `t_symbol` typedef와 테스트에 필요한 헤더·함수 선언을 정리한다.
- [x] 정상 파일의 크기·파일명·매핑 내용과 fd를 닫은 뒤 읽기를 확인한다.
- [x] 빈 파일·없는 파일·디렉터리의 실패 반환과 정리를 검사한다.
- [x] 초기 상태 정리, 정리 후 `f_data == NULL`·`f_size == 0`, 반복 정리를 확인한다.
- [x] 같은 구조체로 64회 처리·정리하며 fd 누수를 검사한다.
- [x] 음수 크기를 거부하고 변환 가능한 최대 크기를 보존한다.
- [x] 32비트 size_t·64비트 off_t 구성에서 `SIZE_MAX + 1` 거부를 검사한다.
- [x] `open_file()`의 사용하지 않는 `file` 인자를 제거한다.

완료 근거:

| 명령 | 결과 | 근거 |
|---|---|---|
| `make test-1` | 8 PASS / 0 FAIL / 1 N/A | 앞선 에이전트 실행 및 사용자 결과 |
| `make test-1-32` | 3 PASS / 0 FAIL / 0 N/A | 사용자가 제공한 실행 결과; 이번 문서 갱신 시 재실행하지 않음 |

기본 테스트의 N/A는 해당 플랫폼의 off_t가 SIZE_MAX 초과 값을 표현할 수 없기 때문이다. 해당 거부 경로는 별도 32비트 테스트에서 검증했다.

완료 범위는 테스트로 확인한 기본 계약이다. 현재 빈 파일은 길이 0 mmap 실패로, 디렉터리는 명시적인 파일 종류 검사 없이 매핑 실패로 거부한다. MAP_FAILED에 대한 munmap 시도를 막는 방어, 오류 종류별 진단, fstat/mmap 실패 강제 주입과 매핑 누수 검사는 통합 5일 차에 보강한다. main 연결과 전체 프로그램 빌드는 Day4에서 확인했다.

### 2일 차 — 주요 구현 검증 완료, 섹션 payload 검사 잔여

- [x] 공통 범위 검사 `offset <= size && length <= size - offset`을 도입한다.
- [x] 최소 식별 길이, magic, class, endian, version을 검사한다.
- [x] ELF32/ELF64 각각 헤더 크기·type·machine과 테이블 엔트리 크기를 검사한다.
- [x] Program header 없음·일반 개수·PN_XNUM 확장 개수를 처리한다.
- [x] 섹션 테이블 없음·일반 개수·확장 개수를 처리하고 실제 개수로 전체 범위를 검사한다.
- [x] 곱셈 overflow와 ELF64 offset/개수의 size_t 변환 범위를 검사한다.
- [x] 헤더·섹션을 지역 구조체로 복사해 원본 주소 정렬 문제를 피한다.
- [x] Getter 성공 0/실패 -1, reader 유효 true/무효 false 계약을 적용한다.
- [x] `make test-2`: 61 PASS / 0 FAIL. ASan·UBSan을 활성화해 앞선 에이전트 실행에서 확인했다.
- [ ] 개별 섹션의 `sh_offset`, `sh_size`를 검사한다. SHT_NOBITS는 실제 파일 payload가 없으므로 동일한 바이트 범위 규칙을 적용하지 않는다. SHT_NULL 등 특수 필드 의미도 구분한다.
- [ ] 위 payload 검사에 대한 정상·손상·NOBITS 테스트를 추가한다.

현재 테스트는 합성 바이트 fixture로 ELF32·ELF64 getter와 reader를 연결한 검사다. 실제 실행 파일·공유 라이브러리의 완전한 유효성이나 전체 nm 출력을 검증한 것은 아니다. 테스트는 네이티브 ABI에서 두 파일 형식을 읽으며, 32비트 실행 프로그램의 ELF64 축소 변환 경로는 별도 실행 검증이 필요하다.

세부 범위와 반환 계약은 `tests/day2/DAY2.md`에 기록했다. 61개 통과를 2일 차 전체 완료로 확대하지 않는다. ELF32 테이블 해석은 당초 5일 차 계획보다 먼저 구현됐으며, 심볼·출력 연결은 Day4에서 완료했다.

### 2일 차 잔여 작업 — 일반 섹션 payload 검증

1. Getter로 검증한 테이블을 얻고 `index < table.count`로 reader를 호출한다.
2. 읽은 섹션의 종류를 확인한다. 실제 파일 바이트가 있는 섹션은 offset/크기의 표현 범위와 파일 범위를 검사한다.
3. SHT_NOBITS는 파일에서 sh_size 바이트를 읽지 않는다. SHT_NULL/섹션 0번의 확장 정보도 일반 데이터로 해석하지 않는다.
4. 일반 섹션 payload와 NOBITS 테스트를 보강한다. 심볼·문자열 테이블 payload는 3일 차에서 별도로 검증·테스트했다.

### 3일 차 — 기본 범위 완료: 심볼과 문자열 연결

구현 흐름: 섹션 순회 → SHT_SYMTAB 발견 → sh_link로 문자열 테이블 선택 → 원본 심볼 복사 → 이름 연결 → 공통 레코드 저장. 분류·정렬·최종 출력은 4일 차 작업이다.

- [x] 섹션 이름 대신 sh_type == SHT_SYMTAB으로 일반 심볼 테이블을 찾는다.
- [x] sh_link의 범위를 검사하고 연결된 섹션이 SHT_STRTAB인지 확인한다.
- [x] 심볼·문자열 테이블 payload 범위, ELF64 size_t 변환 범위를 검사한다.
- [x] sh_entsize와 엔트리 크기, sh_size의 나머지를 검증한다.
- [x] st_name 범위와 문자열 테이블 내부 NUL 종료를 확인한다.
- [x] 이름을 배열 크기에 맞는지 확인하고 복사하며, 값·binding·type·section index·원래 순서를 저장한다.
- [x] s_addr를 uint64_t로 저장해 64비트 심볼 값을 보존한다.
- [x] SHN_UNDEF/ABS/COMMON과 일반 index를 구분하고 잘못된 index를 거부한다.
- [x] 할당 후 실패하면 배열을 해제하고 symbols/count를 초기화한다.
- [x] 심볼 테이블 없음은 빈 결과로, 손상은 실패로 구분하고 DYNSYM으로 대체하지 않는다.
- [x] make test-3: 46 PASS / 0 FAIL / 0 UNAVAILABLE, ASan·UBSan 활성화.
- [x] 실제 ELF32/ELF64 .o 각각 11개 레코드를 독립 Python 참조 파서와 비교해 일치한다.
- [ ] SHN_XINDEX를 SHT_SYMTAB_SHNDX와 연결해 실제 section index로 해석한다. 현재는 안전한 거부만 검사했다.

완료 범위: 네이티브 실행 프로그램으로 두 ELF 형식의 기본 수집 경로를 검증했다. 46개 통과가 SHN_XINDEX 지원이나 최종 nm 출력 일치를 뜻하지 않는다. 이름이 고정 배열을 초과하면 현재 구현은 거부하며, 이는 모든 유효한 긴 이름을 지원한다는 뜻이 아니다. 32비트 ABI의 ELF64 축소 경로, 강제 할당 실패 등은 추가 검증 대상이다.

세부 테스트와 제한: `tests/day3/DAY3.md`. 합성 입력은 44개이며, 실제 .o 비교 2개를 합쳐 46개다. getter/reader/수집/검증의 실제 구현을 연결하고 미완성 분류 코드는 링크하지 않는다.

### 4일 차 — 기본 범위 완료: 비교 가능한 nm 출력

- [x] undefined, text, data, BSS, read-only, absolute, common, weak 등의 분류를 구현한다.
- [x] binding과 섹션 속성을 함께 사용해 대소문자를 결정한다.
- [x] 빈 이름, STT_FILE, STT_SECTION을 기본 출력에서 제외한다.
- [x] 이름만 분리하지 않고 심볼 레코드 전체를 정렬한다.
- [x] 값·공백·출력 문자·이름을 출력하고, ELF32는 8자리, ELF64는 16자리로 출력한다.
- [x] SHN_COMMON은 st_value의 정렬 요구 대신 st_size를 출력한다. 일반 심볼의 주소는 유지한다.
- [x] main에서 수집 → 분류 → 정렬·필터·출력 → 해제를 연결한다.
- [x] 분류 50개와 실제 ELF32·ELF64 .o 출력 비교 2개를 통과한다.

2026-10-27 검증:

```bash
make -j4 CC=/usr/bin/gcc
DAY4_SANITIZERS=address ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make test-4 CC=/usr/bin/gcc DAY4_NM="$PWD/ft_nm"
```

결과: **52 PASS / 0 FAIL / 0 UNAVAILABLE**. 빌드 성공 후 후속 make에서도 재빌드가 없음을 확인했다. GCC 16.2.1, GNU nm 2.46.1, LC_ALL=C 기준이다. 출력 비교는 stdout 바이트 전체와 stderr·종료 상태를 검사한다. 기존 빈 이름·FILE 출력과 COMMON 값 오류가 해소됐다.

분류 테스트에 ASan을 사용했다. 학교 환경에서 UBSan 런타임을 설치할 관리자 권한이 없어 UBSan은 제외했으며, 누수 검사도 비활성화했다. 기본 sanitizer 설정은 address,undefined를 유지하고 DAY4_SANITIZERS로 선택할 수 있다. CLI 출력 비교는 sanitizer 없는 일반 빌드를 사용하므로 전체 처리 흐름의 메모리 검사 완료를 뜻하지 않는다.

완료 범위는 작은 ELF32·ELF64 .o와 명시된 분류 사례다. 모든 심볼 종류·동일 이름 정렬·실행 파일·공유 라이브러리·복수 인자의 정확성을 보장하지 않는다. 세부 범위는 `tests/day4/DAY4.md`에 기록했다.

### 5일 차 — 하루 통합: 필수·보너스 검증과 제출 마무리

기존 5·6·7일 차 작업을 아래 순서로 하루에 진행한다. 오전에는 입력별 차이를 확인하고, 오후에는 결함 수정·보너스 검증, 마지막에는 제출 상태 재현과 회귀 검사를 수행한다. 2026-10-27에 아래 통합 테스트를 실행했다. 실패를 확인한 항목은 완료로 표시하지 않으며, 세부 결과는 이 절 마지막에 기록한다.

#### 1. 필수 형식과 인자 처리 — 기존 Day5

- [x] main의 기본 파일 처리 호출을 연결하고 전체 make 빌드 성공을 확인한다.
- [x] ELF32/ELF64 테이블 해석을 수집·분류·정렬·출력 흐름에 연결한다.
- [x] ELF32/ELF64 각각 .o·정적 실행 파일·PIE·.so와 C 실행 파일을 비교한다. 분류 수정 후 기본/-r/-p/동시 지정의 50건이 완전 일치한다.
- [x] 인자가 없을 때 a.out 처리와 여러 파일 순회를 검증한다. a.out 정상 입력은 완전 일치; 복수 파일 순회는 수행되나 출력 헤더는 누락된다.
- [ ] 파일별 오류 보고, 오류 후 다음 파일 처리, 최종 종료 상태를 검증한다.
- [x] 오류 보고·정리 함수가 파일별 처리 흐름을 중단하지 않는지 확인한다. 없는 파일 뒤 정상 파일, 정상·빈 파일·정상 조합에서 후속 출력과 최종 종료 상태 1을 확인했다.
- [ ] 심볼 없는 파일과 stripped 파일을 안전하게 처리한다.
- [ ] 2일 차 섹션 payload 검사와 3일 차 SHN_XINDEX 잔여 항목을 보강한다.

archive 필수 여부는 PDF 원문과 별도로 확인하며 임의로 확정하지 않는다.

#### 2. 정상·손상 입력과 메모리 검사 — 기존 Day6

- [ ] 빈 파일·파일 종류를 명시적으로 검사하고 MAP_FAILED를 munmap하지 않도록 보강한다.
- [ ] fstat/mmap 실패 강제 주입, 오류 종류별 진단과 매핑 누수를 검사한다.
- [x] LC_ALL=C에서 시스템 nm과 stdout·stderr·종료 상태를 각각 비교한다. 92건 중 세 항목 모두 일치한 것은 21건이다.
- [ ] PIE/non-PIE, local/global, weak, common, 같은 이름 심볼 등 사례를 넓힌다.
- [x] 없는 파일·빈 파일·디렉터리·권한 오류·여러 파일 혼합 입력을 검사한다. 기본 오류 입력은 종료 상태 1; 진단과 복수 파일 출력 차이는 미해결이다.
- [ ] 잘린 테이블, 잘못된 offset/index/entry size, NUL 없는 문자열을 검사한다.
- [x] 전체 CLI의 정상·오류 경로에 ASan을 적용한다. 25건에서 ASan 오류·충돌·timeout 없음. 누수 검사와 UBSan은 SKIP이며 메모리 검증 전체 완료를 뜻하지 않는다.
- [ ] timeout으로 손상 입력의 충돌·무한 반복을 확인하고 원인을 수정한다.

학교 환경에는 UBSan 런타임과 sudo 권한이 없다. 설치를 전제로 삼지 않고 사용 가능한 검사로 진행한다. 누수 검사를 포함해 실행하지 못한 검사는 이유와 함께 SKIP으로 기록하며 통과로 계산하지 않는다.

#### 3. 보너스 플래그 2개 — -r, -p

현재 main의 옵션 해석과 sort_symbols에 두 옵션의 기본 코드가 있다. 구현 존재와 동작 검증 완료를 구분한다.

- [ ] -r: 기본 이름 정렬의 역순으로 심볼 레코드 전체를 정렬하고 시스템 nm -r과 비교한다.
- [ ] -p: 정렬하지 않고 원래 심볼 테이블 순서로 출력하며 시스템 nm -p와 비교한다.
- [ ] ELF32/ELF64와 여러 파일에서 각 옵션의 출력·필터·값 연결을 확인한다.
- [x] -r -p, -p -r 동시 지정 시 -p 우선임을 확인한다. ELF32/64 .o·정적 실행 파일에서는 완전 일치한다. PIE·.so·일반 C 실행 파일에는 플래그와 별개인 분류 오류가 남는다.
- [ ] 잘못된 옵션, 옵션 뒤 파일 인자 처리와 두 옵션의 구두 설명을 확인한다.

#### 4. 학교 공식 바이너리 테스트 — tests/eval_test

**tests/eval_test는 사용자가 확인한 정식 학교 바이너리 테스트 파일 모음이다.** 자체 제작한 tests/day1~day4와 함께 통합 검증에 사용한다. 폴더의 .c 자료와 바이너리 입력을 구분하고, 공식 입력 원본은 보존한다.

- [x] tests/eval_test의 입력 목록과 file 판독·nm 수용 여부를 확인한다. 바이너리 8개와 C 소스 2개를 구분했으며 아래에 기록했다.
- [x] 각 바이너리를 시스템 nm과 ft_nm에 전달해 stdout·stderr·종료 상태를 비교한다. 기본/-r/-p 총 24건: 완전 일치 0건, 종료 상태 일치 24건.
- [x] header, header_copy, header_and_prog, header_and_prog_copy 및 error_header, header_offset_error, unterminated_string, wrong_arch의 결과를 기록한다. 이름 대신 실제 nm 결과를 기준으로 삼았다.
- [x] 공식 입력 모두에 프로세스별 10초 timeout과 별도 ASan을 적용했다. 해당 8개에서 충돌·timeout·ASan 오류는 없었다(누수 검사 제외).
- [x] nm이 수용한 공식 입력 2개에 기본/-r/-p 비교를 적용한다. 심볼 분류 차이로 6건 모두 출력 불일치다.
- [ ] 발견한 실패 원인을 수정한 뒤 공식 입력과 기존 Day1~4 회귀 검사를 재실행한다.

#### 5. 제출 상태 재현과 설명 — 기존 Day7

- [x] 같은 환경의 새 디렉터리에 Makefile·src·inc만 복사해 빌드한다. 빌드 성공; 별도 OS/학교 환경의 재현은 미검증이다.
- [x] 새 디렉터리에서 빌드 → 재실행(재링크 없음) → clean → 빌드 → fclean → 빌드를 확인한다. 일반 빌드는 libft 없이 성공하며 Day1~3 테스트의 불필요한 libft 소스 의존성도 후속 정리에서 제거했다.
- [ ] 필수·보너스·학교 공식 바이너리·정상·오류 회귀 결과를 최종 정리한다.
- [ ] fd와 매핑 수명, 검증 위치, 문자열 연결과 심볼 분류 근거를 설명한다.
- [ ] ELF32/64 차이, COMMON 크기 출력, 정렬·플래그 우선순위, 종료 상태와 남은 제한을 설명한다.

완료 기준: 필수 경로와 보너스 두 옵션을 입력부터 정리까지 검증하고, 학교 공식 바이너리에서 발견한 결함을 수정한다. 빌드·회귀 결과를 재현하고 처리 단계를 직접 설명할 수 있어야 한다. 환경 제약으로 미실행한 검사는 별도로 남긴다.


#### 2026-10-27 통합 검증 결과

환경: GCC 16.2.1, GNU nm 2.46.1, LC_ALL=C. 현재 소스를 `/usr/bin/gcc -Wall -Wextra -Werror -Iinc`로 새 CLI에 직접 빌드해 비교했다. 각 프로세스 timeout은 10초다. PASS는 stdout 바이트·stderr 바이트·종료 상태가 모두 같은 경우만 계산한다. 도구명·진단 문구를 정규화하지 않았으므로 오류 사례의 FAIL은 곧 충돌이나 입력 거부 실패를 뜻하지 않는다.

| 비교 묶음 | 건수 | PASS | FAIL | 확인 내용 |
|---|---:|---:|---:|---|
| 정상 형식 × 기본/-r/-p/-r -p/-p -r | 50 | 20 | 30 | ELF32/64 각각 .o·정적 실행 파일·PIE·.so 8개 + 공식 C 소스에서 빌드한 실행 파일 2개 |
| ELF32 .o + ELF64 .o 복수 인자 × 같은 옵션 5종 | 5 | 0 | 5 | 심볼은 출력하지만 파일명 헤더와 구분 빈 줄 누락 |
| 기본 a.out·파일 뒤 -r·미지원 -z·결합 -rp | 4 | 1 | 3 | 기본 a.out만 완전 일치; -z는 양쪽 모두 종료 1이나 진단이 다름 |
| 오류·stripped·혼합 입력 | 9 | 0 | 9 | 없는 파일·빈 파일·텍스트·80바이트로 자른 .o·권한 000 파일·stripped·디렉터리·혼합 2종 |
| 공식 바이너리 8개 × 기본/-r/-p | 24 | 0 | 24 | 종료 상태 24건 일치; stdout은 3건, stderr는 6건 일치 |
| **합계** | **92** | **21** | **71** | **검사 실행 완료와 기능 완료를 구분한다** |

정상 형식 50건 중 ELF32/64 .o와 정적 실행 파일의 20건은 완전 일치했다. PIE·.so·C 실행 파일 30건은 종료 상태 0과 빈 stderr가 일치하지만 심볼 문자가 다르다. 이번 어셈블리 fixture는 local/global·weak·COMMON을 포함하고, Day4 fixture는 undefined·TLS 등 추가 사례를 검사한다. 동일 이름 심볼 정렬은 이번 통합 fixture에 포함하지 않았다.

공식 입력의 기본 모드 결과(각각 -r/-p에서도 같은 수용·거부 상태):

| 입력 | file 판독 | nm / ft_nm 종료 상태 | 결과 |
|---|---|---|---|
| error_header, header_offset_error | ELF64 실행 파일 | 0 / 0 | 이름과 달리 nm이 수용함. `_DYNAMIC`·init/fini array 심볼 등이 d 대신 ?로 출력됨 |
| header, header_copy | ELF64 공유 객체로 식별 | 1 / 1 | 양쪽 거부; nm의 BFD plugin stdout과 stderr 진단이 달라 완전 일치 아님 |
| header_and_prog, header_and_prog_copy | ELF64, program/section header 잘림 진단 | 1 / 1 | 양쪽 거부; plugin 출력·진단 차이 |
| unterminated_string | ELF64, section header 누락 진단 | 1 / 1 | 양쪽 거부; plugin 출력·진단 차이. 이름만으로 문자열 검증 경로를 통과했다고 판단하지 않음 |
| wrong_arch | ELF32이나 machine은 x86-64로 식별 | 1 / 1 | stdout은 모두 비어 있음; stderr 진단 차이 |

확인한 수정 필요 사항:

1. `symbol_type.c`는 일반 섹션 분류를 주로 SHT_PROGBITS/SHT_NOBITS로 제한한다. SHT_DYNAMIC의 `_DYNAMIC`, SHT_INIT_ARRAY/SHT_FINI_ARRAY의 심볼, SHT_NOTE의 `__abi_tag`가 ?로 남는다. 이번 입력의 nm 기준 기대값은 각각 d/d/r이다.
2. 복수 입력에 `파일명:` 헤더와 빈 줄이 없다. 오류 뒤 다음 정상 파일 처리 및 최종 상태 1은 동작한다.
3. 파일 뒤 -r을 옵션 대신 파일명으로 처리하고, -rp를 거부한다. -r/-p 개별 지정과 동시 지정의 기본 정렬 동작은 위 정상 .o·정적 실행 파일에서 확인했다.
4. 오류 진단에 대상 파일명이 빠지고 nm과 문구가 다르다. stripped 파일은 출력 없이 종료 0이나 nm의 `no symbols` 진단이 없다. 손상 80바이트 .o는 nm이 plugin 경고 및 no symbols와 함께 종료 0, ft_nm은 종료 1이다. 이 차이를 근거로 손상 입력을 무조건 수용하도록 수정해서는 안 된다.
5. 기존 MAP_FAILED 정리 방어·일반 섹션 payload·SHN_XINDEX 과제는 그대로 남는다. 이번 ASan 무보고가 이 구현 결함의 해결을 뜻하지 않는다.

메모리·회귀·빌드 결과:

- 전체 CLI를 `-g -O1 -fsanitize=address -fno-omit-frame-pointer`로 새로 빌드했다. 정상 형식 10개 + 오류/stripped 7개 + 공식 8개, 총 25개를 각각 실행했다. `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`에서 ASan 오류 0, signal 종료 0, timeout 0이다. 기대된 입력 거부 종료 1은 메모리 오류로 계산하지 않았다.
- 누수 검사 **SKIP**: detect_leaks=1로 먼저 실행한 25건 모두 LeakSanitizer가 ptrace 환경에서 동작할 수 없다는 fatal error로 중단됐다. 누수 없음으로 해석하지 않는다. UBSan도 런타임 부재로 **SKIP**이다. fstat/mmap 강제 실패·fd/매핑 누수·Valgrind는 미실행이다.
- 최초 `make test-1 CC=/usr/bin/gcc`: libft/ft_strlcpy.c·ft_strlen.c 부재로 빌드 실패, 런타임 미실행.
- 최초 `make test-2 CC=/usr/bin/gcc`: libubsan.so.1.0.0 부재로 링크 실패, 런타임 미실행.
- 최초 `make test-3 CC=/usr/bin/gcc`: libft/ft_memcpy.c·ft_strlen.c 부재로 빌드 실패, 런타임 미실행. 이 최초 실행에서는 Day1~3의 과거 PASS 기록을 재확인하지 못했으며, 아래 후속 정리에서 ASan으로 재검증했다. test-1-32는 이번에 실행하지 않았다.
- `DAY4_SANITIZERS=address ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make test-4 CC=/usr/bin/gcc DAY4_NM="$PWD/ft_nm"`: **52 PASS / 0 FAIL / 0 UNAVAILABLE**.
- Makefile·src·inc만 복사한 새 디렉터리에서 일반 빌드, 불필요한 재빌드 없음, clean/fclean 후 재빌드를 확인했다. 별도 환경 재현과 구두 설명은 미완료다.

#### 후속 정리 — libft 의존성 제거와 sanitizer 실행 설정

현재 소스는 libc의 memcpy·strlen·strlcpy를 사용한다. tests/day1~day3에 남아 있던 libft/*.c 링크 항목을 제거했고, 누락된 라이브러리를 새로 만들거나 대체 함수를 추가하지 않았다. 일반 CLI 소스와 Makefile은 변경하지 않았다. libc에 strlcpy가 없는 별도 환경의 이식성은 별도 과제다.

Day1~3에 공통 sanitizer 설정을 추가했다. 기본값 address,undefined는 유지하며 `NM_TEST_SANITIZERS` 또는 일차별 `DAY1_SANITIZERS`~`DAY3_SANITIZERS`로 address/undefined/address,undefined/none을 선택한다. ASAN_OPTIONS·UBSAN_OPTIONS는 호출자의 설정을 보존하며 제외된 검사는 SKIP으로 출력한다. Day4는 기존 DAY4_SANITIZERS를 사용한다.

UBSan 원인: GCC의 `/usr/lib/gcc/x86_64-redhat-linux/16/libubsan.so`는 `INPUT ( /usr/lib64/libubsan.so.1.0.0 )`인 링커 스크립트인데 대상 런타임 파일이 없다. 정적 libubsan.a와 Clang의 standalone UBSan 런타임도 찾을 수 없다. 런타임 설치 완료로 처리하지 않으며, 현재 환경에서는 명시적인 ASan 전용 실행으로 테스트 빌드 차단을 해소했다. UBSan 전체 검증은 맞는 런타임을 갖춘 환경에서 다시 수행해야 한다.

```bash
NM_TEST_SANITIZERS=address ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
    make test-1 test-2 test-3 CC=/usr/bin/gcc
```

| 후속 실행 | 결과 | 검사 범위 |
|---|---|---|
| Day1 | 8 PASS / 0 FAIL / 1 N/A | ASan; native ABI에서 SIZE_MAX+1 표현 불가 |
| Day2 | 61 PASS / 0 FAIL | ASan; ELF32/64 헤더·테이블 검사 |
| Day3 | 46 PASS / 0 FAIL / 0 UNAVAILABLE | ASan; 실제 ELF32/64 .o 비교 포함 |
| test-1-32 | 환경 오류, 런타임 미실행 | gnu/stubs-32.h 누락으로 32비트 libc probe 빌드 실패 |

UBSan·누수 검사는 위 실행에서도 SKIP이다. Bash 구문 검사와 sanitizer 선택 none/undefined/잘못된 값 처리도 확인했다. 이 의존성 정리 시점에는 CLI 구현을 변경하지 않아 통합 71 FAIL을 유지했다. 이후 분류 수정 결과는 아래 별도 기록을 따른다. 수정 우선순위는 **분류 범위 확대 → 복수 파일 출력 헤더 → 옵션 처리 → 파일별 오류/no symbols 진단**이다. 진단의 도구명과 BFD plugin 출력 차이는 심볼 값·문자 불일치와 구분해서 판단한다.

임시 실행 스크립트·fixture·개별 stdout/stderr/종료 상태는 `/tmp/nm-day5-20261027/`에 보관했다. `run.py`는 누수 검사를 시도한 기록, `run-noleak.py`와 `results-noleak.json`은 최종 비교 및 누수 제외 ASan 기록이다. 임시 파일은 제출 파일에 포함하지 않았으며 장기 보존을 보장하지 않는다. 권한 오류 fixture가 mode 000이므로 스크립트 재실행 전 `chmod 600 /tmp/nm-day5-20261027/denied`가 필요하다.


#### 후속 수정 — 할당된 섹션의 심볼 분류

`src/symbol/symbol_type.c`에서 PROGBITS/NOBITS에 국한된 분류 조건을 제거했다. SHF_ALLOC이 있는 일반 섹션은 실행 속성 → NOBITS → 쓰기 속성 → 읽기 전용 순으로 T/B/D/R을 선택하고 local binding은 소문자로 출력한다. 이로써 `_DYNAMIC`, init/fini array 심볼은 d, `__abi_tag`는 r이 된다. weak·undefined·absolute·common의 기존 선행 분류는 유지한다.

Day4에 DYNAMIC·INIT_ARRAY·FINI_ARRAY·PREINIT_ARRAY·NOTE의 local/global 사례, 비할당 NOTE의 기존 ? 유지, writable text의 실행 속성 우선 검사를 추가했다. 실제 ELF32/64 어셈블리 fixture에도 배열·NOTE 심볼을 추가해 시스템 nm과 비교했다. 확장 테스트는 수정 전 54 PASS / 22 FAIL, 수정 후 **76 PASS / 0 FAIL / 0 UNAVAILABLE**다(분류 74개 + 실제 .o 2개, ASan 사용·누수 제외).

기존 92건 통합 비교를 같은 임시 스크립트로 재실행했다:

| 비교 묶음 | 수정 전 PASS / FAIL | 수정 후 PASS / FAIL |
|---|---|---|
| 정상 형식·플래그 50건 | 20 / 30 | **50 / 0** |
| 복수 파일 5건 | 0 / 5 | 0 / 5 |
| 기본 인자·옵션 4건 | 1 / 3 | 1 / 3 |
| 오류·stripped·혼합 9건 | 0 / 9 | 0 / 9 |
| 공식 바이너리 24건 | 0 / 24 | **6 / 18** |
| **합계 92건** | **21 / 71** | **57 / 35** |

공식 error_header·header_offset_error의 기본/-r/-p 6건은 stdout·stderr·종료 상태가 모두 일치한다. 나머지 공식 입력 18건은 양쪽 모두 종료 1이지만 진단·BFD plugin 출력 차이로 FAIL이다. 전체 CLI ASan 25건에서도 sanitizer 오류·signal 종료·timeout은 각각 0이다. UBSan·누수 검사는 계속 SKIP이다.

분류 수정 전 상세 표와 실패 목록은 당시 기록으로 유지한다. 최신 개별 결과는 `/tmp/nm-day5-20261027/results-noleak.json`, 수정 전 결과는 `results-before-classification.json`에 있다. 이번 수정은 위 입력에서 확인한 분류 차이를 해결한 것이며 GNU unique·IFUNC 등 모든 심볼 종류를 검증한 것은 아니다. 다음 순서는 복수 파일 출력 헤더다. 옵션·오류 진단은 이번 수정에서 변경하지 않았다.

## 다음 단계에 필요한 공부

| 순서 | 개념 | 설명할 수 있어야 할 내용 |
|---|---|---|
| 1 | Section header와 section payload | Shdr는 목록의 한 항목이고, sh_offset/sh_size가 가리키는 데이터는 별도라는 점 |
| 2 | SHT_SYMTAB과 SHT_STRTAB | 심볼 레코드 목록과 이름 바이트 목록의 역할 차이 |
| 3 | sh_link | 파일 offset이 아니라 연결된 섹션의 index라는 점 |
| 4 | sh_entsize | 심볼 하나의 크기이며, 0·예상 크기 불일치·나머지를 검사하는 이유 |
| 5 | Elf32_Sym/Elf64_Sym | st_name, st_value, st_info, st_shndx의 의미와 두 클래스의 배치 차이 |
| 6 | st_name | 연결된 문자열 테이블 내부 offset이며, 범위와 NUL 종료를 모두 검사하는 이유 |
| 7 | st_info와 section index | binding/type 추출, SHN_UNDEF·SHN_ABS·SHN_COMMON·SHN_XINDEX의 구분 |
| 8 | 메모리 수명 | 심볼 이름이 매핑 내부를 참조하면 출력이 끝나기 전에 munmap하면 안 된다는 점 |

작은 실제 `.o`를 `readelf -h`, `readelf -S`, `readelf -s`로 관찰하고 자신의 reader 결과와 비교한다. 한 심볼을 골라 SYMTAB 섹션 → sh_link → 문자열 테이블 → st_name → 이름 순서를 직접 추적한다. 첫 연습에서는 정렬과 출력 문자를 추가하지 않는다.

학습 근거는 시스템 `<elf.h>`, 로컬 `man 5 elf`, ELF 명세의 section/symbol/string table 설명을 사용한다. PDF는 구현 범위를, ELF 명세는 필드 의미를 확인하는 자료다.

## 검증 기록

- 1일 차: 기본 테스트 8 PASS / 0 FAIL / 1 N/A, 사용자 32비트 변환 테스트 3 PASS.
- 2일 차: 앞선 에이전트 실행 `make test-2` 61 PASS / 0 FAIL, ASan·UBSan 활성화.
- 3일 차: 앞선 에이전트 실행 `make test-3` 46 PASS / 0 FAIL / 0 UNAVAILABLE, ASan·UBSan 활성화. 실제 .o는 ELF32·ELF64 각각 11개 레코드 일치.
- 4일 차: 2026-10-27 빌드 성공 및 Day4 재실행 52 PASS / 0 FAIL / 0 UNAVAILABLE. 분류 50개는 ASan 사용, UBSan·누수 검사 제외. 실제 .o 출력 비교 2개는 일반 CLI 빌드 사용.
- 통합 5일 차: 92건 비교 중 21 PASS / 71 FAIL. 공식 입력 24건은 종료 상태가 모두 일치하나 출력·진단 차이가 남는다. 전체 CLI ASan 25건 오류·충돌·timeout 없음(누수 제외).
- 후속 정리에서 불필요한 libft 링크 항목을 제거하고 Day1 8 PASS / 1 N/A, Day2 61 PASS, Day3 46 PASS를 ASan으로 재확인했다. Day4는 앞선 동일 날짜 실행에서 52 PASS다. 누수 검사·UBSan은 SKIP, test-1-32는 32비트 libc 헤더 부재로 미실행이다. 5일 차는 결함 수정과 잔여 검증 전까지 미완료다.
- 후속 분류 수정: symbol_type.c와 Day4 테스트·문서를 갱신했다. 최신 통합 비교는 57 PASS / 35 FAIL, Day4 확장 회귀는 76 PASS다.
