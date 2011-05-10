require 'rake'
require 'rake/clean'
require 'rake/loaders/makefile'


CLEAN.include('build/*.o')


MAKEDEPEND= "cpp -I include -MM"
BUILD_DIR = 'build'
CC        = 'gcc'

SRC = FileList['src/*.c']
OBJ = SRC.map {  |fn| fn.sub(/\.[^.]*$/, '.o') }

file ".depends.mf" => SRC.to_a do |t|
    sh "cpp -MM  #{t.source} >> #{t.name}" 
end

# import ".depends.mf"


def buildfile(name)
  File.join(BUILD_DIR, name)
end  


def cc(*args)
  args.unshift(CC)
  sh args.join(" ")
end

rule '.o' => ['.c'] do |t|
  infile   = t.source
  outfile  = buildfile(t.name)
  cc '-c', '-o', outfile, infile
end

file buildfile('libraku.o') => OBJ do |t|
   cc *OBJ, '-o', t.name
end
   
task :lib => [ buildfile('libraku.o'), ".depends.mf"] 

# File dependencies go here ...
file 'bract.o' => ['bract.c', 'bract.h']


task :try => :lib do
  puts "OK"
end 

  
