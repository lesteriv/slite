#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/api/API_backups.o \
	${OBJECTDIR}/src/third/libcrest/third/zlib/deflate.o \
	${OBJECTDIR}/src/third/libcrest/src/crest.o \
	${OBJECTDIR}/src/sp/VMachine.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/api/API_config.o \
	${OBJECTDIR}/src/api/API_records.o \
	${OBJECTDIR}/src/api/API_procedures.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_json.o \
	${OBJECTDIR}/src/sp/ParserUtil.o \
	${OBJECTDIR}/src/sp/Command.o \
	${OBJECTDIR}/src/sp/Variable.o \
	${OBJECTDIR}/src/utils/config.o \
	${OBJECTDIR}/src/sp/ParserBlocks.o \
	${OBJECTDIR}/src/sp/CoreFunctions.o \
	${OBJECTDIR}/src/third/libcrest/src/auth_basic.o \
	${OBJECTDIR}/src/sp/checker.o \
	${OBJECTDIR}/src/third/libcrest/third/mongoose/mongoose.o \
	${OBJECTDIR}/src/utils/time_counter.o \
	${OBJECTDIR}/src/sp/Function.o \
	${OBJECTDIR}/src/sp/Value.o \
	${OBJECTDIR}/src/sp/Core.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_connection.o \
	${OBJECTDIR}/src/sp/Statement.o \
	${OBJECTDIR}/src/third/libcrest/src/auth_digest.o \
	${OBJECTDIR}/src/api/API_logs.o \
	${OBJECTDIR}/src/sp/GlobalVariables.o \
	${OBJECTDIR}/src/sp/Utils.o \
	${OBJECTDIR}/src/sp/Parser.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_user_manager.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_utils.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_cache.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_string_map.o \
	${OBJECTDIR}/src/sp/ParserCommands.o \
	${OBJECTDIR}/src/api/API_databases.o \
	${OBJECTDIR}/src/third/sqlite/sqlite3.o \
	${OBJECTDIR}/src/sp/Unicode.o \
	${OBJECTDIR}/src/sp/Expression.o \
	${OBJECTDIR}/src/db/database.o \
	${OBJECTDIR}/src/third/libcrest/third/zlib/trees.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_result.o \
	${OBJECTDIR}/src/api/API_users.o \
	${OBJECTDIR}/src/third/libcrest/src/cr_xml.o \
	${OBJECTDIR}/src/api/API_server.o \
	${OBJECTDIR}/src/utils/utils.o \
	${OBJECTDIR}/src/sp/Cursor.o


# C Compiler Flags
CFLAGS=-Ofast -flto -fno-fast-math

# CC Compiler Flags
CCFLAGS=--std=c++11 -Ofast -flto -fno-exceptions
CXXFLAGS=--std=c++11 -Ofast -flto -fno-exceptions

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -ldl -lrt

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/slite

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/slite: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/slite ${OBJECTFILES} ${LDLIBSOPTIONS} -flto -Ofast -Wl,--gc-sections -s

${OBJECTDIR}/src/api/API_backups.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_backups.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_backups.o src/api/API_backups.cpp

${OBJECTDIR}/src/third/libcrest/third/zlib/deflate.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/third/zlib/deflate.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/third/zlib
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/third/zlib/deflate.o src/third/libcrest/third/zlib/deflate.cpp

${OBJECTDIR}/src/third/libcrest/src/crest.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/crest.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/crest.o src/third/libcrest/src/crest.cpp

${OBJECTDIR}/src/sp/VMachine.o: nbproject/Makefile-${CND_CONF}.mk src/sp/VMachine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/VMachine.o src/sp/VMachine.cpp

${OBJECTDIR}/src/main.o: nbproject/Makefile-${CND_CONF}.mk src/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/api/API_config.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_config.o src/api/API_config.cpp

${OBJECTDIR}/src/api/API_records.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_records.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_records.o src/api/API_records.cpp

${OBJECTDIR}/src/api/API_procedures.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_procedures.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_procedures.o src/api/API_procedures.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_json.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_json.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_json.o src/third/libcrest/src/cr_json.cpp

${OBJECTDIR}/src/sp/ParserUtil.o: nbproject/Makefile-${CND_CONF}.mk src/sp/ParserUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/ParserUtil.o src/sp/ParserUtil.cpp

${OBJECTDIR}/src/sp/Command.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Command.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Command.o src/sp/Command.cpp

${OBJECTDIR}/src/sp/Variable.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Variable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Variable.o src/sp/Variable.cpp

${OBJECTDIR}/src/utils/config.o: nbproject/Makefile-${CND_CONF}.mk src/utils/config.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/utils
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils/config.o src/utils/config.cpp

${OBJECTDIR}/src/sp/ParserBlocks.o: nbproject/Makefile-${CND_CONF}.mk src/sp/ParserBlocks.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/ParserBlocks.o src/sp/ParserBlocks.cpp

${OBJECTDIR}/src/sp/CoreFunctions.o: nbproject/Makefile-${CND_CONF}.mk src/sp/CoreFunctions.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/CoreFunctions.o src/sp/CoreFunctions.cpp

${OBJECTDIR}/src/third/libcrest/src/auth_basic.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/auth_basic.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/auth_basic.o src/third/libcrest/src/auth_basic.cpp

${OBJECTDIR}/src/sp/checker.o: nbproject/Makefile-${CND_CONF}.mk src/sp/checker.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/checker.o src/sp/checker.cpp

${OBJECTDIR}/src/third/libcrest/third/mongoose/mongoose.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/third/mongoose/mongoose.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/third/mongoose
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/third/mongoose/mongoose.o src/third/libcrest/third/mongoose/mongoose.cpp

${OBJECTDIR}/src/utils/time_counter.o: nbproject/Makefile-${CND_CONF}.mk src/utils/time_counter.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/utils
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils/time_counter.o src/utils/time_counter.cpp

${OBJECTDIR}/src/sp/Function.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Function.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Function.o src/sp/Function.cpp

${OBJECTDIR}/src/sp/Value.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Value.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Value.o src/sp/Value.cpp

${OBJECTDIR}/src/sp/Core.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Core.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Core.o src/sp/Core.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_connection.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_connection.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_connection.o src/third/libcrest/src/cr_connection.cpp

${OBJECTDIR}/src/sp/Statement.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Statement.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Statement.o src/sp/Statement.cpp

${OBJECTDIR}/src/third/libcrest/src/auth_digest.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/auth_digest.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/auth_digest.o src/third/libcrest/src/auth_digest.cpp

${OBJECTDIR}/src/api/API_logs.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_logs.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_logs.o src/api/API_logs.cpp

${OBJECTDIR}/src/sp/GlobalVariables.o: nbproject/Makefile-${CND_CONF}.mk src/sp/GlobalVariables.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/GlobalVariables.o src/sp/GlobalVariables.cpp

${OBJECTDIR}/src/sp/Utils.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Utils.o src/sp/Utils.cpp

${OBJECTDIR}/src/sp/Parser.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Parser.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Parser.o src/sp/Parser.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_user_manager.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_user_manager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_user_manager.o src/third/libcrest/src/cr_user_manager.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_utils.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_utils.o src/third/libcrest/src/cr_utils.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_cache.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_cache.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_cache.o src/third/libcrest/src/cr_cache.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_string_map.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_string_map.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_string_map.o src/third/libcrest/src/cr_string_map.cpp

${OBJECTDIR}/src/sp/ParserCommands.o: nbproject/Makefile-${CND_CONF}.mk src/sp/ParserCommands.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/ParserCommands.o src/sp/ParserCommands.cpp

${OBJECTDIR}/src/api/API_databases.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_databases.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_databases.o src/api/API_databases.cpp

${OBJECTDIR}/src/third/sqlite/sqlite3.o: nbproject/Makefile-${CND_CONF}.mk src/third/sqlite/sqlite3.c 
	${MKDIR} -p ${OBJECTDIR}/src/third/sqlite
	${RM} $@.d
	$(COMPILE.c) -s -DNDEBUG -Ofast -flto -fno-fast-math -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/sqlite/sqlite3.o src/third/sqlite/sqlite3.c

${OBJECTDIR}/src/sp/Unicode.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Unicode.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Unicode.o src/sp/Unicode.cpp

${OBJECTDIR}/src/sp/Expression.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Expression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Expression.o src/sp/Expression.cpp

${OBJECTDIR}/src/db/database.o: nbproject/Makefile-${CND_CONF}.mk src/db/database.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/db
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/db/database.o src/db/database.cpp

${OBJECTDIR}/src/third/libcrest/third/zlib/trees.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/third/zlib/trees.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/third/zlib
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/third/zlib/trees.o src/third/libcrest/third/zlib/trees.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_result.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_result.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_result.o src/third/libcrest/src/cr_result.cpp

${OBJECTDIR}/src/api/API_users.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_users.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_users.o src/api/API_users.cpp

${OBJECTDIR}/src/third/libcrest/src/cr_xml.o: nbproject/Makefile-${CND_CONF}.mk src/third/libcrest/src/cr_xml.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/third/libcrest/src
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/third/libcrest/src/cr_xml.o src/third/libcrest/src/cr_xml.cpp

${OBJECTDIR}/src/api/API_server.o: nbproject/Makefile-${CND_CONF}.mk src/api/API_server.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/api
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/api/API_server.o src/api/API_server.cpp

${OBJECTDIR}/src/utils/utils.o: nbproject/Makefile-${CND_CONF}.mk src/utils/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/utils
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils/utils.o src/utils/utils.cpp

${OBJECTDIR}/src/sp/Cursor.o: nbproject/Makefile-${CND_CONF}.mk src/sp/Cursor.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/sp
	${RM} $@.d
	$(COMPILE.cc) -s -DNDEBUG -I/Paradigma/Developer_2/sources --std=c++11 -Ofast -flto -fno-exceptions -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/sp/Cursor.o src/sp/Cursor.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/slite

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
