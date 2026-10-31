# Linux ELF mandatory 평가 자동화

AI가 작성한 테스트다. 구현 코드는 변경하지 않는다. 기존 `tests/eval_test`
fixture를 읽기 전용으로 재사용하며 기존 `eval_test`, `days_test`,
`lifetime_test`와 독립적으로 실행한다. 기존 두 runner는 보너스 `-r`, `-p`도
실행하므로 mandatory 결과를 얻기 위해 이들을 대신 호출하지 않는다.

```sh
make eval-auto
# 동일한 직접 실행
python3 eval-auto/run.py
```

Python 3, GNU make, x86 Linux의 C compiler, GNU nm/ld/strip, file, Valgrind, ASan·UBSan·LeakSanitizer runtime이 필요하다.
`CC`, `NM`으로 도구를 지정할 수 있다. 두 도구에 동일하게 `LC_ALL=C`를 적용한다.
32비트 C 실행 파일은 multilib libc가 필요하다. ELF32 `.o`/`.so`/freestanding
실행 파일은 `cc -m32 -c`와 `ld -m elf_i386`로 별도로 생성하므로 libc가 없어도
검증할 수 있다. 생성된 실행 파일을 실행하지 않고 두 nm의 입력으로 사용한다.

매 실행은 `make -B all`로 현재 소스를 빌드하고 `make all`의 재링크 여부를 검사한다.
기존 빌드 산출물은 갱신된다. 원본 소스, 학교 fixture, 기존 테스트는 수정하지 않는다.
임시 fixture와 로그는 출력된 `/tmp/nm-eval-auto-*` 아래 보존한다.
각 명령의 인자 JSON, stdout, stderr, 종료 상태와 `results.json`, 도구 버전,
PDF SHA-256을 남긴다. 런타임 제한은 15초, Valgrind 60초, 프로젝트 빌드 120초다.

기본 실행에서 32비트와 메모리 검사를 모두 수행한다. 결과는 PASS/FAIL만 사용한다.
전체 PASS이면 종료 0, FAIL이 있으면 종료 1이다. make는 하위 명령 실패를 보통 2로
보고하므로 상세 결과는 `results.json`을 본다. 도구·ABI 부재, 권한 검증 불가,
검사 도구 자체 중단도 검증 완료로 간주하지 않고 FAIL로 처리한다.
예전 `--memory`, `--require-32` 인자는 호환 목적으로만 받으며 기본 동작과 같다.

Valgrind가 손상된 mmap ELF의 debug image를 읽다가 중단하면, 같은 Makefile과
현재 소스로 임시 ASan·UBSan·LeakSanitizer 실행 파일을 빌드해 해당 입력을 재검사한다.
원본 fixture나 mmap 구현은 바꾸지 않는다. 대체 실행의 stdout/stderr/exit가
일반 실행과 모두 같아야 PASS이며 sanitizer의 추가 진단이나 누수는 FAIL이다.
누수 검사를 위해 ptrace 제한이 없는 실행 환경이 필요하다. Valgrind 원래 로그와
sanitizer 대체 실행 로그를 모두 보존한다.

소유권·코드 설명·shortcut 부재·제출 구조의 타당성은 사람 검토 항목이다.
자동 실행 결과에 SKIP이나 PASS로 넣지 않으며 이 문서의 대응표와
`environment.json`의 `manual_review`에 별도 기록한다. 자동 테스트 성공은
이 사람 검토까지 통과했다는 뜻이 아니다.

## 실제로 읽은 근거 문서

- 저장소 루트 `nm.pdf`: Version 4.0, 9 PDF 페이지. 인쇄 p.5–6
  (PDF p.6–7)은 Linux ELF mandatory, 인쇄 p.7은 보너스다.
- 저장소 루트 `eval.pdf`: Last update 08.2026, 6 PDF 페이지.
  p.2–3은 예기치 않은 종료·메모리 누수 및 준비 절차,
  p.3–5는 mandatory 평가 항목, p.5–6은 보너스다.
  PDF는 42 EvalHub 페이지의 인쇄본이다. 공식 평가 시스템 원본과 동일한지는 미확인이다.
- 두 PDF 모두 실제 파일을 pypdf로 전체 추출해 읽고 annotation 링크도 확인했다.
  PDF에 내장된 attachment는 없으며, eval.pdf p.3에 외부 다운로드 링크가 있다.
  별도 제공 첨부 경로는 이번 요청에 없었다. 저장소에서 두 파일 모두 발견했다.
  PDF reader는 테스트 실행 의존성이 아니다.
- `codex.md`, `rule.md`, Makefile, 기존 테스트 runner와 공통 helper를 읽었다.
  저장소 및 상위 경로에서 적용되는 `AGENTS.md`는 발견되지 않았다.
  이번 요청이 명시한 테스트 자동화와 `eval-auto/README.md` 작성 범위에 한해
  `eval-auto/`와 Makefile을 편집했다.

현재 `.gitignore`가 `eval.pdf`를 제외한다. 이 파일을 제출 저장소에 포함할지는
사용자가 결정해야 하며 이 테스트는 ignore 규칙을 변경하지 않는다.

## 평가 항목 대응표

ID의 `{32,64}`는 두 독립 테스트다. 모든 정상 비교에서 stdout은 정렬·공백·주소
padding을 포함하여 바이트 단위로 일치해야 한다. 진단은 도구명 접두사와
`No such file` / `No such file or directory` 표현만 통일한다.
파일명 제목이나 심볼 목록을 지우거나 재정렬하지 않는다.

| 평가 항목 (eval.pdf) | 테스트 ID | 사용한 fixture | 판정 기준 | 자동화 여부와 미검증 사유 |
|---|---|---|---|---|
| Preliminary tests: 파일 위치, 빌드 (p.3) | `PRE.build`, `PRE.incremental`, `PRE.no-relink` | 현재 Makefile과 전체 소스 | 강제 빌드 성공, 후속 make 성공, ft_nm mtime 유지 | 빌드 자동. 제출 파일 위치의 타당성 및 usual rules 전체 의미는 사람 검토 |
| No cheat, libc 허용, 코드 설명 (p.4) | 사람 검토 (자동 ID 없음) | 제출 저장소 | 소유권, 외부 nm/objdump 실행 등의 shortcut 부재, 작성자 설명 | 수동: 정적 문자열 검색으로 부정행위나 이해도를 판정하지 않음 |
| 비정상 종료 금지 (p.2,4) | 전체 실행 ID, `ERR.*` | 모든 정상·오류 입력 | 시그널 종료·timeout 실패, 오류 입력에서 통제된 처리 | 실행한 입력에 자동. 모든 가능한 입력의 안전성을 증명하지 않음 |
| 메모리 누수 부재 (p.2–3) | 각 실행 ID의 `.memory`, 대체 빌드 `MEM.sanitizer-build` | 각 비교와 동일 입력 | Valgrind leak/error 0 또는 ASan·UBSan·LSan 무진단, 일반 실행과 동작 일치 | 기본 자동. Valgrind debug reader 중단 시 sanitizer로 재검사; 도구 부재·대체 검사 실패는 FAIL |
| nm error tests (p.4) | `ERR.error_header`, `ERR.header_offset_error`, `ERR.header`, `ERR.header_copy`, `ERR.header_and_prog`, `ERR.header_and_prog_copy`, `ERR.unterminated_string`, `ERR.wrong_arch` | p.3 링크와 이름이 대응되는 기존 8개 학교 binary | 아래 오류 fixture 정책 | 자동. CDN 원본과 현재 로컬 파일의 byte 동일성은 미확인 |
| nm error tests: 추가 조작 입력 (p.4) | `ERR.truncated{1,16,40}` | 정상 실행 파일의 선두 1/16/40바이트 복사본 | crash/hang 없이 거부; stderr 존재, stdout 없음 | 자동. 가능한 모든 offset·문자열 손상 조합은 미검증 |
| Nm easy test (p.4) | `EASY.build`, `EASY.64` | 기존 `easy_test.c`를 cc로 빌드 | nm stdout/stderr/exit 비교 | 자동 |
| Test less easy (p.4) | `LESS.{32,64}` | 기존 `not_so_easy_test.c`를 각각 `-m32`/`-m64`로 빌드 | 심볼 순서·값·padding 포함 정확한 출력, 진단·상태 비교 | 기본 자동. ABI 부재 시 FAIL |
| Other Test (p.4–5) | `EXEC.{32,64}`, `PIE.{32,64}`, `OBJ.{32,64}`, `SO.{32,64}`, `OTHER.stripped` | 생성 assembly, non-stripped ET_EXEC/PIE/ET_REL/ET_DYN 및 strip 복사본 | nm 출력·진단·상태 비교 | 자동. “any test”의 무한한 입력 범위는 미검증 |
| Multiples arguments (p.5) | `CLI.multiple`, `CLI.mixed` | 모든 생성 정상 파일, 정상/없는 파일/정상 조합 | 각 파일 제목·순서·심볼, 오류 후 후속 파일 처리, 최종 exit 비교 | 자동 |
| Object files (p.5) | `OBJ.{32,64}` | 생성 `symbols32.o`, `symbols64.o` | stdout 정확한 일치 (본문의 더 엄격한 순서 기준 적용) | 자동 |
| Dynamic library: .so (p.5) | `SO.{32,64}` | 같은 assembly를 GNU ld로 링크한 `so32`, `so64` | stdout 정확한 일치, 진단·상태 비교 | 자동. 시스템의 stripped .so 대신 재현 가능한 non-stripped fixture 사용 |
| Dynamic library: .dylib (p.5) | 해당 없음 | 미사용 | Linux ELF 범위 밖 | 제외: Mach-O 지원은 nm.pdf의 명시 요구가 아님 |
| Universal binary (p.5) | 해당 없음 | 미사용 | Linux ELF 범위 밖 | 제외: 여러 Mach-O 아키텍처를 담은 fat binary는 이번 범위 밖 |
| 보너스 a/g/u/r/p (p.5–6) | 해당 없음 | 미사용 | mandatory와 분리 | 제외: 새 옵션 구현·보너스 테스트 추가 없음 |

본문의 “every other aspect”와 오류 처리 요구를 위한 보충 테스트:

| 평가 항목 | 테스트 ID | 사용한 fixture | 판정 기준 | 자동화 여부와 미검증 사유 |
|---|---|---|---|---|
| 기본 a.out | `CLI.default`, `CLI.default-missing` | 임시 a.out 존재/부재 | nm 출력·진단·exit 비교 | 자동 |
| 파일 오류 | `ERR.empty`, `ERR.text`, `ERR.directory`, `ERR.missing`, `ERR.permission` | 빈 파일, 텍스트, 디렉터리, 부재 경로, mode 000 | nm 출력·진단·exit 비교 | 자동. 권한 우회 identity에서는 검증 불가로 FAIL; 일반 사용자로 실행 |
| stripped 입력 | `OTHER.strip`, `OTHER.stripped` | 정상 실행 파일 복사본을 strip | strip 성공, nm과 no-symbols 처리 비교 | 자동. 주된 정상 테스트는 non-stripped |
| 심볼 분류 | `OBJ.{32,64}` 및 링크 결과 비교 | local/global text·data, BSS, readonly, undefined, weak, absolute, common | 생성 fixture에 대한 nm 전체 심볼 목록 비교 | 자동. 모든 ELF binding/type/section 조합은 미검증 |

## 오류 fixture 정책 및 문서 차이

학교 fixture의 이름만으로 무조건 거부를 요구하지 않는다.
현재 GNU nm 2.42는 `error_header`, `header_offset_error`를 성공 처리한다.
`ERR.*`의 학교·truncation 그룹에서는 reference 성공 시 ft_nm도 성공하고
stdout이 같아야 한다. reference 실패 시 ft_nm은 exit 1, 빈 stdout,
비어 있지 않은 stderr를 내야 한다. BFD plugin의 세부 오류 문구는 재현하지 않는다.
어느 경우에도 시그널 종료와 timeout은 실패다. 일반 파일 오류는 이 완화 정책을
사용하지 않고 진단과 종료 상태까지 비교한다.

- nm.pdf 인쇄 p.6은 심볼 **순서**까지 유사해야 한다. eval.pdf p.5의
  `.o`/동적 라이브러리는 순서가 달라도 된다고 한다. 이 자동화는 본문 기준으로
  순서를 보존하며 더 엄격하게 비교한다. 평가표만으로는 허용될 차이도 FAIL일 수 있다.
- 두 문서 모두 심볼 목록 외의 일부 작은 차이를 허용한다. 이 자동화는 위에 명시한
  두 진단 차이만 허용하므로 다른 합리적인 차이는 로그를 보고 사람이 판단해야 한다.
- nm.pdf는 Linux와 x86_32/x86_64 ELF `.o`/실행 파일/`.so`를 명시한다.
  eval.pdf에는 `.dylib`, `lipo`, universal binary 등 macOS 시대 항목이 남아 있다.
  이들은 Linux ELF mandatory와 구분해 제외했다. archive `.a`도 별도 미검증이다.
- eval.pdf의 No cheat 항목과 nm.pdf는 모두 libc 전체를 허용한다.
  libft 의무나 일반 libc 호출 금지를 임의로 추가하지 않았다.

## 첨부 fixture 추적

eval.pdf p.3 annotation에서 읽은 URL은
`https://cdn.intra.42.fr/document/document/<ID>/<name>` 형식이다.

| ID | 링크 파일명 | 재사용한 로컬 파일 (`tests/eval_test/` 아래) |
|---|---|---|
| 55635 | header_and_prog_copy | header_and_prog_copy |
| 55636 | header_offset_error | header_offset_error |
| 55637 | header_copy | header_copy |
| 55638 | easy_test.c | easy_test.c |
| 55639 | not_so_easy_test.c | not_so_easy_test.c |
| 55640 | header_and_prog | header_and_prog |
| 55641 | wrong_arch | wrong_arch |
| 55642 | unterminated_string | unterminated_string |
| 55643 | header | header |
| 55644 | error_header | error_header |

별도 subject 링크는 `https://cdn.intra.42.fr/pdf/pdf/223037/en.subject.pdf`다.
외부 링크 대상은 다운로드하지 않았다. 이번 요구사항은 실제로 읽은 로컬 nm.pdf와
eval.pdf를 근거로 구성했다. 링크 파일과 로컬 fixture의 이름 대응만 확인했으며,
내용 동일성을 검증했다고 주장하지 않는다.

## 이번 환경의 검증 결과

2026-10-31, GCC 13.3.0 / GNU nm 2.42 / Valgrind 3.22.0에서
추가 인자 없이 `make eval-auto` 실행: **70 PASS / 0 FAIL**, 종료 상태 **0**.
ELF32/64 비교와 메모리 검사를 모두 실행했고, 기존 손상 fixture 4건도
ASan·UBSan·LeakSanitizer 대체 검사로 통과했다. SKIP 결과는 없다.

로그: `/tmp/nm-eval-auto-yy2z96jn/results.json` (임시 경로이므로 영속성 보장 없음).
이 실행은 LeakSanitizer의 ptrace 제한이 없는 환경에서 수행했다.
사람 검토 항목은 위 대응표에 별도로 남아 있다.

