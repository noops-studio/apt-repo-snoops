PHP_ARG_ENABLE(mysqldump_ext, whether to enable mysqldump_ext support,
[ --enable-mysqldump_ext   Enable mysqldump_ext support])

if test "$PHP_MYSQLDUMP_EXT" != "no"; then
    PHP_NEW_EXTENSION(mysqldump_ext, mysqldump_ext.c, $ext_shared)
fi
