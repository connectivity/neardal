aclocal
libtoolize --copy
autoheader
autoconf
automake --add-missing --copy --foreign
rm -Rf autom4te.cache

./configure --disable-dependency-tracking $@
