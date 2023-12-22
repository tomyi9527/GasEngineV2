# 最初来自于 https://github.com/rpavlik/cmake-modules/blob/master/GetGitRevisionDescription.cmake
# 参考其结构进行修改，加入一些其他用法。
#
#  # get commit hash on current head
#  get_git_head_revision(REF_NAME COMMIT_HASH)
#  # get tag on some commit
#  git_describe(COMMIT_TAG --exact-match ${COMMIT_HASH})
#  # get tag info
#  git_tag(${COMMIT_TAG} TAG_MESSAGE "--format=%(contents)")
#  # get commit info
#  git_log(${COMMIT_HASH} COMMIT_AUTHOR --pretty=format:%an)
#  git_log(${COMMIT_HASH} COMMIT_TIME --pretty=format:%cd --date=iso)
#  git_log(${COMMIT_HASH} COMMIT_MESSAGE --pretty=format:%s)
#  # get branch name
#  git_current_branch(BRANCH_NAME)

if(__get_git_revision_description)
	return()
endif()
set(__get_git_revision_description YES)

# We must run the following at "include" time, not at function call time,
# to find the path to this module rather than the path to a calling list file
get_filename_component(_gitdescmoddir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(get_git_head_revision _refspecvar _hashvar)
	set(GIT_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
	set(GIT_DIR "${GIT_PARENT_DIR}/.git")
	while(NOT EXISTS "${GIT_DIR}")	# .git dir not found, search parent directories
		set(GIT_PREVIOUS_PARENT "${GIT_PARENT_DIR}")
		get_filename_component(GIT_PARENT_DIR ${GIT_PARENT_DIR} PATH)
		if(GIT_PARENT_DIR STREQUAL GIT_PREVIOUS_PARENT)
			# We have reached the root directory, we are not in git
			set(${_refspecvar} "GITDIR-NOTFOUND" PARENT_SCOPE)
			set(${_hashvar} "GITDIR-NOTFOUND" PARENT_SCOPE)
			return()
		endif()
		set(GIT_DIR "${GIT_PARENT_DIR}/.git")
	endwhile()
	# check if this is a submodule
	if(NOT IS_DIRECTORY ${GIT_DIR})
		file(READ ${GIT_DIR} submodule)
		string(REGEX REPLACE "gitdir: (.*)\n$" "\\1" GIT_DIR_RELATIVE ${submodule})
		get_filename_component(SUBMODULE_DIR ${GIT_DIR} PATH)
		get_filename_component(GIT_DIR ${SUBMODULE_DIR}/${GIT_DIR_RELATIVE} ABSOLUTE)
	endif()
	if(NOT IS_DIRECTORY "${GIT_DIR}")
		file(READ ${GIT_DIR} worktree)
 		string(REGEX REPLACE "gitdir: (.*)worktrees(.*)\n$" "\\1" GIT_DIR ${worktree})
 	endif()
	set(GIT_DATA "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/git-data")
	if(NOT EXISTS "${GIT_DATA}")
		file(MAKE_DIRECTORY "${GIT_DATA}")
	endif()

	if(NOT EXISTS "${GIT_DIR}/HEAD")
		return()
	endif()
	set(HEAD_FILE "${GIT_DATA}/HEAD")
	configure_file("${GIT_DIR}/HEAD" "${HEAD_FILE}" COPYONLY)

	configure_file("${_gitdescmoddir}/GetGitRevisionDescription.cmake.in"
		"${GIT_DATA}/grabRef.cmake"
		@ONLY)
	include("${GIT_DATA}/grabRef.cmake")

	set(${_refspecvar} "${HEAD_REF}" PARENT_SCOPE)
	set(${_hashvar} "${HEAD_HASH}" PARENT_SCOPE)
endfunction()

function(git_log _hash _var)
	if(NOT GIT_FOUND)
		find_package(Git QUIET)
	endif()
	if(NOT GIT_FOUND)
		set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
		return()
	endif()
	if(NOT _hash)
		set(${_var} "HEAD-HASH-NOTFOUND" PARENT_SCOPE)
		return()
	endif()

	# TODO sanitize
	#if((${ARGN}" MATCHES "&&") OR
	#	(ARGN MATCHES "||") OR
	#	(ARGN MATCHES "\\;"))
	#	message("Please report the following error to the project!")
	#	message(FATAL_ERROR "Looks like someone's doing something nefarious with git_describe! Passed arguments ${ARGN}")
	#endif()

	# message(STATUS "Arguments to execute_process: ${ARGN}")

	execute_process(COMMAND
		"${GIT_EXECUTABLE}" log ${_hash} -1 ${ARGN}
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		RESULT_VARIABLE res
		OUTPUT_VARIABLE out
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(NOT res EQUAL 0)
		set(out "${out}-${res}-NOTFOUND")
	endif()

	set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_current_branch _var)
	if(NOT GIT_FOUND)
		find_package(Git QUIET)
	endif()
	if(NOT GIT_FOUND)
		set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
		return()
	endif()

	execute_process(COMMAND
		"${GIT_EXECUTABLE}" symbolic-ref --short -q HEAD
		RESULT_VARIABLE res
		OUTPUT_VARIABLE out
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	if(NOT res EQUAL 0)
		set(out "${out}-${res}-NOTFOUND")
	endif()

	set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_tag tag _var)
	if(NOT GIT_FOUND)
		find_package(Git QUIET)
	endif()
	if(NOT GIT_FOUND)
		set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
		return()
	endif()

	execute_process(COMMAND
		"${GIT_EXECUTABLE}" tag -l ${tag} ${ARGN}
		RESULT_VARIABLE res
		OUTPUT_VARIABLE out
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	if(NOT res EQUAL 0)
		set(out "${out}-${res}-NOTFOUND")
	endif()

	set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_describe _var)
	if(NOT GIT_FOUND)
		find_package(Git QUIET)
	endif()
	if(NOT GIT_FOUND)
		set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
		return()
	endif()

	# TODO sanitize
	#if((${ARGN}" MATCHES "&&") OR
	#	(ARGN MATCHES "||") OR
	#	(ARGN MATCHES "\\;"))
	#	message("Please report the following error to the project!")
	#	message(FATAL_ERROR "Looks like someone's doing something nefarious with git_describe! Passed arguments ${ARGN}")
	#endif()

	#message(STATUS "Arguments to execute_process: ${ARGN}")

	execute_process(COMMAND
		"${GIT_EXECUTABLE}"
		describe
		${ARGN}
		WORKING_DIRECTORY
		"${CMAKE_CURRENT_SOURCE_DIR}"
		RESULT_VARIABLE
		res
		OUTPUT_VARIABLE
		out
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(NOT res EQUAL 0)
		set(out "${out}-${res}-NOTFOUND")
	endif()

	set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_get_exact_tag _var)
	git_describe(out --exact-match ${ARGN})
	set(${_var} "${out}" PARENT_SCOPE)
endfunction()

function(git_local_changes _var)
	if(NOT GIT_FOUND)
		find_package(Git QUIET)
	endif()
	if(NOT GIT_FOUND)
		set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
		return()
	endif()
	get_git_head_revision(ref hash)
	if(NOT hash)
		set(${_var} "HEAD-HASH-NOTFOUND" PARENT_SCOPE)
		return()
	endif()

	execute_process(COMMAND
		"${GIT_EXECUTABLE}"
		diff-index --quiet HEAD --
		WORKING_DIRECTORY
		"${CMAKE_CURRENT_SOURCE_DIR}"
		RESULT_VARIABLE
		res
		OUTPUT_VARIABLE
		out
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(res EQUAL 0)
		set(${_var} "CLEAN" PARENT_SCOPE)
	else()
		set(${_var} "DIRTY" PARENT_SCOPE)
	endif()
endfunction()
