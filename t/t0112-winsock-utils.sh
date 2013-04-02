#!/bin/sh

. ./test-lib.sh

test_desciption='windows specific socket utility tests'

save_config() {
	SAVED_WIN_SOCK_TIME_WAIT=`git config --int 'win.sock.time.wait'`
	return 0
}

setup_config() {
	git config --add --int 'win.sock.time.wait' 120	
}


restore_config() {
	if test -n "${SAVED_WIN_SOCK_TIME_WAIT}"
	then
		git config --add --int 'win.sock.time.wait' "${SAVED_WIN_SOCK_TIME_WAIT}"
	else
		git config --unset 'win.sock.time.wait'	
	fi
}

save_config

is_valid_output()
{
	local a_line
	
	local saved_ifs
	saved_ifs="$IFS"
	read a_line
		
	IFS="$saved_ifs"
}

test_expect_success 'run setup' '
	setup_conifg
'
test_expect_success 'read from registry' '
	git-test-win-sock-utils >actual	
'

restore_config

test_done

