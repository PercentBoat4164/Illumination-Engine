function (RunSequentiallyStartBlock RunSequentially_MUTEX)
    file(LOCK ${RunSequentially_MUTEX})
endfunction()

function (RunSequentiallyEndBlock RunSequentially_MUTEX)
    file(LOCK ${RunSequentially_MUTEX} RELEASE)
endfunction()

function (RunOnceStallAllStartBlock RunOnceStallAll_MUTEX RunOnceStallAll_SHOULD_CONTINUE)
    file(LOCK ${RunOnceStallAll_MUTEX} TIMEOUT 0 RESULT_VARIABLE SHOULD_CONTINUE)
    if (SHOULD_CONTINUE STREQUAL "0")
        set(${RunOnceStallAll_SHOULD_CONTINUE} TRUE PARENT_SCOPE)
    else ()
        set(${RunOnceStallAll_SHOULD_CONTINUE} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function(RunOnceStallAllEndBlock RunOnceStallAll_MUTEX RunOnceStallAll_SHOULD_CONTINUE)
    if (NOT ${RunOnceStallAll_SHOULD_CONTINUE})
        file(LOCK ${RunOnceStallAll_MUTEX} RESULT_VARIABLE IGNORED)
    endif ()
    file(LOCK ${RunOnceStallAll_MUTEX} RELEASE)
endfunction()

function (RunOnceStallOneStartBlock RunOnceStallOne_MUTEX RunOnceStallOne_SHOULD_CONTINUE)
    file(LOCK ${RunOnceStallOne_MUTEX} TIMEOUT 0 RESULT_VARIABLE SHOULD_CONTINUE)
    if (SHOULD_CONTINUE STREQUAL "0")
        set(${RunOnceStallOne_SHOULD_CONTINUE} TRUE PARENT_SCOPE)
    else ()
        set(${RunOnceStallOne_SHOULD_CONTINUE} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function (RunOnceStallOneEndBlock RunOnceStallOne_MUTEX RunOnceStallOne_SHOULD_CONTINUE)
    if (${RunOnceStallOne_SHOULD_CONTINUE})
        file(LOCK ${RunOnceStallOne_MUTEX} RELEASE RESULT_VARIABLE IGNORED)
    endif ()
endfunction()