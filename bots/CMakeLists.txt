# Various TeamPlanets bots building script
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

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(teamplanets_bots)

# Common source files
include_directories(${PROJECT_SOURCE_DIR}/../libs/libteamplanets/src)

# Creating a target per bot directory
execute_process(COMMAND find ${PROJECT_SOURCE_DIR} -mindepth 1 -type d -print
                OUTPUT_VARIABLE bots_dirs)
foreach(bot_dir_orig ${bots_dirs})
  string(STRIP ${bot_dir_orig} bot_dir)
  get_filename_component(bot_name ${bot_dir} NAME_WE)
  
  # Computing the list of this bots source files
  file(GLOB_RECURSE src_files ${bot_dir}/*.cpp)
  
  # Creating the bots target
  add_executable(${bot_name} ${src_files})
  add_dependencies(${bot_name} teamplanets)
  target_include_directories(${bot_name} PRIVATE ${bot_dir})
  target_link_libraries(${bot_name} teamplanets)
  
  # Defining the bot installation rules
  install(TARGETS ${bot_name}
          RUNTIME DESTINATION bin)
endforeach(bot_dir_orig)
