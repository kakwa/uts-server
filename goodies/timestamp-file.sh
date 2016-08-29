#!/bin/sh

RCol='\33[0m'    # Text Reset

# Regular            Bold                 Underline            High Intensity       BoldHigh Intens
Bla='\33[0;30m';     BBla='\33[1;30m';    UBla='\33[4;30m';    IBla='\33[0;90m';    BIBla='\33[1;90m';
Red='\33[0;31m';     BRed='\33[1;31m';    URed='\33[4;31m';    IRed='\33[0;91m';    BIRed='\33[1;91m';
Gre='\33[0;32m';     BGre='\33[1;32m';    UGre='\33[4;32m';    IGre='\33[0;92m';    BIGre='\33[1;92m';
Yel='\33[0;33m';     BYel='\33[1;33m';    UYel='\33[4;33m';    IYel='\33[0;93m';    BIYel='\33[1;93m';
Blu='\33[0;34m';     BBlu='\33[1;34m';    UBlu='\33[4;34m';    IBlu='\33[0;94m';    BIBlu='\33[1;94m';
Pur='\33[0;35m';     BPur='\33[1;35m';    UPur='\33[4;35m';    IPur='\33[0;95m';    BIPur='\33[1;95m';
Cya='\33[0;36m';     BCya='\33[1;36m';    UCya='\33[4;36m';    ICya='\33[0;96m';    BICya='\33[1;96m';
Whi='\33[0;37m';     BWhi='\33[1;37m';    UWhi='\33[4;37m';    IWhi='\33[0;97m';    BIWhi='\33[1;97m';

SYSLOG=1

help(){
  cat <<EOF

usage: `basename $0` -i <input file> -u <ts server url> \\
    -o <output ts file> -O <openssl options> -C <curl options>

HTTP timestamping client using openssl and curl (RFC 3161)

arguments:

 * mandatory:
   -i <input file>:     the input file to timestamp
   -u <ts server url>:  the timestamp server url

 * optionnal:
   -l                 :  enable logging to syslog
   -o <output ts file>:  output timestamp file name (default: <input file>.ts)
   -O <openssl options>: openssl additionnal options (man ts for more details) 
   -C <curl options>:    curl additionnal options (man curl for more details)

EOF
  exit 1
}

simple_logger(){
    [ $SYSLOG -eq 0 ] && logger -t `basename $0` -p  user.$1 $2
}

clean(){
    rm -f -- "$TMPREQ"
}

clean_exit(){
    clean
    exit 1
}

exit_error(){
    msg=$1
    simple_logger err "error, $msg"
    printf "${BIRed}[ERROR]   ${IYel}%s${RCol}\n" "$msg"
    clean_exit
}

info(){
    msg=$1
    simple_logger debug "$msg"
    printf "${BIBlu}[INFO]    ${RCol}%s${RCol}\n" "$msg"
}

success(){
    msg=$1
    simple_logger info "$msg"
    printf "${BIGre}[SUCCESS] ${RCol}%s${RCol}\n" "$msg"
}

trap clean_exit HUP INT TERM
TMPREQ=`mktemp`

REMOVE_TS=0

while getopts ":lhru:i:o:O:C:" opt; do
  case $opt in
    h)  help;;
    l)  SYSLOG=0;;
    u)  TS_URL="$OPTARG";;
    i)  INPUT_FILE="`readlink -f $OPTARG`";;
    o)  OUTPUT_FILE="`readlink -f $OPTARG`";;
    O)  OPENSSL_OPTS="$OPTARG";;
    C)  CURL_OPTS="$OPTARG";;
    r)  REMOVE_TS=1;;
    \?) echo "Invalid option: -$OPTARG" >&2; help; exit 1;;
    :)  echo "Option -$OPTARG requires an argument." >&2; help; exit 1;;
  esac
done

# If no output file specified, output to <input file>.ts
[ -z "$OUTPUT_FILE" ]   && OUTPUT_FILE="${INPUT_FILE}.tsr"

# Check that input file exists
[ -f "$INPUT_FILE" ]    || exit_error "Input file '$INPUT_FILE' doesn't exist"
# Check that output file doesn't exit
if [ $REMOVE_TS -eq 1 ]
then
	[ -f "$OUTPUT_FILE" ] && rm -f "$OUTPUT_FILE"
else
	! [ -f "$OUTPUT_FILE" ] || exit_error "Output timestamp file '$OUTPUT_FILE' already exists"
fi
# Check that url is not empty
! [ -z "$TS_URL" ]      || exit_error "Missing timestamp server url"

info "Generating timestamp on file '$INPUT_FILE', to '$OUTPUT_FILE', using server '$TS_URL'"

# Building the timestamp request with openssl
openssl ts $OPENSSL_OPTS \
    -query -data "$INPUT_FILE" \
    -out "$TMPREQ" || exit_error "Request generation failed"

# Submitting the timestamp request to the RFC 3161 server with curl
curl "$TS_URL" $CURL_OPTS \
    -H "Content-Type: application/timestamp-query" \
    -f -g \
    --data-binary @$TMPREQ \
    -o "$OUTPUT_FILE" 2>/dev/null || exit_error "Timestamp query failed"

openssl ts -verify -data "$INPUT_FILE" -in "$OUTPUT_FILE" 2>&1 | grep -q "asn1 encoding routines" && exit_error \
	"Reponse doesn't appear to be a timestamp response"

success "Timestamp of file '$INPUT_FILE' using server '$TS_URL' succeed, ts written to '$OUTPUT_FILE'"

clean
