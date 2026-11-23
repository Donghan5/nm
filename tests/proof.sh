#!/usr/bin/env bash
# Live differential proof: run ft_nm and GNU nm over every real binary on this
# host and compare stdout, stderr and exit status. Nothing is precomputed; the
# numbers on screen come from the run you are watching.
set -uo pipefail

ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
FT="$ROOT/ft_nm"
export LC_ALL=C

DIRS=(/usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu /usr/lib /lib /usr/bin /bin /usr/sbin)

die() { printf '%s\n' "$*" >&2; exit 1; }
[[ -x "$FT" ]] || die "ft_nm not built. Run: make"
command -v nm > /dev/null 2>&1 || die "GNU nm not found in PATH."

# Colours only on a terminal.
if [[ -t 1 ]] && command -v tput > /dev/null 2>&1 && [[ $(tput colors 2>/dev/null || echo 0) -ge 8 ]]; then
    G=$(tput setaf 2); R=$(tput setaf 1); Y=$(tput setaf 3); D=$(tput setaf 8); B=$(tput bold); N=$(tput sgr0)
else
    G=""; R=""; Y=""; D=""; B=""; N=""
fi
TTY=0; [[ -t 1 ]] && TTY=1

tmp=$(mktemp -d "${TMPDIR:-/tmp}/ft_nm-proof.XXXXXX") || die "cannot create temp dir"
cleanup() { [[ -n "${tmp:-}" && -d "$tmp" ]] && rm -rf -- "$tmp"; [[ "$TTY" == 1 ]] && printf '\033[?25h'; }
trap cleanup EXIT INT TERM

rule() { printf '  %s%s%s\n' "$D" "$(printf '─%.0s' $(seq 1 62))" "$N"; }

printf '\n  %sft_nm%s  vs  %sGNU nm%s   —  차분 증명\n' "$B" "$N" "$B" "$N"
rule
printf '  대상: 이 호스트의 모든 ELF 파일\n'
printf '        %s\n' "${DIRS[*]}"
printf '  %s목록 수집 중…%s' "$D" "$N"

# Collect the file list once so the progress bar has a real total. The subject
# scopes this project to ELF binaries, object files and shared libraries, so the
# list is filtered by what the file(1) command actually identifies: an ar
# archive is not an ELF file and is counted as skipped, never as a pass.
all="$tmp/all"; list="$tmp/list"
find "${DIRS[@]}" -maxdepth 1 -type f 2>/dev/null | sort -u > "$all"
seen=$(wc -l < "$all")
if command -v file > /dev/null 2>&1; then
    file -b --mime-type -f "$all" > "$tmp/types" 2>/dev/null
    if [[ $(wc -l < "$tmp/types") == "$seen" ]]; then
        paste -d'\t' "$tmp/types" "$all" \
            | grep -E '^application/x-(executable|pie-executable|sharedlib|object|coredump)\b' \
            | cut -f2- > "$list"
    else
        grep -v '\.a$' "$all" > "$list"
    fi
else
    grep -v '\.a$' "$all" > "$list"
fi
total=$(wc -l < "$list")
skipped=$(( seen - total ))
printf '\r  대상 파일 %s%s개%s  %s(ELF 아닌 %s개 제외: ar 아카이브, 스크립트 등)%s\n\n' \
    "$B" "$total" "$N" "$D" "$skipped" "$N"
[[ "$total" -gt 0 ]] || die "no ELF files found to compare"

match=0; mism=0; crash=0; flaky=0; done_n=0
start=$SECONDS
drawn=0
findings="$tmp/findings"
: > "$findings"

# One comparison. Returns 0 match, 1 mismatch, 2 crash/hang.
compare_once() {
    local f=$1 tag=$2
    nm "$f" > "$tmp/$tag.nm.out" 2> "$tmp/$tag.nm.err"; local na=$?
    timeout 20s "$FT" "$f" > "$tmp/$tag.ft.out" 2> "$tmp/$tag.ft.err"; local fa=$?
    [[ "$fa" -ge 124 ]] && return 2
    [[ "$na" == "$fa" ]] || { echo "exit nm=$na ft_nm=$fa" > "$tmp/$tag.why"; return 1; }
    cmp -s "$tmp/$tag.nm.out" "$tmp/$tag.ft.out" || { echo "stdout" > "$tmp/$tag.why"; return 1; }
    if ! cmp -s "$tmp/$tag.nm.err" "$tmp/$tag.ft.err"; then
        # Only the program name may differ on stderr.
        sed 's/^nm: /@: /' "$tmp/$tag.nm.err" > "$tmp/$tag.nm.err2"
        sed 's/^ft_nm: /@: /' "$tmp/$tag.ft.err" > "$tmp/$tag.ft.err2"
        cmp -s "$tmp/$tag.nm.err2" "$tmp/$tag.ft.err2" || { echo "stderr" > "$tmp/$tag.why"; return 1; }
    fi
    return 0
}

draw() {
    [[ "$TTY" == 1 ]] || return 0
    local cur=$1
    local width=30
    local filled=$(( total ? done_n * width / total : 0 ))
    local pct=$(( total ? done_n * 100 / total : 0 ))
    local el=$(( SECONDS - start ))
    local eta="—"
    [[ "$done_n" -gt 20 ]] && eta="$(( el * (total - done_n) / done_n ))s"
    local bar
    bar="$(printf '█%.0s' $(seq 1 $((filled > 0 ? filled : 0)) ) 2>/dev/null)"
    bar+="$(printf '░%.0s' $(seq 1 $((width - filled > 0 ? width - filled : 0)) ) 2>/dev/null)"
    [[ "$drawn" == 1 ]] && printf '\033[4A'
    drawn=1
    printf '  [%s%s%s]  %s/%s   %s%%\033[K\n' "$G" "$bar" "$N" "$done_n" "$total" "$pct"
    printf '  %s현재%s     %.46s\033[K\n' "$D" "$N" "$cur"
    printf '  %s일치%s     %s%-8s%s %s불일치%s %s%-6s%s %s크래시%s %s\033[K\n' \
        "$D" "$N" "$G" "$match" "$N" "$D" "$N" \
        "$([[ $mism -gt 0 ]] && printf '%s' "$R")" "$mism" "$N" "$D" "$N" \
        "$([[ $crash -gt 0 ]] && printf '%s%s%s' "$R" "$crash" "$N" || printf '%s' "$crash")"
    printf '  %s경과%s     %ss        %s남은 예상%s %s\033[K\n' "$D" "$N" "$el" "$D" "$N" "$eta"
}

[[ "$TTY" == 1 ]] && printf '\033[?25l'
while IFS= read -r f; do
    done_n=$((done_n + 1))
    compare_once "$f" a; rc=$?
    if [[ "$rc" == 0 ]]; then
        match=$((match + 1))
    else
        # Re-verify before recording: a result that does not reproduce is
        # reported as nondeterministic rather than quietly counted either way.
        compare_once "$f" b; rc2=$?
        if [[ "$rc2" != "$rc" ]]; then
            flaky=$((flaky + 1))
            printf 'FLAKY   %s (1회차 rc=%s, 2회차 rc=%s)\n' "$f" "$rc" "$rc2" >> "$findings"
        elif [[ "$rc" == 2 ]]; then
            crash=$((crash + 1))
            printf 'CRASH   %s (타임아웃 또는 시그널)\n' "$f" >> "$findings"
        else
            mism=$((mism + 1))
            printf 'MISMATCH %s :: %s\n' "$(cat "$tmp/b.why" 2>/dev/null)" "$f" >> "$findings"
            { diff -u "$tmp/b.nm.out" "$tmp/b.ft.out" | head -12
              diff -u "$tmp/b.nm.err" "$tmp/b.ft.err" | head -6; } >> "$findings" 2>/dev/null
        fi
    fi
    if [[ "$TTY" == 1 ]]; then
        if [[ $((done_n % 8)) == 0 || "$done_n" == "$total" ]]; then
            draw "$(basename -- "$f")"
        fi
    elif [[ $((done_n % 400)) == 0 ]]; then
        printf '  %s/%s  일치 %s  불일치 %s  크래시 %s\n' \
            "$done_n" "$total" "$match" "$mism" "$crash"
    fi
done < "$list"
draw "완료"
[[ "$TTY" == 1 ]] && printf '\033[?25h'

elapsed=$(( SECONDS - start ))
printf '\n'
rule
printf '\n'
bad=$(( mism + crash + flaky ))
if [[ "$bad" == 0 ]]; then
    printf '  %s%s✔%s  %s%s / %s  완전 일치%s\n' "$B" "$G" "$N" "$B" "$match" "$total" "$N"
    printf '     stdout · stderr · exit code  모두 바이트 단위\n'
    printf '     불일치 0   크래시 0   경과 %ss\n' "$elapsed"
else
    printf '  %s%s✗%s  %s / %s 일치,  %s불일치 %s   크래시 %s   비결정 %s%s\n' \
        "$B" "$R" "$N" "$match" "$total" "$R" "$mism" "$crash" "$flaky" "$N"
    printf '     경과 %ss\n\n' "$elapsed"
    printf '  %s--- 상세 ---%s\n' "$D" "$N"
    head -60 "$findings" | sed 's/^/  /'
fi
printf '\n  %s비교 방식: nm 과 ft_nm 을 같은 인자로 실행, stdout 바이트 비교,%s\n' "$D" "$N"
printf '  %sstderr 는 프로그램명만 정규화, 종료코드 일치 요구.%s\n' "$D" "$N"
printf '  %s스크립트: tests/proof.sh   (사전 계산 없음 — 지금 실행한 결과)%s\n\n' "$D" "$N"

[[ "$bad" == 0 ]]
