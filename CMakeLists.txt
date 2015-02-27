# TeamPlanets building script
# TeamPlanets is an engine and bots for MachineZone candidates test
#
# Copyright (c) 2015 Vadim Litvinov <vadim_litvinov@fastmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the author nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(teamplanets)

# Setting standard compiler options
if(NOT TEAMPLANETS_COMPILER_OPTS_WAS_SET)
  set(CMAKE_C_FLAGS "-march=native -Wall" CACHE STRING "" FORCE)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "" FORCE)
  set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
  
  set(CMAKE_CXX_FLAGS "-march=native -std=c++11 -Wall" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
  
  set(TEAMPLANETS_COMPILER_OPTS_WAS_SET ON CACHE INTERNAL "")
endif(NOT TEAMPLANETS_COMPILER_OPTS_WAS_SET)

# Adding subprojects
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/libteamplanets)
