# Tests

Day1~4 회귀는 [days_test](days_test/README.md)로 통합했다.
필요한 fixture와 비교기는 `days_test/fixtures/`, 이전 범위·실행 기록은
`days_test/reference/`에 보존했다. 기존 `day1`~`day4` 폴더와 일차별 Makefile 명령은 제거했다.

```sh
make days_test CC=/usr/bin/gcc
# 32비트 검사를 필수로 하고 상세 결과 저장
CC=/usr/bin/gcc python3 tests/days_test/run.py --require-32 --report /tmp/days-test.json
```

총 194건: 경계·오류 검사, 정상 대조군, 실제 ELF32/64 참조 및 출력 비교.
기본은 ASan·UBSan·누수 검사이며, 실제 32비트 크기 변환 3건은 sanitizer 없이 실행한다.
기존 native N/A였던 SIZE_MAX+1 거부도 실제 ABI로 검증한다.

`NM_TEST_SANITIZERS=address|undefined|address,undefined|none` 및
`ASAN_OPTIONS`·`UBSAN_OPTIONS`를 지원한다. 생략된 검사는 통과로 계산하지 않는다.

현재 샌드박스는 32비트 실행과 LeakSanitizer를 제한하므로 지원되는 환경에서 실행한다.
누수 검사를 명시적으로 제외하려면:

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make days_test CC=/usr/bin/gcc
```

ABI 미지원 시 3 SKIP(종료 2), `--require-32` 사용 시 FAIL이다.
32비트 libc 개발 파일과 실행 지원이 필요하다. libc의 strlcpy도 필요하다.

`eval_test/`는 학교 공식 입력 원본이며 변경하지 않았다.
Day5 전용 검사는 현재 임시 스크립트로 실행하며 저장소 등록은 별도 작업이다.
