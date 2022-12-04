#!/usr/bin/env bash

clean=$1
if [ ${clean}v = "clean"v ]; then
    clean=true
else 
    clean=false
fi

declare -A options=(
    ["buildExamples"]="OFF" 
    ["useEpoll"]="ON" 
    ["buildType"]="Debug")

die() { echo "$*" >&2; exit 2; }  # complain to STDERR and exit with error

usage(){
    echo "usage: $0 [--buildExamples[=]<true|false> --useEpoll[=]<true|false> --buildType[=]<value>]" >&2
}

setCmakeOption() { 
    if [ -z "$OPTARG" ];then
        value=ON;
    elif [ "$OPTARG" = "false" ];then
        value=OFF;
    elif [ "$OPTARG" = "true" ];then
        value=ON;
    else
        die "Not support value $OPTARG for --$OPT option";
    fi; 
    options[$1]=${value}
}

optspec=":h-:"
while getopts "$optspec" OPT; do
    # support long options: https://stackoverflow.com/a/28466267/519360
    if [ "$OPT" = "-" ]; then     # long option: reformulate OPT and OPTARG
        OPT="${OPTARG%%=*}"       # extract long option name
        OPTARG="${OPTARG#$OPT}"   # extract long option argument (may be empty)
        OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
    fi
    case "${OPT}" in
        buildExamples)
            setCmakeOption buildExamples
            ;;
        useEpoll)
            setCmakeOption useEpoll
            ;;
        h)
            usage
            exit 2
            ;;
        ??* ) 
            die "Illegal option --$OPT" ;;  # bad long option
        ? ) exit 2   ;;  # bad short option (error reported via getopts)
    esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

build() {
    if ${clean}; then
        if [ -d build ]; then
            rm -rf build
        fi
    else
        if [ ! -d "build" ]; then mkdir build ;fi
        cd build
        if [ -f "Makefile" ]; then make clean ;fi

        cmake \
        --no-warn-unused-cli \
        -DBUILD_EXAMPLES=${options["buildExamples"]} \
        -DBUILD_USE_EPOLL=${options["useEpoll"]}  \
        -DCMAKE_BUILD_TYPE:STRING=${options["buildType"]} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
        -S.. \
        -B../build \
        -G "Unix Makefiles"
    
        make -j 6
        cd ..
    fi
}

build
