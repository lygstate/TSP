# This CMake script may be used
# with a command line like
# cmake -DWDIR:PATH=<workdir_path> -DODIR:PATH=<output_path> -P CreatePatchCVS.cmake
#
# The script will
#   1) find an appropriate cvs command.
#   2) create a unified diff from CVS diff command
#      rooted at WDIR and put the resulting diff
#      in ODIR/patch.diff
FIND_PROGRAM(CVS_COMMAND
             NAMES cvs 
             DOC "The command line cvs command")

SET(CVS_DIFF_ARGS "diff -u")
MESSAGE(STATUS "CVS Patch:: Generating patch using CVS...")
MESSAGE(STATUS "CVS Patch:: Using command     : ${CVS_COMMAND} ${CVS_DIFF_ARGS}")
MESSAGE(STATUS "CVS Patch:: Rooted at Workdir : ${WDIR}")
EXECUTE_PROCESS(COMMAND ${CVS_COMMAND} ${CVS_DIFF_ARGS}
                OUTPUT_FILE ${ODIR}/patch.diff
                RESULT_VARIABLE PATCHOK
                WORKING_DIRECTORY ${WDIR}
                ERROR_QUIET)               
MESSAGE(STATUS "CVS Patch:: Patch file is : ${ODIR}/patch.diff")              