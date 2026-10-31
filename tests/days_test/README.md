# days_test

Day1~4를 일차별 실행 대신 경계·오류 계약별로 묶은 통합 실행기다.
C fixture와 독립 참조 비교기는 fixtures/에 통합했다. 이전 일차별 범위·실행 기록은 reference/에 보존했다.

```sh
make days_test CC=/usr/bin/gcc
# 32비트 ABI 검사를 필수로 하고 개별 결과 저장
CC=/usr/bin/gcc python3 tests/days_test/run.py --require-32 --report /tmp/days-test.json
```

| 검사 | 건수 | 경계·오류 기준 |
|---|---:|---|
| 파일 수명 | 8 | 빈/없는 파일·디렉터리, 반복/중복 정리, 음수·최대 크기 |
| 실제 32비트 변환 | 3 | 음수, SIZE_MAX 보존, SIZE_MAX+1 거부 |
| ELF 식별 | 7 | 짧은 입력·범위·magic/class/endian/version |
| 헤더·테이블 | 54 | 잘린 테이블, 잘못된 offset/count/entry size, 확장 수량 |
| 심볼 수집·참조 비교 | 46 | 잘못된 link/index/이름, NUL 누락, 이름 길이 경계, 실제 .o |
| 분류·출력 | 76 | weak/binding/section 속성 경계, 잘못된 index, 실제 출력 비교 |
| **합계** | **194** | 정상 대조군도 유지해 정상 입력의 과잉 거부를 감지 |

native ABI에서 표현 불가능해 N/A였던 SIZE_MAX+1을 실행하는 대신, size_t=32비트와
off_t=64비트인 실제 ABI로 해당 분기를 검증한다. 32비트 변환 3건은 sanitizer 없이
실행하며 32비트 libc 개발 환경과 커널 실행 지원이 필요하다.
ABI probe 실패 시 3 SKIP으로 표시한다. `--require-32`에서는 FAIL로 처리한다.

기본 ASan·UBSan과 누수 검사를 유지한다. CLI 출력 비교도 새로 빌드한 sanitizer
바이너리를 사용한다. `NM_TEST_SANITIZERS=address|undefined|address,undefined|none` 및
호출자의 ASAN_OPTIONS/UBSAN_OPTIONS를 지원한다. 기존 DAY1~DAY4별 override는
통합 실행기에서는 사용하지 않는다.

샌드박스는 32비트 시스템 호출과 LeakSanitizer를 제한할 수 있다. 이 경우 지원되는
환경에서 실행한다. native 검사만 실행하려면 누수 제외를 명시할 수 있지만, 이는
전체 검증 완료가 아니다.

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make days_test
```

종료 상태는 전체 통과 0, 실패 1, 실패 없이 검사 제외가 있으면 2다. 빌드 실패는
해당 그룹을 실행하지 못한 FAIL로 표시한다. timeout은 각 명령에 20초를 적용한다.

이 실행기는 기존 Day1~4 범위를 통합한 것이다. Day5의 전체 형식·옵션 조합과
별도 .shstrtab 변형, 모든 payload·누수/실패 주입 검증을 대체하지 않는다.
기존 tests/day1~day4 폴더와 make test-1~test-4/test-1-32는 제거했다. 실행 진입점은 make days_test로 통일한다.
