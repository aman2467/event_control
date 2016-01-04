# =========================================================================================
#  @file    : Makefile
#
#  @description : Makefile for project.
#
#  @author  : Aman Kumar (2015)
#
#  @copyright   : The code contained herein is licensed under the GNU General Public License.
#               You may obtain a copy of the GNU General Public License Version 2 or later
#               at the following locations:
#               http://www.opensource.org/licenses/gpl-license.html
#               http://www.gnu.org/copyleft/gpl.html
# =========================================================================================*/

VERBOSE = @
BASEDIR = $(PWD)
PROJECT="event_control"
CC =gcc
NONE=\033[0m
BOLD=\033[01;37m
GREEN=\033[01;32m
RED=\033[01;31m
BIN_DIR=$(BASEDIR)/bin
INC_DIR=$(BASEDIR)/inc
SRC_DIR=$(BASEDIR)/src
HELPER_DIR=$(BASEDIR)/helper
UTILS_DIR=$(BASEDIR)/utils
CFLAGS += -g -O3 -Wall
LIBS= -lpthread
CPPFLAGS += -I. \
		-I./inc/ \

.PHONY: clean all ${PROJECT} cscope

all: ${PROJECT}

${PROJECT}:
	${VERBOSE}${CC} $(SRC_DIR)/${PROJECT}.c $(HELPER_DIR)/socket.c ${CFLAGS} ${CPPFLAGS} -o ${BIN_DIR}/${PROJECT} ${LIBS}
	${VERBOSE}echo "${BOLD}All ${GREEN}Done${BOLD}..!!${NONE}"

clean:
	${VERBOSE}rm -f $(BIN_DIR)/*
	${VERBOSE}echo "${BOLD}All ${RED}Cleaned${NONE}"

cscope:
	${VERBOSE}rm -f cscope.*
	${VERBOSE}cscope -bvRq > /dev/null
