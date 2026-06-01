#! /usr/bin/env lua

local print_help = function()
   print("TODO: print help for this")
end

local main = function()
   local keep_apps = false
   local is_dev = false

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
   local app_name = is_dev and "app_debug" or "app"
   local build_cmd = table.concat(
      {
         "g++", "-Wall -Wextra --std=c++17", flags,
         "-I./include",
         "-o ", app_name,
         "./src/main.cpp ./src/glad.c",
         -- "-ldl -lSDL2 -lGL",
         "-ldl -lSDL2",
      },
      " "
   )

   -- Remove old stuff (unless keep flag)
   if not keep_apps then
      os.execute("rm -f app_debug app")
   end

   if is_dev then
      print("Compiling in Dev/Debug mode")
   else
      print("Compiling in Release mode")
   end

   os.execute(build_cmd)
end

main()
