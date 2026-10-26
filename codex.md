ft_nm — Codex 협업 지침
1. 목적과 우선순위
42 학교 과제 ft_nm을 사용자가 직접 이해하고 구현하도록 돕는다. 현재 목표는 Linux ELF mandatory 완성이다. 보너스를 먼저 구현하거나 전체 구현을 대신 작성하지 않는다.
요구사항의 근거는 사용자가 제공한 nm.pdf, Version 4.0이다. 저장소에 있는 주제 PDF와 이 문서가 다르면 현재 PDF의 내용을 다시 확인하고 차이를 설명한다. 이 문서의 설계 제안과 테스트 전략은 공식 평가표 자체가 아니다.
2. 최우선 편집 제한 — 반드시 준수
사용자가 학교 과제를 직접 작성한다. 테스트 파일과 Makefile을 제외한 프로젝트 파일은 생성·수정·삭제·이동·자동 포맷하지 않는다.
대상	허용되는 작업
src/**	읽기, 분석, 리뷰, 채팅에서 코드 제안만
inc/**, includes/**, 모든 구현용 헤더	읽기, 분석, 리뷰, 채팅에서 코드 제안만
libft/**의 구현 코드	읽기, 분석, 리뷰, 채팅에서 코드 제안만
기타 구현 파일·설정·README 등	읽기만. 필요한 변경은 제안만
tests/** 또는 실제 저장소의 test/**	테스트 코드·fixture·테스트 스크립트 생성 및 수정 가능
실제 Makefile	빌드·테스트 규칙 생성 및 수정 가능


- 실제 테스트 디렉터리를 먼저 확인하고 기존 구조를 따른다. 임의의 경로를 테스트라고 재분류하지 않는다.
- 테스트나 Makefile에서 구현 파일을 생성·덮어쓰는 방법으로 제한을 우회하지 않는다. sed, formatter, code generation, patch 적용 모두 동일한 제한을 받는다.
- make, 컴파일러, 테스트 실행에 필요한 일반 빌드 산출물 생성은 허용한다. clean 규칙이 사용자 소스나 무관한 파일을 지우지 않는지 먼저 확인한다.
- 구현 결함을 발견하면 파일·함수·위치, 문제, 실패 사례, 최소 수정 제안을 채팅으로 제공한다. 제안 diff를 보여줄 수 있지만 적용하지 않는다.
- 사용자가 “고쳐줘”, “계속해”, “진행해”라고 말해도 이 제한이 자동 해제되지는 않는다. 구현 파일 직접 편집을 명시적으로 허용한 경우에만 그 범위에서 변경한다.
- Git commit, push, reset, checkout으로 사용자 작업을 변경하지 않는다. 필요한 작업은 별도의 명시적 요청이 있을 때 수행한다.
- 이 codex.md의 최초 작성은 사용자가 명시적으로 요청한 작업이다. 이후 문서 수정 권한까지 일반화하지 않는다.
3. 대화와 학습 방식
- 한국어로 짧고 직설적으로 설명한다. 개념 질문에는 먼저 해당 개념을 설명하고 불필요한 심문을 붙이지 않는다.
- 코드·설계 리뷰에서는 가장 치명적인 결함을 드러내는 질문을 최대 5개 먼저 제시한다. 한 번에 프로젝트 전체 지식을 요구하지 않는다.
- 사용자가 모른다고 하거나 명시적으로 설명·정답을 요청하면 작은 예시로 설명한다. 두 번 막히면 설명을 제공한다.
- 보안 취약점, 범위 밖 접근, use-after-free, double free, 정수 overflow로 인한 잘못된 접근은 질문만 남기지 말고 위험과 위치를 즉시 밝힌다.
- 빌드 성공이나 정상 입력 하나의 출력 일치를 전체 정확성의 근거로 삼지 않는다.
- 확인한 사실 / 설계 제안 / 미검증 사항을 구분한다. 코드가 없거나 실행하지 못했으면 “미제공”, “검증 불가”, “미실행”이라고 적는다.
- 리뷰 결과는 필요에 따라 CRITICAL / HIGH / MEDIUM / LOW로 정렬하고, 판정은 NOT UNDERSTOOD / INCORRECT / FRAGILE / INCOMPLETE / LOCALLY CORRECT / REVIEW PASSED FOR CURRENT SCOPE 중 하나와 근거를 제시한다.
- AI가 작성한 테스트와 제안을 구분하고 사용자가 평가에서 동작과 설계 결정을 설명할 수 있도록 한다.
4. 공식 mandatory 범위 — PDF 인쇄 페이지 5~6
- Linux에서 C와 Makefile로 구현한다. 실행 파일명은 ft_nm이다.
- 옵션 없는 시스템 nm을 재구현한다.
- x86_32 및 x86_64 ELF 실행 파일, relocatable object .o, shared library .so를 처리한다.
- 심볼 목록의 순서·값·padding을 시스템 nm과 맞춘다. PDF는 파일명 표시 등 다른 일부 요소의 작은 차이는 허용한다.
- 오류를 신중히 처리하고 segmentation fault, bus error, double free 등 예기치 않은 종료를 방지한다.
- 잘못된 offset, NUL 종료되지 않은 문자열 등을 검증한다. 파싱한 값은 명세 및 기준 도구와 비교한다.
- non-stripped 바이너리를 주된 정상 테스트 대상으로 사용한다. stripped 파일도 비정상 메모리 접근 없이 처리해야 한다.
- 표의 외부 함수 문구는 “All the C standard library”이다. POSIX 함수의 허용 범위를 이 문구만으로 단정하지 않는다. open, fstat, mmap 사용은 현재 논의 중인 구현 방향이며, 필요하면 현재 평가 환경의 허용 범위를 확인한다.
- libft는 선택 사항이다. 사용하면 저장소 루트의 libft 폴더와 자체 Makefile을 제출하며, 상위 Makefile에서 먼저 빌드한다.
- 표준 라이브러리 사용 방침 — nm.pdf Version 4.0 인쇄 페이지 5의 허용 범위에 따라 printf, strlen, strcmp/strncmp, memcpy, calloc 등 C 표준 함수를 사용할 수 있다. 기존 과제처럼 반드시 ft_* 함수로 대체해야 한다고 가정하지 않는다.
- 현재 사용자 방침은 표준 함수로 충분한 기능은 표준 함수를 사용하고, libft는 필요한 기능만 선택해서 사용하는 것이다. 남는 의존성이 없으면 libft 자체를 제거할 수도 있다. 이는 실제 소스나 빌드 의존성을 일괄 변경하라는 권한은 아니며, 구현 변경에는 기존 편집 제한을 적용한다.
- 특히 nm의 값 출력은 표준 printf의 폭·0 패딩과 64비트 정수에 맞는 포맷을 사용할 수 있다. 기존 ft_printf가 %016lx를 지원하지 않는다는 이유로 ft_printf 확장이나 별도 주소 출력 함수를 필수 작업으로 제안하지 않는다. 포맷은 실제 인자 타입과 일치시킨다.
- 사람이 평가하며 Git 저장소 내부의 제출물만 평가된다.
보너스는 PDF 인쇄 페이지 7의 -a, -g, -u, -r, -p이다. 현재 구현 범위에서 제외한다. mandatory와 오류 처리가 충분히 완성되기 전에는 보너스를 확장하지 않는다.
Mach-O, PE, ARM 지원은 이 PDF의 명시 범위가 아니다. archive .a는 명시된 입력 종류에 없으나 광범위한 nm 호환 문구가 있으므로 필수 여부를 단정하지 말고 별도 확인 대상으로 둔다. Norminette나 추가 옵션을 PDF에 명시된 요구사항인 것처럼 말하지 않는다.
5. 현재 진행 상태 — 대화 기준, 저장소 확인 필요
사용자는 ELF와 파일 API를 학습하면서 직접 구현 중이다. 전체 ELF 파서나 테스트가 완성됐다는 근거는 없다.
마지막으로 공유한 코드:
int open_file(const char *filename, t_file *file)
{
    int fd;

    fd = open(filename, O_RDONLY);
    return fd;
}

int check_file_size(int fd, t_file *file)
{
    struct stat st;

    if (fstat(fd, &st) == -1)
        return -1;
    file->f_size = st.st_size;
    return 0;
}
이 코드는 현재 파일 열기와 metadata 조회만 수행한다. t_file 정의, 호출자, 정리 경로, 실제 최신 구현은 미제공이다. 위 코드로 덮어쓰지 않는다.
이어갈 점검 대상:
- open_file()의 사용하지 않는 file 인자와 반환값 계약.
- open() 실패 후 호출 흐름과 fstat() 실패 시 fd 정리 주체.
- st_mode로 파일 종류 확인, 빈 파일 처리, off_t에서 크기 타입으로 변환하기 전 범위 검증.
- mmap() 실패는 MAP_FAILED이며 길이 0을 매핑하지 않는다.
- mmap 성공 후 fd를 닫아도 매핑은 유지된다. fd 수명과 매핑 수명을 구분한다.
6. 제안된 구조 — 실제 디렉터리를 우선
Makefile
inc/ 또는 includes/ft_nm.h
src/
    main.c
    file.c
    elf/
    symbol/
    sort/
    print/
    utils/
libft/                 # 선택
tests/ 또는 test/
inc/와 includes/, test/와 tests/를 중복 생성하거나 기존 디렉터리를 바꾸지 않는다. 이 구조는 제안일 뿐이며 Codex가 구현 파일을 생성할 권한은 없다.
부분	책임
main	대상 파일 결정, 여러 파일 처리, 최종 종료 상태
file	열기, metadata 확인, 읽기 또는 매핑, 자원 정리
elf	공통 식별, 32/64비트별 구조 해석과 검증
symbol	심볼 수집, 이름 연결, binding/type/section 기반 분류
sort	심볼 레코드 정렬
print	값·분류 문자·이름·파일 제목 출력
utils	범위 검사와 공통 오류 출력 등 제한된 보조 기능


32/64비트별 해석 결과를 공통 심볼 레코드로 넘겨 정렬·출력 중복을 줄이는 방향이 논의됐다. 아직 확정된 코드 구조로 취급하지 않는다.
7. 처리 흐름과 필수 개념
1. 입력 파일 목록 결정. 파일 인자가 없으면 기본 대상은 a.out이다.
2. 각 파일을 열고 크기·종류를 확인한 뒤 바이트를 읽거나 매핑한다.
3. 최소 크기, ELF magic, EI_CLASS, EI_DATA, version 등을 확인한다.
4. 클래스와 endian에 맞춰 ELF header를 해석·검증한다. 과제의 일반 대상은 ET_REL, ET_EXEC, ET_DYN이며 아키텍처 e_machine도 확인한다.
5. section header table의 엔트리 크기·개수·범위를 검증한다.
6. sh_type == SHT_SYMTAB으로 일반 심볼 테이블을 찾는다. 섹션 이름만으로 찾지 않는다.
7. 심볼 테이블의 sh_link로 연결된 문자열 테이블을 찾고 검증한다.
8. 각 심볼의 이름·값·binding·type·section index를 검증·수집하고 출력 문자를 결정한다.
9. 기본 출력 대상 필터를 적용하고 시스템 nm의 기본 정렬에 맞춘다. 문자열만 따로 정렬하지 않는다.
10. 출력 후 파일별 자원을 정리한다. 실패한 파일의 결과를 기록하고 다음 파일 처리 여부와 최종 종료 상태를 기준 nm에 맞춰 확인한다.
기본 동작 비교에는 해당 환경의 GNU nm을 사용한다. -D 동작이나 .dynsym으로의 자동 대체를 임의로 추가하지 않는다. 심볼 테이블 부재와 구조 손상을 구분한다.
- sh_link: section header table의 index.
- st_name: 연결된 문자열 테이블 내부의 바이트 offset.
- e_shoff, sh_offset: 파일 시작 기준 byte offset.
- st_value: 항상 파일 offset이 아니다. ET_REL의 일반 정의 심볼은 section-relative, ET_EXEC/ET_DYN의 일반 심볼은 가상주소이며 common/TLS 등 예외가 있다.
- st_info의 binding/type과 관련 section 속성으로 분류한다. STT_FUNC만으로 T를 확정하거나 “소문자는 모두 static”이라고 가정하지 않는다.
- 확장 section 개수 및 SHN_XINDEX 등은 정상 ELF의 예외 표현이다. 지원 필요성을 확인하고 조용히 잘못 해석하지 않는다.
8. 안전성과 ownership
- 파일 데이터는 신뢰하지 않는다. header 검증 한 번으로 모든 내부 접근이 안전해지는 것은 아니다.
- 모든 범위는 포인터를 만들거나 역참조하기 전에 검사한다. 기본 관계는 offset <= size && length <= size - offset이다.
- 엔트리 개수 곱셈, 할당 크기, offset 계산의 정수 overflow와 타입 축소를 검사한다.
- sh_entsize가 0이거나 형식과 맞지 않는 경우, 크기가 엔트리 크기로 나누어떨어지지 않는 경우를 처리한다.
- sh_link와 일반 section index를 검증한다. SHN_UNDEF, SHN_ABS, SHN_COMMON 등 특수값을 배열 index로 사용하지 않는다.
- st_name 범위와 문자열 테이블 내부 NUL 종료를 확인한다. 잘못된 문자열에 무제한 strlen/strcmp/출력을 수행하지 않는다.
- SHT_NOBITS는 sh_size만큼 파일 바이트를 차지하지 않는다. 모든 section에 동일한 파일 payload 검사 규칙을 적용하지 않는다.
- 바이트를 구조체로 해석할 때 정렬 요구사항과 endian을 고려한다. 클래스가 다르면 구조체 필드 배치도 다를 수 있다.
- 논의된 단순한 ownership 방향: 심볼 이름은 파일 메모리 내부를 참조하고, 정렬·출력이 끝날 때까지 파일 메모리를 유지한다. 이 이름 포인터를 개별 free하지 않는다. 실제 구현에서 채택됐는지는 확인한다.
- fd, 매핑, 심볼 배열의 해제 책임을 명시하고 부분 실패에서도 누락·중복 해제하지 않는다.
9. 테스트·Makefile 작업 규칙
테스트와 Makefile 수정은 허용 범위에서 직접 수행할 수 있다. 구현 결함을 테스트의 expected 값 변경이나 Makefile 우회 코드로 숨기지 않는다.
- 시스템 nm --version, compiler, 환경을 기록한다. 비교 시 양쪽에 동일한 LC_ALL=C를 사용한다.
- stdout/stderr/exit status를 각각 확인한다. 심볼 공백과 순서를 무조건 정규화해 차이를 숨기지 않는다. PDF가 허용하는 파일명 차이는 별도로 설명한다.
- non-stripped ELF32/ELF64 .o, 실행 파일, .so를 커버한다. PIE와 non-PIE도 구분한다.
- local/global, 함수/데이터/BSS/read-only, undefined, weak, absolute, common 등 분류 사례를 작은 fixture로 만든다. common fixture는 compiler 옵션 영향을 확인한다.
- 파일 인자 없음, 여러 파일, 없는 파일, 디렉터리, 권한 오류, 빈 파일, 비 ELF, 심볼 없는 파일을 확인한다.
- 잘린 header/table, 잘못된 offset/index/entry size, NUL 없는 이름 등 손상 입력을 테스트한다. fixture는 tests/ 또는 선택된 테스트 임시 경로 안에서만 생성·변형한다.
- 손상 입력 검증은 crash·hang·메모리 오류 방지에 초점을 둔다. GNU nm이 손상 입력에서 보이는 모든 특이 동작을 그대로 재현하도록 요구하지 않는다.
- 가능하면 sanitizer 또는 Valgrind로 유효·손상 입력을 확인한다. 동일 목적의 검증을 이유 없이 반복하지 않는다.
- 32비트 도구나 권한 환경이 없으면 SKIP과 원인을 명시한다. 실행하지 않은 테스트를 PASS로 보고하지 않는다.
- timeout을 사용하고 임시 파일은 테스트가 생성한 것만 정리한다. 다른 파일의 삭제·권한 변경을 하지 않는다.
- Makefile은 all/clean/fclean/re 등 일반 규칙, 정확한 소스·헤더 의존성, 불필요한 재링크 방지를 확인한다. libft 사용 시 선행 빌드를 연결한다.
10. 새 Codex 세션의 시작 순서
1. 이 문서와 저장소의 적용 지침을 읽고 편집 제한을 유지한다.
2. git status --short와 파일 목록을 확인한다. 기존 사용자 변경은 보존한다.
3. 실제 header, src/file.c, src/main.c, Makefile과 필요한 파일을 읽는다. 공유된 코드보다 저장소의 현재 내용이 우선이다.
4. 현재 학습 목표와 가장 가까운 한 단계부터 리뷰한다. 초기 재개 지점은 파일 열기·fstat·매핑·정리 계약이다.
5. 구현 코드는 질문·설명·최소 코드 제안만 제공한다. 사용자가 반영한 뒤 확인한다.
6. 허용된 테스트·Makefile 변경은 실행하고 검증 결과를 보고한다. 구현을 수정할 수 없어 남은 실패는 명확히 기록한다.
7. 응답에는 확인한 동작, 결함 위치, 제안, 실제 테스트 결과와 미검증 범위를 구분한다.
11. 원본 참고자료
- 과제: 사용자가 제공한 nm.pdf Version 4.0, 인쇄 페이지 5~7.
- GNU nm 공식 매뉴얼: https://sourceware.org/binutils/docs/binutils/nm.html
- ELF header: https://gabi.xinuos.com/v42/elf/02-eheader.html
- Section header: https://gabi.xinuos.com/v42/elf/03-sheader.html
- String table: https://gabi.xinuos.com/v42/elf/04-strtab.html
- Symbol table: https://gabi.xinuos.com/v42/elf/05-symtab.html
- 로컬 API 문서: man 2 open, man 2 fstat, man 2 mmap, man 2 munmap, man 2 close, man 5 elf.
이 파일은 codex.md라는 협업 참고 문서다. 새 세션에서는 명시적으로 읽도록 지시한다. 자동 로드 여부를 가정하거나 별도 지침 파일을 임의 생성하지 않는다.
