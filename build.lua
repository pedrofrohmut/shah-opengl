#! /usr/bin/env lua

--[[
   This is a build script for cpp main app.

   when called with no flags this build should detect the platform and do a release
   build for it or show a todo message otherwise.

   All other options should be with use of flags.
]]

local print_help = function()
   print("TODO: print help for this")
end

local main = function()
   local keep_apps = false -- Remove old executables by default
   local is_dev = false -- Release build by default

   -- Read build args
   for i = 1, #arg do
      if arg[i] == "--dev" or arg[i] == "--debug" then
         is_dev = true
      elseif arg[i] == "--keep" then
         keep_apps = true
      elseif arg[i] == "-h" or arg[i] == "--help" then
         print_help()
         os.exit(0)
      else
         print("ERROR: flag '"..arg[i].."' not reconized.")
         print_help()
         os.exit(1)
      end
   end

   local dev_flags = "-g -O0 -DDEBUG_MODE"
   local release_flags = "-O2"

   -- Setup build command for linux
   local flags = is_dev and dev_flags or release_flags
   local app_name = is_dev and "out/app_debug" or "app"
   print("app_name: "..app_name)
   local build_cmd = table.concat(
      {
         "g++",
         "-Wall -Wextra --std=c++20", flags, -- Flags
         "-I ./include -I ./deps/glm/", -- Include path
         "-o ", app_name, -- Output path
         "./src/main.cpp", -- Source code
         "-ldl -lSDL2",
      },
      " "
   )

   -- Remove old stuff (unless keep flag)
   if not keep_apps then
      os.execute("rm -f out/app_debug app")
   end

   if is_dev then
      print("Compiling in Dev/Debug mode")
   else
      print("Compiling in Release mode")
   end

   os.execute(build_cmd)
end

main()
