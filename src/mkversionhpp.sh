#!/bin/sh

# Default version if we can't take it from git
if [ -e version.txt ]; then
    FLORB_V=`cat version.txt`
else
    FLORB_V="v1.0"
fi

# Check whether we have git installed, and do nothing if not
which git > /dev/null 2>&1
if [ $? -eq 0 ]; then
    
    # Check whether this is a git repo with version info at all
    git rev-parse --git-dir > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        FLORB_V=$( git rev-parse --short HEAD )
    fi

    # Use a tag on HEAD if we have one
    git describe --exact-match HEAD > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        FLORB_V=$( git describe --exact-match HEAD )
    fi

fi

echo "#ifndef VERSION_HPP" >  version.hpp.tmp
echo "#define VERSION_HPP" >> version.hpp.tmp

echo "#define FLORB_VERSION     \"$FLORB_V\"" >> version.hpp.tmp
echo "#define FLORB_PROGSTR     \"florb $FLORB_V\"" >> version.hpp.tmp
echo "#define FLORB_USERAGENT   \"florb/$FLORB_V\"" >> version.hpp.tmp

echo "#endif //VERSION_HPP" >> version.hpp.tmp

diff version.hpp.tmp version.hpp >/dev/null 2>&1
if [ $? -eq 0 ]; then
    rm version.hpp.tmp
else
    mv version.hpp.tmp version.hpp
fi
