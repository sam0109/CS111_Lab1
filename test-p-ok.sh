#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
true

g++ -c foo.c

: : :

cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

a b<c > d

cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!

a&&b||
 c &&
  d | e && f|

g<h

# This is a weird example: nobody would ever want to run this.
a<b>c|d<e>f|g<h>i
EOF

cat >testb.sh <<'EOF'
(echo a || echo b)

echo a > testb2.exp && echo b

EOF

cat >testc.sh <<'EOF'
echo test1 >testc.out

echo test2 >testc.out

echo test3 >testc.out

echo test4 >testc2.out

EOF

cat >test.exp <<'EOF'
# 1
  true
# 2
  g++ -c foo.c
# 3
  : : :
# 4
      cat</etc/passwd \
    |
      tr a-z A-Z \
    |
      sort -u \
  ||
    echo sort failed!
# 5
  a b<c>d
# 6
      cat</etc/passwd \
    |
      tr a-z A-Z \
    |
      sort -u>out \
  ||
    echo sort failed!
# 7
        a \
      &&
        b \
    ||
      c \
  &&
      d \
    |
      e \
  &&
      f \
    |
      g<h
# 8
    a<b>c \
  |
    d<e>f \
  |
    g<h>i
EOF

cat >testb.exp <<'EOF'
a
b
EOF

cat >testc.exp <<'EOF'
test3
EOF

cat >testc2.exp <<'EOF'
test4
EOF

../timetrash -p test.sh >test.out 2>test.err || exit

../timetrash testb.sh >testb.out 2>testb.err || exit

../timetrash -t testc.sh 2>testc.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

diff -u testb.exp testb.out || exit
test ! -s testb.err || {
  cat testb.err
  exit 1
}

diff -u testc.exp testc.out || exit
diff -u testc2.exp testc2.out || exit
test ! -s testc.err || {
  cat testc.err
  exit 1
}

) || exit

rm -fr "$tmp"
