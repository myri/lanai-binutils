#! /bin/sh

warn () {
    echo "@@@" "$@" "@@@"
}
die () {
    warn "$@"
    exit 1
}
must () {
    "$@" || die "$@" "failed."
}

# V is the release version number, extracted from the source code.
V=`fgrep AM_INIT_AUTOMAKE bfd/configure.in \
    | sed -e 's/[\t ]*//g' \
    | sed -e 's@[^,]*,\([^)]*\).*@\1@' \
    | sed -e 's/[^-._a-zA-Z0-9]/_/g'` || die "Could not extract version"
# NM is the name of untarred directory, and basename of tarball.
NM="lanai-binutils-$V"
tarball="../$NM.tar.bz2"

# Verify that all files that will be tarred are committed, so we have a
# record of exactly what was released.

# First, verify that there are no untracked files or directories.
test `git clean -nfxd | wc -l` = 0 || die "Need to \"git clean -fxd\" with care"
# Next, verify all changes are committed.
test `git status | fgrep modified: | wc -l` = 0 || die "Uncommitted modifications"

# Verify the user wants to build the tarball.
# This is a subtle reminder that the version number might need updating.

echo -n "Build $tarball? "
read yes
test "x$yes" = "xyes" || die "You did not type \"yes\"."

# Verify builds work, and cleanup.  This is not required.

must ./configure CFLAGS="-O2 -g" --target=lanai --enable-shared --prefix="`pwd`/install"
must make -j8
must make install
must git clean -fxd
test `git status | fgrep modified: | wc -l` = 0 || die "Build modified files."

# Create the tarball, including the Git commit ID in case we need to
# look at the source later.

( git log | head -1 > .git_id ) || die "Could not create .git_id"
must tar cjf "$tarball.tmp" --exclude-vcs --transform "s@^\.@$NM@" .
must rm .git_id
must mv -f "$tarball.tmp" "$tarball"
echo "Tarball is $tarball"
