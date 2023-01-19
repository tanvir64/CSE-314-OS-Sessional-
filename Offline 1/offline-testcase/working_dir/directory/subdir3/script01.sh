find "$1" -type f -! -perm 0600 -exec chmod 0600 '{}' ';'
find "$1" -type d -! -perm 0700 -exec chmod 0700 '{}' ';'

