# Shared configuration for the Day 1-3 Bash runners.
# The caller supplies sanitizers; keep the default checks unless explicitly changed.
sanitizer_flags=()
case "$sanitizers" in
    address,undefined|address|undefined)
        sanitizer_flags=("-fsanitize=$sanitizers" -fno-omit-frame-pointer)
        ;;
    none) ;;
    *) printf 'Invalid sanitizer selection: %s\n' "$sanitizers" >&2; exit 1 ;;
esac
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
export UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}
printf 'Sanitizers: %s\n' "$sanitizers"
if [[ "$sanitizers" != *undefined* ]]; then
    printf 'SKIP: UBSan (not selected).\n'
fi
if [[ "$sanitizers" != *address* ]]; then
    printf 'SKIP: ASan and LeakSanitizer (not selected).\n'
elif [[ ":$ASAN_OPTIONS:" == *:detect_leaks=0:* ]]; then
    printf 'SKIP: LeakSanitizer (detect_leaks=0).\n'
fi
